// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityTimelineTrackWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"
#include "SNeAbilityTimelineTrackNode.h"
#include "NeAbilityTrackNodeDragDropOp.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Framework/Commands/GenericCommands.h"
#include "NeAbilityEditorTypes.h"
#include "NeAbilityTimelineTrack.h"
#include "NeAbilityEditorDelegates.h"
#include "NeAbilityEditorUtilities.h"
#include "NeAbilityTimelineMode.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "TabFactory/SNeAbilityEditorTab_Palette.h"

#define LOCTEXT_NAMESPACE "SNeAbilityTimelineTrackWidget"

const float Node_Link_Height = 12.0f;
const float Node_Link_TopHeight = 2;

void SNeAbilityTimelineTrackWidget::Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityTimelineTrack>& InTimelineTrack, const TWeakPtr<FNeAbilityTimelineTrack>& InParentTrack)
{
	SetClipping(EWidgetClipping::ClipToBounds);
	TimelineTrack = InTimelineTrack;
	ParentTimelineTrack = InParentTrack;

	CommandList = InArgs._CommandList;

	ViewInputMin = InArgs._ViewInputMin;
	ViewInputMax = InArgs._ViewInputMax;
	InputMin = InArgs._InputMin;
	InputMax = InArgs._InputMax;

	TimelinePlayLength = InArgs._TimelinePlayLength.Get();
	FrameRate = InArgs._FrameRate.Get();

	OnTrackNodeSelectChanged = InArgs._OnTrackNodeSelectChanged;

	OnUpdatePanel = InArgs._OnUpdatePanel;
	OnSetInputViewRange = InArgs._OnSetInputViewRange;

	OnAddChildTrack = InArgs._OnAddNewChildTrack;

	OnAddNewNode = InArgs._OnAddNewNode;
	OnDeleteNode = InArgs._OnDeleteNode;
	OnDeselectAllTrackNodes = InArgs._OnDeselectAllTrackNodes;

	this->ChildSlot
		[
			SAssignNew(TrackArea, SBorder)
			.Visibility(EVisibility::SelfHitTestInvisible)
			.BorderImage(FAppStyle::GetBrush("NoBorder"))
			.Padding(FMargin(0.f, 0.f))
			.ColorAndOpacity(FLinearColor::White)
		];


	RefreshTrackWidget();
}

FVector2D SNeAbilityTimelineTrackWidget::ComputeDesiredSize(float) const
{
	FVector2D Size;
	Size.X = 200;
	Size.Y = NeAbilityEditorConstants::GetTrackHeight(IsChildTrack());
	return Size;
}

int32 SNeAbilityTimelineTrackWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* StyleInfo = FAppStyle::GetBrush(TEXT("Persona.NotifyEditor.NotifyTrackBackground"));

	FPaintGeometry MyGeometry = AllottedGeometry.ToPaintGeometry();

	int32 CustomLayerId = LayerId + 1;
	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0.f, 0.f, AllottedGeometry.Size);

	bool bAnyDraggedNodes = false;
	for (int32 I = 0; I < TrackNodes.Num(); ++I)
	{
		if (TrackNodes[I].Get()->BeingDragged() == false)
		{
			TrackNodes[I].Get()->UpdateSizeAndPosition(AllottedGeometry);
		}
		else
		{
			bAnyDraggedNodes = true;
		}
	}

	{
		// Draw track bottom border
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			CustomLayerId,
			AllottedGeometry.ToPaintGeometry(),
			TArray<FVector2D>({ FVector2D(0.0f, AllottedGeometry.GetLocalSize().Y), FVector2D(AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y) }),
			ESlateDrawEffect::None,
			FLinearColor(0.1f, 0.1f, 0.1f, 0.3f)
		);
	}

	++CustomLayerId;

	if (bAnyDraggedNodes)
	{
		float Value = CalculateDraggedNodePos();
		if (Value >= 0.0f)
		{
			float XPos = Value;
			TArray<FVector2D> LinePoints;
			LinePoints.Add(FVector2D(XPos, 0.f));
			LinePoints.Add(FVector2D(XPos, AllottedGeometry.Size.Y));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				CustomLayerId,
				MyGeometry,
				LinePoints,
				ESlateDrawEffect::None,
				FLinearColor(1.0f, 0.5f, 0.0f)
			);
		}
	}

	if (IsChildTrack()&& ParentTimelineTrack.IsValid())
	{
		const TSharedPtr<SNeAbilityTimelineTrackWidget>& ParentTimelineTrackWidget = ParentTimelineTrack.Pin()->GetTrackWidget();
		if (ParentTimelineTrackWidget.IsValid())
		{
			const TArray<TSharedPtr<SNeAbilityTimelineTrackNode>>& ParentTrackNodes = ParentTimelineTrackWidget->GetTrackNodes();

			TArray<FVector2D> LinePoints;
			LinePoints.AddUninitialized(2);

			for (int32 i = 0; i < TrackNodes.Num(); i++)
			{
				if (!TrackNodes[i].IsValid())
					continue;

				const FTrackNodeDataPtr& NodeData = TrackNodes[i]->GetTrackNodeData();

				TSharedPtr<SNeAbilityTimelineTrackNode> FindParent = nullptr;
				for (int32 idx = 0; idx < ParentTrackNodes.Num(); idx++)
				{
					const FTrackNodeDataPtr& ParentNodeData = ParentTrackNodes[idx]->GetTrackNodeData();
					if (ParentNodeData .IsValid() && ParentNodeData->GetAllChildSegments().Contains(NodeData->GetID()))
					{
						FindParent = ParentTrackNodes[idx];
						break;
					}
				}

				if (FindParent == nullptr)
					continue;

				LinePoints[0] = FindParent->GetWidgetPosition() + 10;
				LinePoints[0].Y = Node_Link_Height;
				LinePoints[1] = TrackNodes[i]->GetWidgetPosition() + 6;
				LinePoints[1].Y = Node_Link_Height;

				FSlateDrawElement::MakeLines(OutDrawElements, CustomLayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, FLinearColor(1, 1, 1, 0.4f));

				LinePoints[1] = LinePoints[0];
				LinePoints[1].Y = Node_Link_TopHeight;

				FSlateDrawElement::MakeLines(OutDrawElements, CustomLayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, FLinearColor(1, 1, 1, 0.4f));
			}
		}
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, CustomLayerId, InWidgetStyle, bParentEnabled);
}

FCursorReply SNeAbilityTimelineTrackWidget::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	/*if (ViewInputMin.Get() > 0.f || ViewInputMax.Get() < Sequence->GetPlayLength())
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}*/

	return FCursorReply::Unhandled();
}


FReply SNeAbilityTimelineTrackWidget::OnNotifyNodeDragStarted(TSharedRef<SNeAbilityTimelineTrackNode> NotifyNode, const FPointerEvent& MouseEvent, const FVector2D& ScreenNodePosition, const bool bDragOnMarker, int32 NotifyIndex)
{
	// Check to see if we've already selected the triggering node
	/*if (!NotifyNode->IsSelected())
	{
		SelectTrackObjectNode(NotifyIndex);
	}*/

	if (!bDragOnMarker)
	{
		ClearSelections();

		TArray<TSharedPtr<SNeAbilityTimelineTrackNode>> NodesToDrag;
		NodesToDrag.Add(NotifyNode);

		FVector2D DecoratorPosition = NodesToDrag[0]->GetWidgetPosition();
		DecoratorPosition = CachedGeometry.LocalToAbsolute(DecoratorPosition);

		TSharedRef<SOverlay> NodeDragDecoratorOverlay = SNew(SOverlay);
		TSharedRef<SBorder> NodeDragDecorator = SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				NodeDragDecoratorOverlay
			];

		FBox2D OverlayBounds(NodesToDrag[0]->GetScreenPosition(), NodesToDrag[0]->GetScreenPosition() + FVector2D(NodesToDrag[0]->GetDurationSize(), 0.0f));
		for (int32 Idx = 1; Idx < NodesToDrag.Num(); ++Idx)
		{
			TSharedPtr<SNeAbilityTimelineTrackNode> Node = NodesToDrag[Idx];
			FVector2D NodePosition = Node->GetScreenPosition();
			float NodeDuration = Node->GetDurationSize();

			OverlayBounds += FBox2D(NodePosition, NodePosition + FVector2D(NodeDuration, 0.0f));
		}

		FVector2D OverlayOrigin = OverlayBounds.Min;
		FVector2D OverlayExtents = OverlayBounds.GetSize();

		for (TSharedPtr<SNeAbilityTimelineTrackNode> Node : NodesToDrag)
		{
			FVector2D OffsetFromFirst(Node->GetScreenPosition() - OverlayOrigin);

			NodeDragDecoratorOverlay->AddSlot()
				.Padding(FMargin(OffsetFromFirst.X, OffsetFromFirst.Y, 0.0f, 0.0f))
				[
					Node->AsShared()
				];
		}

		TArray<TSharedPtr<SNeAbilityTimelineTrackWidget>> NotifyAnimTracks;
		//NotifyAnimTracks.Add(SharedThis(this));

		const TArray<TSharedRef<FNeAbilityTimelineTrack>>& TimelineTracks = GetAbilityTimelineMode()->GetAllTimelineTracks();
		for (int32 i = 0; i < TimelineTracks.Num(); i++)
		{
			const TSharedPtr<SNeAbilityTimelineTrackWidget>& Track = TimelineTracks[i]->GetTrackWidget();
			if (Track.IsValid())
			{
				NotifyAnimTracks.Add(Track);
			}
		}

		FOnUpdatePanel UpdateDelegate = FOnUpdatePanel::CreateSP(this, &SNeAbilityTimelineTrackWidget::RefreshTrackWidget);

		return FReply::Handled().BeginDragDrop(FNeAbilityTrackNodeDragDropOp::New(SharedThis(this),
			NodesToDrag, NodeDragDecorator, NotifyAnimTracks, TimelinePlayLength, MouseEvent.GetScreenSpacePosition(),
			OverlayOrigin, OverlayExtents, CurrentDragXPosition));
	}
	else
	{
		// Capture the mouse in the node
		return FReply::Handled().CaptureMouse(NotifyNode).UseHighPrecisionMouseMovement(NotifyNode);
	}
}

