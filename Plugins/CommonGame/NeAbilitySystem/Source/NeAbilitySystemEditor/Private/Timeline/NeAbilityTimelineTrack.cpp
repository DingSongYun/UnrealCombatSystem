// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityTimelineTrack.h"
#include "NeAbilitySegmentEditorObject.h"
#include "NeAbilityEditorTypes.h"
#include "NeAbilityEditorUtilities.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Timeline/SNeTimelineOutlinerItem.h"
#include "NeAbilityTimelineMode.h"
#include "NeAbilityTrackDragDropOp.h"
#include "NeAbilityTrackNodeDragDropOp.h"
#include "SNeAbilityTimelineTrackOutliner.h"
#include "SNeAbilityTimelineTrackWidget.h"
#include "TabFactory/SNeAbilityEditorTab_Palette.h"

#define LOCTEXT_NAMESPACE "FNeAbilityTimelineTrack"

ANIMTIMELINE_IMPLEMENT_TRACK(FNeAbilityTimelineTrack);

FNeAbilityTimelineTrack::FNeAbilityTimelineTrack(const TSharedPtr<FNeAbilityTimelineMode>& InModel, FNeAbilityTrack& InTrackData, const FText& InDisplayName, const FText& InToolTipText)
	: FNeTimelineTrack(StaticCastSharedPtr<FNeTimelineMode>(InModel), InDisplayName, InToolTipText, true)
	, TrackData(InTrackData)
{
	bIsSelecting = false;
	SetHeight(GetDesiredHeight());
}

TSharedRef<SWidget> FNeAbilityTimelineTrack::GenerateContainerWidgetForTimeline()
{
	if (!TrackWidget.IsValid())
	{
		TrackWidget = SNew(SNeAbilityTimelineTrackWidget, SharedThis(this), AttachedParent)
			.TrackData(&TrackData)
			.InputMin(this, &FNeAbilityTimelineTrack::GetMinInput)
			.InputMax(this, &FNeAbilityTimelineTrack::GetMaxInput)
			.ViewInputMin(this, &FNeAbilityTimelineTrack::GetViewMinInput)
			.ViewInputMax(this, &FNeAbilityTimelineTrack::GetViewMaxInput)
			.TimelinePlayLength(GetTimelineMode()->GetPlayLength())
			.FrameRate(GetTimelineMode()->GetFrameRate())
			.CommandList(GetTimelineMode()->GetCommandList())
			.OnUpdatePanel(this, &FNeAbilityTimelineTrack::RefreshTrack)
			.OnSetInputViewRange(this, &FNeAbilityTimelineTrack::InputViewRangeChanged)
			.OnTrackNodeSelectChanged(this, &FNeAbilityTimelineTrack::OnTrackNodeSelectChanged)
			.OnAddNewChildTrack(this, &FNeAbilityTimelineTrack::AddNewChildTrackByIndex)
			.OnAddNewNode(this, &FNeAbilityTimelineTrack::AddNewSegment)
			.OnDeleteNode(this, &FNeAbilityTimelineTrack::RemoveSegment)
			.OnDeselectAllTrackNodes(this, &FNeAbilityTimelineTrack::OnDeselectAllTrackNodes);

		GetTimelineMode()->OnHandleObjectsSelected().AddSP(this, &FNeAbilityTimelineTrack::HandleObjectsSelected);
	}

	return TrackWidget.ToSharedRef();
}

TSharedRef<SWidget> FNeAbilityTimelineTrack::GenerateContainerWidgetForOutliner(const TSharedRef<SNeTimelineOutlinerItem>& InRow)
{
	TSharedRef<SWidget> Outliner =  SNew(SNeAbilityTimelineTrackOutliner, SharedThis(this))
		.DisplayText(this, &FNeAbilityTimelineTrack::GetLabel)
		.ToolTipText(this, &FNeTimelineTrack::GetToolTipText)
		.ExpanderArrow(!IsChildTrack()? SNew(SExpanderArrow, InRow) : SNullWidget::NullWidget);

		return Outliner;
}

