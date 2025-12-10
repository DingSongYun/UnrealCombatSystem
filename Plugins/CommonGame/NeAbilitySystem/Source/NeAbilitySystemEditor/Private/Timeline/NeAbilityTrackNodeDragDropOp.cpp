// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityTrackNodeDragDropOp.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/Application/SlateApplication.h"
#include "SNeAbilityTimelineTrackNode.h"
#include "SNeAbilityTimelineTrackWidget.h"

#define LOCTEXT_NAMESPACE "FNeAbilityTrackNodeDragDropOp"

FNeAbilityTrackNodeDragDropOp::FNeAbilityTrackNodeDragDropOp(float& InCurrentDragXPosition) :
	CurrentDragXPosition(InCurrentDragXPosition),
	SelectionTimeLength(0.0f)
{
}

void FNeAbilityTrackNodeDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}

void FNeAbilityTrackNodeDragDropOp::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	NodeGroupPosition = DragDropEvent.GetScreenSpacePosition() + DragOffset;

	FTrackClampInfo* SelectionPositionClampInfo = &GetTrackClampInfo(DragDropEvent.GetScreenSpacePosition());
	//if ((SelectionPositionClampInfo->NotifyTrack->GetTrackIndex() + TrackSpan) >= ClampInfos.Num())
	//{
	//	// Our selection has moved off the bottom of the notify panel, adjust the clamping information to keep it on the panel
	//	SelectionPositionClampInfo = &ClampInfos[ClampInfos.Num() - TrackSpan - 1];
	//}

	const FGeometry& TrackGeom = SelectionPositionClampInfo->NotifyTrack->GetCachedGeometry();
	const FTrackScaleInfo& TrackScaleInfo = SelectionPositionClampInfo->NotifyTrack->GetCachedScaleInfo();

	FVector2D SelectionBeginPosition = TrackGeom.LocalToAbsolute(TrackGeom.AbsoluteToLocal(NodeGroupPosition) + SelectedNodes[0]->GetNotifyPositionOffset());

	float LocalTrackMin = TrackScaleInfo.InputToLocalX(0.0f);
	float LocalTrackMax = TrackScaleInfo.InputToLocalX(ViewPlayLength);
	float LocalTrackWidth = LocalTrackMax - LocalTrackMin;

	// Tracks the movement amount to apply to the selection due to a snap.
	float SnapMovement = 0.0f;
	// Clamp the selection into the track
	float SelectionBeginLocalPositionX = TrackGeom.AbsoluteToLocal(SelectionBeginPosition).X;
	const float ClampedEnd = FMath::Clamp(SelectionBeginLocalPositionX + NodeGroupSize.X, LocalTrackMin, LocalTrackMax);
	const float ClampedBegin = FMath::Clamp(SelectionBeginLocalPositionX, LocalTrackMin, LocalTrackMax);
	if (ClampedBegin > SelectionBeginLocalPositionX)
	{
		SelectionBeginLocalPositionX = ClampedBegin;
	}
	else if (ClampedEnd < SelectionBeginLocalPositionX + NodeGroupSize.X)
	{
		SelectionBeginLocalPositionX = ClampedEnd - NodeGroupSize.X;
	}

	SelectionBeginPosition.X = TrackGeom.LocalToAbsolute(FVector2D(SelectionBeginLocalPositionX, 0.0f)).X;

	// Handle node snaps
	bool bSnapped = false;
	for (int32 NodeIdx = 0; NodeIdx < SelectedNodes.Num() && !bSnapped; ++NodeIdx)
	{
		TSharedPtr<SNeAbilityTimelineTrackNode> CurrentNode = SelectedNodes[NodeIdx];

		const FTrackClampInfo& NodeClamp = GetTrackClampInfo(CurrentNode->GetScreenPosition());

		FVector2D EventPosition = SelectionBeginPosition + FVector2D(TrackScaleInfo.PixelsPerInput * NodeTimeOffsets[NodeIdx], 0.0f);

		// Always clamp the Y to the current track
		SelectionBeginPosition.Y = SelectionPositionClampInfo->TrackPos - 1.0f;
	}

	SelectionBeginPosition.X += SnapMovement;

	CurrentDragXPosition = TrackGeom.AbsoluteToLocal(FVector2D(SelectionBeginPosition.X, 0.0f)).X;

	FVector2D MovePositon = TrackGeom.LocalToAbsolute(TrackGeom.AbsoluteToLocal(SelectionBeginPosition) - SelectedNodes[0]->GetNotifyPositionOffset());
	//MovePositon.Y = NodeGroupPosition.Y;

	CursorDecoratorWindow->MoveWindowTo(MovePositon);
	NodeGroupPosition = SelectionBeginPosition;
}