TSharedRef<FNeAbilityTimelineMode> SNeAbilityTimelineTrackWidget::GetAbilityTimelineMode() const
{
	check(TimelineTrack.IsValid());
	return TimelineTrack.Pin()->GetAbilityTimelineMode();
}

FNeAbilityTrack& SNeAbilityTimelineTrackWidget::GetTrackData() const
{
	check(TimelineTrack.IsValid());
	return TimelineTrack.Pin()->GetTrackData();
}

bool SNeAbilityTimelineTrackWidget::IsChildTrack() const
{
	check(TimelineTrack.IsValid());
	return TimelineTrack.Pin()->IsChildTrack();
}

FReply SNeAbilityTimelineTrackWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FVector2D CursorPos = MouseEvent.GetScreenSpacePosition();
	CursorPos = MyGeometry.AbsoluteToLocal(CursorPos);
	int32 HittedInnerNodeIndex = -1;
	int32 HitIndex = GetHitNotifyNode(MyGeometry, CursorPos, HittedInnerNodeIndex);

	if (HitIndex != INDEX_NONE)
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			// Hit a node, record the mouse position for use later so we can know when / where a
			// drag happened on the node handles if necessary.
			TrackNodes[HitIndex]->SetLastMouseDownPosition(CursorPos);

			if (!GetAbilityTimelineMode()->IsDebuggerMode())
			{
				return FReply::Handled().DetectDrag(TrackNodes[HitIndex].ToSharedRef(), EKeys::LeftMouseButton);
			}
		}
		else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			// Hit a node, return handled so we can pop a context menu on mouse up
			return FReply::Handled();
		}
	}

	return FReply::Handled();
}

FReply SNeAbilityTimelineTrackWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bool bLeftMouseButton = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	bool bRightMouseButton = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;

	if (bRightMouseButton)
	{
		LastClickedTime = CalculateTime(MyGeometry, MouseEvent.GetScreenSpacePosition());
		TSharedPtr<SWidget> WidgetToFocus = SummonContextMenu(MyGeometry, MouseEvent);

		return (WidgetToFocus.IsValid())
			? FReply::Handled().ReleaseMouseCapture().SetUserFocus(WidgetToFocus.ToSharedRef(), EFocusCause::SetDirectly)
			: FReply::Handled().ReleaseMouseCapture();

	}
	else if (bLeftMouseButton)
	{
		if (!bDoubleClickSelectionLock)
		{
			FVector2D CursorPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			int32 HittedInnerNodeIndex = -1;
			int32 NodeIndex = GetHitNotifyNode(MyGeometry, CursorPos, HittedInnerNodeIndex);

			if (NodeIndex > INDEX_NONE)
			{
				SelectTrackObjectNode(NodeIndex, -1 /*单击不选中CompositeNode*/);
			}
			else
			{
				// Clicked in empty space, clear selection
				ClearSelections();
			}
		}
		bDoubleClickSelectionLock = false;

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SNeAbilityTimelineTrackWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

FReply SNeAbilityTimelineTrackWidget::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		FVector2D CursorPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		int32 HittedInnerNodeIndex = -1;
		int32 NodeIndex = GetHitNotifyNode(MyGeometry, CursorPos, HittedInnerNodeIndex);

		if (NodeIndex >= 0 && HittedInnerNodeIndex >= 0)
		{
			SelectTrackObjectNode(NodeIndex, HittedInnerNodeIndex);
			bDoubleClickSelectionLock = true;
		}

		return FReply::Handled();
	}
	return SCompoundWidget::OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
}

FReply SNeAbilityTimelineTrackWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey& Key = InKeyEvent.GetKey();
	if (Key == EKeys::Platform_Delete && !GetAbilityTimelineMode()->IsDebuggerMode())
	{
		this->OnDeletePressed();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}


FReply SNeAbilityTimelineTrackWidget::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return TimelineTrack.Pin()->OnDropToTrack(MyGeometry, DragDropEvent);
}

void SNeAbilityTimelineTrackWidget::SelectTrackObjectNode(int32 TrackNodeIndex, int32 InnerNodeIndex)
{
	if (TrackNodeIndex == INDEX_NONE || !TrackNodes.IsValidIndex(TrackNodeIndex)) return;
	TSharedPtr<SNeAbilityTimelineTrackNode> Node = TrackNodes[TrackNodeIndex];
	if (Node->IsSelected(InnerNodeIndex))
	{
		// Is under selection, do nothing
		return;
	}

	Node->SetSelected(true, InnerNodeIndex);
	SelectedNodeIndices.AddUnique(TrackNodeIndex);
	OnTrackNodeSelectChanged.Execute(TrackNodeIndex, InnerNodeIndex);
}


void SNeAbilityTimelineTrackWidget::OnDeletePressed()
{
	// If there's no focus on the panel it's likely the user is not editing notifies
	// so don't delete anything when the key is pressed.
	if (HasKeyboardFocus() || HasFocusedDescendants())
	{
		DeleteSelectedNodeObjects();
	}
}


void SNeAbilityTimelineTrackWidget::DeleteSelectedNodeObjects()
{
	if (SelectedNodeIndices.Num() > 0)
	{
		OnDeleteNode.ExecuteIfBound(SelectedNodeIndices[0]);
		ClearSelections();
		OnTrackNodeSelectChanged.Execute(INDEX_NONE, -1);

		RefreshTrackWidget();
	}
}

void SNeAbilityTimelineTrackWidget::DeleteNodeObjects(int32 TrackNodeIndex)
{
	if (TrackNodes.IsValidIndex(TrackNodeIndex))
	{
		OnDeleteNode.Execute(TrackNodeIndex);

		ClearSelections();

		OnTrackNodeSelectChanged.Execute(INDEX_NONE, -1);

		RefreshTrackWidget();
	}
}


void SNeAbilityTimelineTrackWidget::DeselectTrackObjectNode(int32 TrackNodeIndex)
{
	check(TrackNodes.IsValidIndex(TrackNodeIndex));
	TSharedPtr<SNeAbilityTimelineTrackNode> Node = TrackNodes[TrackNodeIndex];
	Node->SetSelected(false);

	int32 ItemsRemoved = SelectedNodeIndices.Remove(TrackNodeIndex);
	check(ItemsRemoved > 0);
}