void FNeAbilityTimelineTrack::RefreshOutlinerWidget()
{
	
}

float FNeAbilityTimelineTrack::GetDesiredHeight() const
{
	// int32 CompositeTaskNum = 0;
	// for (const int32 SegmentID : TrackData.Segments)
	// {
	// 	if (SegmentID <= 0)
	// 		continue;
	//
	// 	if (UNeAbilityComposite* TaskComposite = Cast<UNeAbilityComposite>(Task.Get()))
	// 	{
	// 		CompositeTaskNum = FMath::Max(TaskComposite->GetVailidTaskNum(), CompositeTaskNum);
	// 	}
	// }
	// return NeAbilityEditorConstants::GetTrackHeight(bIsChildTrack) + CompositeTaskNum * NeAbilityEditorConstants::CompositeTrackHeight;
	return NeAbilityEditorConstants::GetTrackHeight(IsChildTrack());
}

void FNeAbilityTimelineTrack::AttachToTrack(const TWeakPtr<FNeAbilityTimelineTrack>& TrackPanel)
{
	check(TrackPanel.IsValid());
	AttachedParent = TrackPanel;
	SetIsHeaderTrack(!IsChildTrack());
}

TSharedRef<FNeAbilityTimelineMode> FNeAbilityTimelineTrack::GetAbilityTimelineMode() const
{
	return StaticCastSharedRef<FNeAbilityTimelineMode>(TimelineMode.Pin().ToSharedRef());
}

