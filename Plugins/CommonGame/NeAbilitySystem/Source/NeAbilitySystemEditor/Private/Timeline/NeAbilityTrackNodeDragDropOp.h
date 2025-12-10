// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SBoxPanel.h"
#include "SNodePanel.h"
#include "SCurveEditor.h"
#include "SNeAbilityTimelineTrackNode.h"

class SNeAbilityTimelineTrackWidget;

class FNeAbilityTrackNodeDragDropOp : public FDragDropOperation
{
public:
	FNeAbilityTrackNodeDragDropOp(float& InCurrentDragXPosition);

	struct FTrackClampInfo
	{
		int32 TrackPos;
		int32 TrackSnapTestPos;
		TSharedPtr<SNeAbilityTimelineTrackWidget> NotifyTrack;
	};

	DRAG_DROP_OPERATOR_TYPE(FNeAbilityTrackNodeDragDropOp, FDragDropOperation)

	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	virtual void OnDragged(const class FDragDropEvent& DragDropEvent) override;

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return Decorator;
	}

	FTrackClampInfo& GetTrackClampInfo(const FVector2D NodePos);
	FText GetHoverText() const;

	static TSharedRef<FNeAbilityTrackNodeDragDropOp> New(const TSharedPtr<SNeAbilityTimelineTrackWidget> InCurNotifyTrack,
		TArray<TSharedPtr<SNeAbilityTimelineTrackNode>> NotifyNodes, TSharedPtr<SWidget> Decorator,
		const TArray<TSharedPtr<SNeAbilityTimelineTrackWidget>>& NotifyTracks, float InViewPlayLength, const FVector2D& CursorPosition,
		const FVector2D& SelectionScreenPosition, const FVector2D& SelectionSize, float& CurrentDragXPosition);

public:

	TSharedPtr<SNeAbilityTimelineTrackWidget> CurNotifyTrack;

	FVector2D							DragOffset;				// Offset from the mouse to place the decorator
	TArray<FTrackClampInfo>				ClampInfos;				// Clamping information for all of the available tracks
	float& CurrentDragXPosition;	// Current X position of the drag operation
	TArray<float>			NodeTimes;				// Times to drop each selected node at

	TArray<TSharedPtr<SNeAbilityTimelineTrackNode>> SelectedNodes;			// The nodes that are in the current selection

	TArray<float>						NodeTimeOffsets;		// Time offsets from the beginning of the selection to the nodes.
	TArray<float>						NodeXOffsets;			// Offsets in X from the widget position to the scrub handle for each node.
	FVector2D							NodeGroupPosition;		// Position of the beginning of the selection
	FVector2D							NodeGroupSize;			// Size of the entire selection
	TSharedPtr<SWidget>		Decorator;				// The widget to display when dragging
	float								SelectionTimeLength;	// Length of time that the selection covers
	int32							TrackSpan;				// Number of tracks that the selection spans

	float ViewPlayLength;
};