TSharedPtr<SWidget> SNeAbilityTimelineTrackWidget::SummonContextMenu(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FVector2D CursorPos = MouseEvent.GetScreenSpacePosition();
	int32 HittedInnerNodeIndex = -1;
	int32 NodeIndex = GetHitNotifyNode(MyGeometry, MyGeometry.AbsoluteToLocal(CursorPos), HittedInnerNodeIndex);
	LastClickedTime = CalculateTime(MyGeometry, MouseEvent.GetScreenSpacePosition());

	if (!GetAbilityTimelineMode()->IsDebuggerMode())
	{
		FMenuBuilder MenuBuilder(true, CommandList);

		if (NodeIndex != INDEX_NONE)
		{
			const FNeAbilitySegmentPtr HitNode = GetAbilityTimelineMode()->GetAbilitySegmentPtr(GetTrackData().Segments[NodeIndex]);
			if (!TrackNodes[NodeIndex]->IsSelected())
			{
				SelectTrackObjectNode(NodeIndex);
			}

			MenuBuilder.BeginSection("TrackNode", LOCTEXT("TrackNode_Action", "Node Action"));

			// Add item to directly set notify time
			TSharedRef<SWidget> TimeWidget =
				SNew(SBox)
				.HAlign(HAlign_Right)
				.ToolTipText(LOCTEXT("SetTimeToolTip", "Set the time of this notify directly"))
				[
					SNew(SBox)
					.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
					.WidthOverride(100.0f)
					[
						SNew(SNumericEntryBox<float>)
						.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
						.MinValue(0.0f)
						.MaxValue(TimelinePlayLength)
						.Value(TrackNodes[NodeIndex]->GetEditorNodeData().GetStartTime())
						.AllowSpin(false)
						.OnValueCommitted_Lambda([this, NodeIndex](float InValue, ETextCommit::Type InCommitType)
						{
							if (InCommitType == ETextCommit::OnEnter && TrackNodes.IsValidIndex(NodeIndex))
							{
								//float NewTime = FMath::Clamp(InValue, 0.0f, TimelinePlayLength - TrackNodes[NodeIndex]->GetEditorNodeData().GetDuration());
								float NewTime = FMath::Clamp(InValue, 0.0f, TimelinePlayLength);
								TrackNodes[NodeIndex]->GetEditorNodeDataPtr()->SetStartTime(NewTime);
								TrackNodes[NodeIndex]->GetEditorNodeDataPtr()->Snapped();

								OnUpdatePanel.Execute();

								FSlateApplication::Get().DismissAllMenus();
							}
						})
					]
				];

			MenuBuilder.AddWidget(TimeWidget, LOCTEXT("TimeMenuText", "Start Time"));

			// Add item to directly set notify frame
			TSharedRef<SWidget> FrameWidget =
				SNew(SBox)
				.HAlign(HAlign_Right)
				.ToolTipText(LOCTEXT("SetFrameToolTip", "Set the dura of this notify directly"))
				[
					SNew(SBox)
					.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
					.WidthOverride(100.0f)
					[
						SNew(SNumericEntryBox<float>)
						.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
						.MinValue(0)
						.MaxValue(TimelinePlayLength)
						.Value(TrackNodes[NodeIndex]->GetEditorNodeData().GetDuration())
						.AllowSpin(false)
						.OnValueCommitted_Lambda([this, NodeIndex](float InValue, ETextCommit::Type InCommitType)
						{
							if (InCommitType == ETextCommit::OnEnter && TrackNodes.IsValidIndex(NodeIndex))
							{
								float NewTime = FMath::Clamp(InValue, 0.0f, TimelinePlayLength - TrackNodes[NodeIndex]->GetEditorNodeData().GetStartTime());
								TrackNodes[NodeIndex]->GetEditorNodeDataPtr()->SetDuration(NewTime);

								OnUpdatePanel.Execute();

								FSlateApplication::Get().DismissAllMenus();
							}
						})
					]
				];

			MenuBuilder.AddWidget(FrameWidget, LOCTEXT("FrameMenuText", "Duration"));

			// Enable
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ToggleSegmentEnable", "Enable"),
				LOCTEXT("ToggleSegmentEnableTooltip", "Toggle segment enable/disable"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([this, NodeIndex] { TrackNodes[NodeIndex]->GetEditorNodeDataPtr()->ToggleEnable(); }),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this, NodeIndex] { return TrackNodes[NodeIndex]->GetEditorNodeDataPtr()->IsEnable(); })),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);

			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Paste);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Track_Delete", "Delete"),
				LOCTEXT("Track_Delete_Tooltips", "Delete the segment."),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::DeleteNodeObjects, NodeIndex)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			if (HitNode.IsValid() && HitNode->GetAllChildSegments().Num() <= 0)
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("Action_AttachTo_Lable", "Attach To"),
					LOCTEXT("Action_AttachTo_Tooltip", "挂接到父节点"),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::StartAttachNodeTo, NodeIndex)
					),
					NAME_None,
					EUserInterfaceActionType::Button
				);
			}

			if (HitNode.IsValid() && HitNode->HasParent())
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("Action_Detach_Lable", "Detach"),
					LOCTEXT("Action_Detach_Tooltip", "从父节点分离"),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::DetachNode, NodeIndex)
					),
					NAME_None,
					EUserInterfaceActionType::Button
				);
			}

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_AlignSection", "Align section"),
				LOCTEXT("Action_AlignSection_Tooltip", "将Section长度与节点对齐"),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([=, this]()
					{
						FNeAbilitySegmentPtr ToNodeData = GetAbilityTimelineMode()->GetAbilitySegmentPtr(GetTrackData().Segments[NodeIndex]);
						GetAbilityTimelineMode()->AlignSectionTimeWithSegment(ToNodeData);
					})
					// FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::DetachNode, NodeIndex)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);


			MenuBuilder.EndSection();

			MenuBuilder.BeginSection("Tracks", LOCTEXT("TrackMenuSection", "Option"));
			{
				if (HitNode.IsValid() && HitNode->IsCompound() && !HitNode->HasParent())
				{
					MenuBuilder.AddSubMenu(
						LOCTEXT("Action_AddChildTrack_Lable", "Add child track"),
						LOCTEXT("Action_AddChildTrack_Tooltip", "Add a new child track"),
						FNewMenuDelegate::CreateRaw(this, &SNeAbilityTimelineTrackWidget::FillNewChildNodeMenu, NodeIndex)
					);
				}
			}
			MenuBuilder.EndSection();

			if (HitNode.IsValid())
			{
				FNeAbilityEditorDelegates::BuildSegmentContextMenuDelegate.Broadcast(MenuBuilder, HitNode.Get());
			}
		}
		else
		{
			MenuBuilder.BeginSection("Track", LOCTEXT("TrackActions", "Track Actions"));

			MenuBuilder.AddSubMenu(
				NSLOCTEXT("TrackSubMenu", "TrackSubMenu_AddSegment", "Add Node"),
				NSLOCTEXT("TrackSubMenu", "TrackSubMenu_AddSegment_ToolTip", "Add a new node on track"),
				FNewMenuDelegate::CreateRaw(this, &SNeAbilityTimelineTrackWidget::FillNewNodeMenu));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_Paste_Lable", "Paste"),
				LOCTEXT("Action_Paste_Tooltip", "Paste node from copy."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::PasteNode, 0)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_Sort_Lable", "Sort Track"),
				LOCTEXT("Action_Sort_Tooltip", "Sort node by startTime."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::SortTrack)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_AlighSectionTime_Lable", "Align To Animation"),
				LOCTEXT("Action_AlighSectionTime_Tooltip", "Align section time use montage length."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::AlignSectionTimeWithAnimation)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.EndSection();
		}

		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		// Display the newly built menu
		FSlateApplication::Get().PushMenu(SharedThis(this), WidgetPath, MenuBuilder.MakeWidget(), CursorPos, FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

		return TSharedPtr<SWidget>();
	}
	else
	{
		FMenuBuilder MenuBuilder(true, CommandList);

		if (NodeIndex != INDEX_NONE)
		{
			if (!TrackNodes[NodeIndex]->IsSelected())
			{
				SelectTrackObjectNode(NodeIndex);
			}

			MenuBuilder.BeginSection("TrackNode", LOCTEXT("TrackNodeAction", "Debugger"));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_Sort_Lable", "Add BreakPoint"),
				LOCTEXT("Action_Sort_Tooltip", "Add BreakPoint."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::AddBreakPoint, NodeIndex)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_Sort_Lable", "Remove BreakPoint"),
				LOCTEXT("Action_Sort_Tooltip", "Remove BreakPoint."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineTrackWidget::RemoveBreakPoint, NodeIndex)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.EndSection();
		}

		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		// Display the newly built menu
		FSlateApplication::Get().PushMenu(SharedThis(this), WidgetPath, MenuBuilder.MakeWidget(), CursorPos, FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

		return TSharedPtr<SWidget>();
	}
}