FReply FNeAbilityTimelineTrack::OnDropToTrack(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid())
	{
		return FReply::Unhandled();
	}

	if (Operation->IsOfType<FNeAbilityTrackDragDropOp>())
	{
		const auto& TrackDragDropOp = StaticCastSharedPtr<FNeAbilityTrackDragDropOp>(Operation);
		GetAbilityTimelineMode()->SwapTrackPosition(*TrackDragDropOp->SourceTrackData, TrackDragDropOp->bSourceChildTrack, TrackData, IsChildTrack());

		return FReply::Handled();
	}
	else if (Operation->IsOfType<FGraphSchemaActionDragDropAction_AbilityPaletteItem>())
	{
		const auto& FrameDragDropOp = StaticCastSharedPtr<FGraphSchemaActionDragDropAction_AbilityPaletteItem>(Operation);

		if (IsChildTrack())
		{
			GetAbilityTimelineMode()->AddSubTrack(TrackData, FrameDragDropOp->GetAbilityActionClass());
		}
		else
		{
			GetAbilityTimelineMode()->AddNewSegment(TrackData, FrameDragDropOp->GetAbilityActionClass());
		}
		return FReply::Handled();
	}
	else if (Operation->IsOfType<FNeAbilityTrackNodeDragDropOp>())
	{
		const auto& NodeDragDropOp = StaticCastSharedPtr<FNeAbilityTrackNodeDragDropOp>(Operation);
		const int32 NumNodes = NodeDragDropOp->SelectedNodes.Num();
		if (NodeDragDropOp->SelectedNodes.Num() > 0)
		{
			// 其他Track的Node拖拽到当前Track
			if (NodeDragDropOp->CurNotifyTrack != TrackWidget)
			{
				const FTrackNodeDataPtr& NodeData = NodeDragDropOp->SelectedNodes[0]->GetTrackNodeData();
				bool bIsSrcChildTrack = NodeDragDropOp->SelectedNodes[0]->IsChildNode();

				GetAbilityTimelineMode()->SwapNodeToTrack(NodeData, bIsSrcChildTrack, GetTrackData(), IsChildTrack());
			}
			// 当前Track内的Node在行内进行拖拽
			else
			{
				for (int32 CurrentNode = 0; CurrentNode < NumNodes; ++CurrentNode)
				{
					TSharedPtr<SNeAbilityTimelineTrackNode> Node = NodeDragDropOp->SelectedNodes[CurrentNode];
					const float NodePositionOffset = NodeDragDropOp->NodeXOffsets[CurrentNode];
					const FNeAbilityTrackNodeDragDropOp::FTrackClampInfo& ClampInfo = NodeDragDropOp->GetTrackClampInfo(Node->GetScreenPosition());
					ClampInfo.NotifyTrack->OffsetNodePosition(Node, NodePositionOffset);
					Node->DropCancelled();
				}

				RefreshTrack();
			}
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

void FNeAbilityTimelineTrack::FillNewChildTrackMenu(class FMenuBuilder& MenuBuilder)
{
	TSharedPtr<FNeAbilityTimelineMode> AbilityTimelineMode = StaticCastSharedPtr<FNeAbilityTimelineMode>(TimelineMode.Pin());
	FNeAbilityEditorUtilities::MakeNewSegmentPicker(AbilityTimelineMode->GetAbilityEditor().ToSharedRef(), MenuBuilder, FOnClassPicked::CreateSP(this, &FNeAbilityTimelineTrack::AddNewChildTrack));
}

void FNeAbilityTimelineTrack::RefreshTrack()
{
	SetHeight(GetDesiredHeight());
	
	RefreshOutlinerWidget();

	if (TrackWidget.IsValid())
	{
		TrackWidget->RefreshTrackWidget();
	}
}

void FNeAbilityTimelineTrack::InputViewRangeChanged(float ViewMin, float ViewMax)
{
	/*float Ratio = (ViewMax - ViewMin) / Sequence->GetPlayLength();
	float OffsetFraction = ViewMin / Sequence->GetPlayLength();
	if (NotifyTrackScrollBar.IsValid())
	{
		NotifyTrackScrollBar->SetState(OffsetFraction, Ratio);
	}

	SAnimTrackPanel::InputViewRangeChanged(ViewMin, ViewMax);*/
}


void FNeAbilityTimelineTrack::ChangeTrackName(const FString& NewName)
{
	if (TimelineMode.Pin()->IsDebuggerMode())
		return;

	const TSharedPtr<FNeAbilityTimelineMode> AbilityTimelineMode = StaticCastSharedPtr<FNeAbilityTimelineMode>(TimelineMode.Pin());
	AbilityTimelineMode->ChangeTrackName(TrackData, FName(*NewName));

	this->RefreshTrack();
}

void FNeAbilityTimelineTrack::OnTrackNodeSelectChanged(int32 NodeIndex, int32 InnerNodeIndex)
{
	if (!bIsSelecting)
	{
		TGuardValue<bool> GuardValue(bIsSelecting, true);

		if (TrackData.Segments.IsValidIndex(NodeIndex))
		{
			const int32 SegID = TrackData.Segments[NodeIndex];
			FNeAbilitySegmentPtr SegmentPtr = GetAbilityTimelineMode()->GetAbilitySegmentPtr(SegID);
			const FString ObjName = MakeUniqueObjectName(GetTransientPackage(), UNeAbilitySegmentEditorObject::StaticClass()).ToString();
			UNeAbilitySegmentEditorObject* SegmentObject = NewObject<UNeAbilitySegmentEditorObject>(GetTransientPackage(), FName(*ObjName), RF_Public | RF_Standalone | RF_Transient);
			SegmentObject->Initialize(SegmentPtr);


			// UObject* SelectedObject = TrackData.Tasks[NodeIndex].Get();
			// if (InnerTaskIndex >= 0)
			// {
			// 	if (UNeAbilitySegmentComposite* TaskComposite = Cast<UNeAbilitySegmentComposite>(TrackData.Tasks[NodeIndex]))
			// 	{
			// 		if (TaskComposite->TaskBarriers.IsValidIndex(InnerTaskIndex) && TaskComposite->TaskBarriers[InnerTaskIndex].InnerTask)
			// 		{
			// 			SelectedObject = TaskComposite->TaskBarriers[InnerTaskIndex].InnerTask;
			// 		}
			// 	}
			// }
			//
			// TaskObjects.Add(SelectedObject);
			TimelineMode.Pin()->SelectObjects({SegmentObject});
			TimelineMode.Pin()->SetTrackSelected(StaticCastSharedRef<FNeTimelineTrack>(SharedThis(this)), true);
			// TimelineMode.Pin()->SelectStructs(
			// {
			// 	MakeShareable( new FStructOnScope(FNeAbilitySegment::StaticStruct(), reinterpret_cast<uint8*>(&GetAbilityTimelineMode()->GetAbilitySegmentPtr(SegID).Get()) ) )
			// });
		}
	}
}

void FNeAbilityTimelineTrack::HandleObjectsSelected(const TArray<UObject*>& InObjects)
{
	if (!bIsSelecting)
	{
		if (TrackWidget.IsValid())
		{
			TrackWidget->ClearSelections();
		}
	}
}

void FNeAbilityTimelineTrack::AddNewChildTrackByIndex(UClass* InTaskClass, int32 NodeIndex)
{
	if (!TrackData.Segments.IsValidIndex(NodeIndex))
		return;

	// UGBSTask* NewTask = TimelineMode.Pin()->AddSubTask(TrackData, TrackData.Tasks[NodeIndex], InTaskClass);
	// if (NewTask)
	// {
	// 	NewTask->SetStartTime(TrackData.Tasks[NodeIndex]->GetStartTime());
	// }

	this->RefreshTrack();
}

void FNeAbilityTimelineTrack::AddNewChildTrack(UClass* InTaskClass)
{
	// for (int32 i = 0; i < TrackData.Tasks.Num(); i++)
	// {
	// 	if (TrackData.Tasks[i]->IsCompoundTask())
	// 	{
	// 		AddNewChildTrackByIndex(InTaskClass, i);
	// 		break;
	// 	}
	// }
}

void FNeAbilityTimelineTrack::RemoveTrack()
{
	GetAbilityTimelineMode()->RemoveTrack(TrackData);
	this->RefreshTrack();
}

FText FNeAbilityTimelineTrack::GetLabel() const
{
	return TrackData.TrackName.IsNone() ? DisplayName : FText::FromName(TrackData.TrackName);
}

void FNeAbilityTimelineTrack::AddNewSegment(UClass* InTaskClass,  float InStartTime)
{
	FNeAbilitySegment& NewSegment = GetAbilityTimelineMode()->AddNewSegment(TrackData, InTaskClass);
	NewSegment.SetStartTime(InStartTime);

	this->RefreshTrack();
}

void FNeAbilityTimelineTrack::RemoveSegment(int32 NodeIndex)
{
	GetAbilityTimelineMode()->RemoveTrackNode(TrackData, GetAbilityTimelineMode()->GetAbilitySegmentPtr(TrackData.Segments[NodeIndex]));
	this->RefreshTrack();
}

void FNeAbilityTimelineTrack::OnDeselectAllTrackNodes()
{
	TimelineMode.Pin()->ClearTrackSelection();
	TimelineMode.Pin()->SelectObjects({});
}

bool FNeAbilityTimelineTrack::HasAnyCompoundNode() const
{
	const TSharedRef<FNeAbilityTimelineMode> AbilityTimelineMode = GetAbilityTimelineMode();
	for (int32 i = 0; i < TrackData.Segments.Num(); i++)
	{
		FNeAbilitySegmentPtr SegmentPtr = AbilityTimelineMode->GetAbilitySegmentPtr(TrackData.Segments[i]);
		if (SegmentPtr.IsValid() && SegmentPtr->IsCompound())
		{
			return true;
		}
	}
	
	return false;
}

#undef LOCTEXT_NAMESPACE