FNeAbilityTrackNodeDragDropOp::FTrackClampInfo& FNeAbilityTrackNodeDragDropOp::GetTrackClampInfo(const FVector2D NodePos)
{
	int32 ClampInfoIndex = 0;
	int32 SmallestNodeTrackDist = FMath::Abs(ClampInfos[0].TrackSnapTestPos - NodePos.Y);
	for (int32 i = 0; i < ClampInfos.Num(); ++i)
	{
		int32 Dist = FMath::Abs(ClampInfos[i].TrackSnapTestPos - NodePos.Y);
		if (Dist < SmallestNodeTrackDist)
		{
			SmallestNodeTrackDist = Dist;
			ClampInfoIndex = i;
		}
	}
	return ClampInfos[ClampInfoIndex];
}

TSharedRef<FNeAbilityTrackNodeDragDropOp> FNeAbilityTrackNodeDragDropOp::New(const TSharedPtr<SNeAbilityTimelineTrackWidget> InCurNotifyTrack,  TArray<TSharedPtr<SNeAbilityTimelineTrackNode>> NotifyNodes, TSharedPtr<SWidget>	Decorator,
	const TArray<TSharedPtr<SNeAbilityTimelineTrackWidget>>& NotifyTracks, float InViewPlayLength, const FVector2D& CursorPosition,
	const FVector2D& SelectionScreenPosition, const FVector2D& SelectionSize, float& CurrentDragXPosition)
{
	TSharedRef<FNeAbilityTrackNodeDragDropOp> Operation = MakeShareable(new FNeAbilityTrackNodeDragDropOp(CurrentDragXPosition));
	Operation->CurNotifyTrack = InCurNotifyTrack;
	Operation->ViewPlayLength = MAX_FLT;// InViewPlayLength* MaxMoveOutSide;

	Operation->NodeGroupPosition = SelectionScreenPosition;
	Operation->NodeGroupSize = SelectionSize;
	Operation->DragOffset = SelectionScreenPosition - CursorPosition;
	Operation->Decorator = Decorator;
	Operation->SelectedNodes = NotifyNodes;
	Operation->TrackSpan = NotifyTracks.Num();  

	// Caclulate offsets for the selected nodes
	float BeginTime = MAX_flt;
	for (TSharedPtr<SNeAbilityTimelineTrackNode> Node : NotifyNodes)
	{
		float NotifyTime = Node->GetEditorNodeData().GetStartTime();

		if (NotifyTime < BeginTime)
		{
			BeginTime = NotifyTime;
		}
	}

	// Initialise node data
	for (TSharedPtr<SNeAbilityTimelineTrackNode> Node : NotifyNodes)
	{
		float NotifyTime = Node->GetEditorNodeData().GetStartTime();

		Operation->NodeTimeOffsets.Add(NotifyTime - BeginTime);
		Operation->NodeTimes.Add(NotifyTime);
		Operation->NodeXOffsets.Add(Node->GetNotifyPositionOffset().X);

		// Calculate the time length of the selection. Because it is possible to have states
		// with arbitrary durations we need to search all of the nodes and find the furthest
		// possible point
		Operation->SelectionTimeLength = FMath::Max(Operation->SelectionTimeLength, NotifyTime + Node->GetEditorNodeData().GetDuration() - BeginTime);
	}

	Operation->Construct();

	for (int32 i = 0; i < NotifyTracks.Num(); ++i)
	{
		FTrackClampInfo Info;
		Info.NotifyTrack = NotifyTracks[i];
		const FGeometry& CachedGeometry = Info.NotifyTrack->GetCachedGeometry();
		Info.TrackPos = CachedGeometry.AbsolutePosition.Y;
		Info.TrackSnapTestPos = Info.TrackPos + (CachedGeometry.Size.Y / 2);
		Operation->ClampInfos.Add(Info);
	}

	Operation->CursorDecoratorWindow->SetOpacity(0.5f);
	return Operation;
}

FText FNeAbilityTrackNodeDragDropOp::GetHoverText() const
{
	FText HoverText = LOCTEXT("Invalid", "Invalid");

	if (SelectedNodes[0].IsValid())
	{
		HoverText = SelectedNodes[0]->GetEditorNodeData().GetDisplayText();
	}

	return HoverText;
}

#undef LOCTEXT_NAMESPACE