void SNeAbilityTimelineTrackWidget::FillNewNodeMenu(FMenuBuilder& MenuBuilder)
{
	FNeAbilityEditorUtilities::MakeNewSegmentPicker(GetAbilityTimelineMode()->GetAbilityEditor().ToSharedRef(), MenuBuilder, FOnClassPicked::CreateSP(this, &SNeAbilityTimelineTrackWidget::CreateNewNodeAtCursor));
}

void SNeAbilityTimelineTrackWidget::CreateNewNodeAtCursor(UClass* NotifyClass)
{
	FSlateApplication::Get().DismissAllMenus();

	OnAddNewNode.ExecuteIfBound(NotifyClass, LastClickedTime);
}

void SNeAbilityTimelineTrackWidget::FillNewChildNodeMenu(class FMenuBuilder& MenuBuilder, int32 TrackNodeIndex)
{
	FNeAbilityEditorUtilities::MakeNewSegmentPicker(GetAbilityTimelineMode()->GetAbilityEditor().ToSharedRef(), MenuBuilder, FOnClassPicked::CreateSP(this, &SNeAbilityTimelineTrackWidget::AddNewChildNodeTrack, TrackNodeIndex));
}

void SNeAbilityTimelineTrackWidget::AddNewChildNodeTrack(UClass* InClass, int32 TrackNodeIndex)
{
	FSlateApplication::Get().DismissAllMenus();

	OnAddChildTrack.ExecuteIfBound(InClass, TrackNodeIndex);
}

void SNeAbilityTimelineTrackWidget::FillAttachToNodeMenu(class FMenuBuilder& MenuBuilder, int32 TrackNodeIndex)
{
	const FNeAbilitySegmentPtr ToNodeData = GetAbilityTimelineMode()->GetAbilitySegmentPtr(GetTrackData().Segments[TrackNodeIndex]);
	if (!ToNodeData.IsValid()) return ;
	UNeAbility* Asset = ToNodeData.GetOutter();

	FNeAbilitySectionPtr SectionPtr = GetAbilityTimelineMode()->GetAbilitySectionPtr();
	if (SectionPtr.IsValid())
	{
		for (const FNeAbilitySegment& Element : SectionPtr->Segments)
		{
			const FText Description = LOCTEXT("AttachToTrack_ToolTip", "Attach to an existing track");
			const FText Label = FText::FromName(Element.GetName());

			FUIAction UIAction;
			MenuBuilder.AddMenuEntry(Label, Description, FSlateIcon(), UIAction);
		}
	}
}

void SNeAbilityTimelineTrackWidget::StartAttachNodeTo(int32 TrackNodeIndex) const
{
	GetAbilityTimelineMode()->EnterNodePickMode(FNeAbilityTimelineMode::FOnPickNodeDelegate::CreateLambda([this, TrackNodeIndex] (FNeAbilitySegmentPtr& InParentSegment)
	{
		FNeAbilitySegmentPtr ToNodeData = GetAbilityTimelineMode()->GetAbilitySegmentPtr(GetTrackData().Segments[TrackNodeIndex]);
		if (!ToNodeData.IsValid()) return ;
		GetAbilityTimelineMode()->AttachTo(InParentSegment, ToNodeData);
	}));
}

void SNeAbilityTimelineTrackWidget::DetachNode(int32 TrackNodeIndex) const
{
	FNeAbilitySegmentPtr ToNodeData = GetAbilityTimelineMode()->GetAbilitySegmentPtr(GetTrackData().Segments[TrackNodeIndex]);
	if (!ToNodeData.IsValid()) return ;
	GetAbilityTimelineMode()->Detach(ToNodeData);
}

void SNeAbilityTimelineTrackWidget::PasteNode(int32 TrackNodeIndex)
{
	GetAbilityTimelineMode()->DoPasteNodeToTrack(&GetTrackData(), IsChildTrack(), LastClickedTime);
}

void SNeAbilityTimelineTrackWidget::SortTrack()
{
	GetAbilityTimelineMode()->SortTrackByStartTime();
}

void SNeAbilityTimelineTrackWidget::AlignSectionTimeWithAnimation()
{
	GetAbilityTimelineMode()->AlignSectionTimeWithAnimation(false);
}

void SNeAbilityTimelineTrackWidget::AddBreakPoint(int32 NodeIndex)
{
	const FNeAbilitySegmentPtr ToNodeData = GetAbilityTimelineMode()->GetAbilitySegmentPtr(GetTrackData().Segments[NodeIndex]);
	if (!ToNodeData.IsValid()) return;
	GetAbilityTimelineMode()->AddBreakPoint(ToNodeData);
}

void SNeAbilityTimelineTrackWidget::RemoveBreakPoint(int32 NodeIndex)
{
	const FNeAbilitySegmentPtr ToNodeData = GetAbilityTimelineMode()->GetAbilitySegmentPtr(GetTrackData().Segments[NodeIndex]);
	if (!ToNodeData.IsValid()) return;
	GetAbilityTimelineMode()->RemoveBreakPoint(ToNodeData);
}

void SNeAbilityTimelineTrackWidget::RefreshTrackWidget()
{
	TrackNodes.Empty();

	TrackArea->SetContent(
		SAssignNew(NodeSlots, SOverlay)
	);

	FNeAbilityTrack& TrackData = GetTrackData();
	TSharedRef<FNeAbilityTimelineMode> AbilityTimelineMode = GetAbilityTimelineMode();
	const bool bChildTrack = IsChildTrack();
	for (int32 NodeIndex = 0; NodeIndex < TrackData.Segments.Num(); ++NodeIndex)
	{
		TSharedPtr<SNeAbilityTimelineTrackNode> NewTrackNode = nullptr;

		FNeAbilitySegmentPtr NodeData = GetAbilityTimelineMode()->GetAbilitySegmentPtr(TrackData.Segments[NodeIndex]);
		if (NodeData->ValidateAsset() == false) continue;
		
		SAssignNew(NewTrackNode, SNeAbilityTimelineTrackNode, AbilityTimelineMode, bChildTrack, GetAbilityTimelineMode()->IsDebuggerMode())
			.NodeData(GetAbilityTimelineMode()->GetAbilitySegmentPtr(TrackData.Segments[NodeIndex]))
			.NodeIndex(NodeIndex)
			.ViewInputMin(ViewInputMin)
			.ViewInputMax(ViewInputMax)
			.TimelinePlayLength(TimelinePlayLength)
			.FrameRate(FrameRate)
			.OnNodeDragStarted(this, &SNeAbilityTimelineTrackWidget::OnNotifyNodeDragStarted, 0)
			.OnUpdatePanel(OnUpdatePanel)
			.OnTrackNodeSelectChanged(OnTrackNodeSelectChanged);

		NodeSlots->AddSlot()
			.Padding(TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(this, &SNeAbilityTimelineTrackWidget::GetNotifyTrackPadding, NodeIndex)))
			[
				NewTrackNode->AsShared()
			];

		TrackNodes.Add(NewTrackNode);
	}
}

FMargin SNeAbilityTimelineTrackWidget::GetNotifyTrackPadding(int32 NodeIndex) const
{
	float LeftMargin = TrackNodes[NodeIndex]->GetWidgetPosition().X;
	float RightMargin = CachedGeometry.GetLocalSize().X - TrackNodes[NodeIndex]->GetWidgetPosition().X - TrackNodes[NodeIndex]->GetSize().X;
	return FMargin(LeftMargin, 0, RightMargin, 0);
}

int32 SNeAbilityTimelineTrackWidget::GetHitNotifyNode(const FGeometry& MyGeometry, const FVector2D& CursorPosition, int32& OutHitedInnerNodeIndex)
{
	for (int32 I = TrackNodes.Num() - 1; I >= 0; --I)
	{
		if (TrackNodes[I].Get()->HitTest(MyGeometry, CursorPosition, OutHitedInnerNodeIndex))
		{
			return I;
		}
	}

	return INDEX_NONE;
}

float SNeAbilityTimelineTrackWidget::CalculateTime(const FGeometry& MyGeometry, FVector2D NodePos, bool bInputIsAbsolute)
{
	if (bInputIsAbsolute)
	{
		NodePos = MyGeometry.AbsoluteToLocal(NodePos);
	}
	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0, 0, MyGeometry.Size);
	return FMath::Clamp<float>(ScaleInfo.LocalXToInput(NodePos.X), 0.f, TimelinePlayLength);
}


void SNeAbilityTimelineTrackWidget::OffsetNodePosition(TSharedPtr<SNeAbilityTimelineTrackNode> Node, float Offset)
{
	ensure(Node.IsValid());

	float LocalX = GetCachedGeometry().AbsoluteToLocal(Node->GetScreenPosition() + Offset).X;
	float Time = GetCachedScaleInfo().LocalXToInput(LocalX);

	Node->GetEditorNodeDataPtr()->SetStartTime(Time, true);
}

void SNeAbilityTimelineTrackWidget::UpdateCachedGeometry(const FGeometry& InGeometry)
{
	CachedGeometry = InGeometry;

	for (TSharedPtr<SNeAbilityTimelineTrackNode> Node : TrackNodes)
	{
		Node->CacheGeometry(InGeometry);
	}
}

void  SNeAbilityTimelineTrackWidget::ClearSelections()
{
	for (TSharedPtr<SNeAbilityTimelineTrackNode> Node : TrackNodes)
	{
		Node->SetSelected(false);
	}
	SelectedNodeIndices.Empty();
	OnDeselectAllTrackNodes.Execute();
}

void SNeAbilityTimelineTrackWidget::ClearNodeTooltips()
{
	FText EmptyTooltip;

	for (TSharedPtr<SNeAbilityTimelineTrackNode> Node : TrackNodes)
	{
		Node->SetToolTipText(EmptyTooltip);
	}
}


#undef LOCTEXT_NAMESPACE
