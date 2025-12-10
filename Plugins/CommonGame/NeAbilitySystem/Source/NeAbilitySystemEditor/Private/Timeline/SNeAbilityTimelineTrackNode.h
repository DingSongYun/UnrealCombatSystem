// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilitySection.h"
#include "NeAbilityWeakPtr.h"
#include "Misc/Attribute.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SBoxPanel.h"
#include "SNodePanel.h"
#include "SCurveEditor.h"

struct FNeAbilitySegment;
struct FNeAbilityTrack;

typedef FNeAbilitySegmentPtr FTrackNodeDataPtr;

DECLARE_DELEGATE_RetVal_FourParams(FReply, FOnNotifyNodeDragStarted, TSharedRef<class SNeAbilityTimelineTrackNode>, const FPointerEvent&, const FVector2D&, const bool)
DECLARE_DELEGATE_TwoParams(FOnTrackNodeSelectChanged, int32, int32)
DECLARE_DELEGATE(FOnUpdatePanel)

namespace ENotifyStateHandleHit
{
	enum Type
	{
		Start,
		End,
		None
	};
}


struct FNeAbilityTrackNodeData
{
	FNeAbilityTrackNodeData();

	void Initialize(FTrackNodeDataPtr InData, float InFrameRate);
	const FTrackNodeDataPtr& GetNodeData() const { return InnerData; }

	void SetStartTime(float Time, bool bSnapped = false);
	void SetDuration(float InDuration);
	float GetStartTime() const;
	float GetDuration()  const;
	bool IsEnable() const ;
	void ToggleEnable();

	bool IsInstantType() const;
	bool HasDuration() const;

	void Snapped();

	void Modify();

	FText GetDisplayText() const;
	TOptional<FLinearColor> GetEditorColor()  const;
	FText GetNodeTooltip()  const;

	FTrackNodeDataPtr InnerData;
	double FrameRate;
};

class SNeAbilityTimelineTrackNode final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityTimelineTrackNode)
		:_ViewInputMin()
		, _ViewInputMax()
		, _OnNodeDragStarted()
		, _OnUpdatePanel()
		, _OnTrackNodeSelectChanged()
	{
	}

	SLATE_ARGUMENT(FTrackNodeDataPtr, NodeData)
	SLATE_ATTRIBUTE(int32, NodeIndex)
	SLATE_ATTRIBUTE(float, ViewInputMin)
	SLATE_ATTRIBUTE(float, ViewInputMax)
	SLATE_ATTRIBUTE(float, TimelinePlayLength)
	SLATE_ATTRIBUTE(double, FrameRate)
	SLATE_EVENT(FOnNotifyNodeDragStarted, OnNodeDragStarted)
	SLATE_EVENT(FOnUpdatePanel, OnUpdatePanel)
	SLATE_EVENT(FOnTrackNodeSelectChanged, OnTrackNodeSelectChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& Declaration, TWeakPtr<class FNeAbilityTimelineMode> InController, bool IsChildNode, bool IsDebugger);

	// SWidget interface
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	// End of SWidget interface

	// SNodePanel::SNode interface
	void UpdateSizeAndPosition(const FGeometry& AllottedGeometry);
	FVector2D GetWidgetPosition() const;
	FVector2D GetNotifyPosition() const;
	FVector2D GetNotifyPositionOffset() const;
	FVector2D GetSize() const;
	float GetDesiredHeight() const;
	bool HitTest(const FGeometry& AllottedGeometry, FVector2D MouseLocalPose, int32& OutHitInnerNodeIndex) const;

	// Extra hit testing to decide whether or not the duration handles were hit on a state node
	ENotifyStateHandleHit::Type DurationHandleHitTest(const FVector2D& CursorScreenPosition) const;
	// End of SNodePanel::SNode

	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const /*override*/;

	/**
	 * 绘制组合Node
	 */
	virtual int32 OnPaintCompositeNodes(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;

	void DrawScrubHandle(float ScrubHandleCentre, FSlateWindowElementList& OutDrawElements, int32 ScrubHandleID, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FLinearColor NodeColour) const;
	/** Helpers to draw scrub handles and snap offsets */
	void DrawHandleOffset(const float& Offset, const float& HandleCentre, FSlateWindowElementList& OutDrawElements, int32 MarkerLayer, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FLinearColor NodeColour) const;

	void DrawBreakPoint(float ScrubHandleCentre, FSlateWindowElementList& OutDrawElements, int32 ScrubHandleID, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FLinearColor NodeColour) const;

	FLinearColor GetNodeColor() const;
	FText GetNotifyText() const;

	void DropCancelled();

	/** Returns the size of this notifies duration in screen space */
	float GetDurationSize() const { return NotifyDurationSizeX; }

	/** Scrub Width */
	float GetScrubHandleWidth() const;

	/** Sets the position the mouse was at when this node was last hit */
	void SetLastMouseDownPosition(const FVector2D& CursorPosition) { LastMouseDownPosition = CursorPosition; }

	/** The minimum possible duration that a notify state can have */
	static const float MinimumStateDuration;

	const FVector2D& GetScreenPosition() const
	{
		return ScreenPosition;
	}

	bool BeingDragged() const { return bBeingDragged; }
	bool IsSelected() const { return bSelected; }
	bool IsSelected(int32 ChildIndex) const { return bSelected && ChildIndex == SelectedSubNode; }
	void SetSelected (bool bNewSelected, int32 InSelectChild = -1)
	{ 
		bSelected = bNewSelected; 
		SelectedSubNode = InSelectChild;
	}
	const FNeAbilityTrackNodeData& GetEditorNodeData() const { return EditorNodeData; }
	FNeAbilityTrackNodeData* GetEditorNodeDataPtr()  { return &EditorNodeData; }

	const FTrackNodeDataPtr& GetTrackNodeData() const { return EditorNodeData.GetNodeData(); }
	bool IsChildNode() const { return bIsChildNode; }

	void CacheGeometry(const FGeometry& Geometry) { CachedTrackGeometry = Geometry; }

private:
	FText GetNodeTooltip() const;

	/** Detects any overflow on the anim notify track and requests a track pan */
	float HandleOverflowPan(const FVector2D& ScreenCursorPos, float TrackScreenSpaceXPosition, float TrackScreenSpaceMin, float TrackScreenSpaceMax);

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

private:

	TWeakPtr<class FNeAbilityTimelineMode> WeakTimelineMode;

	FNeAbilityTrackNodeData EditorNodeData;

	FSlateFontInfo Font;

	TAttribute<float>			ViewInputMin;
	TAttribute<float>			ViewInputMax;
	float TimelinePlayLength;
	int32 NodeIndex;

	FVector2D					CachedAllotedGeometrySize;
	FVector2D					ScreenPosition;

	bool						bDrawTooltipToRight;
	bool						bBeingDragged;

	bool						bSelected = false;
	int32						SelectedSubNode = -1;

	// Index for undo transactions for dragging, as a check to make sure it's active
	int32						DragMarkerTransactionIdx;

	/** The scrub handle currently being dragged, if any */
	ENotifyStateHandleHit::Type CurrentDragHandle;

	float						NotifyTimePositionX;
	float						NotifyDurationSizeX;
	float						NotifyScrubHandleCentre;

	FVector2D					TextSize;
	float						LabelWidth;

	bool bIsChildNode;
	bool bIsDebugger;

	/** Last position the user clicked in the widget */
	FVector2D					LastMouseDownPosition;

	/** Delegate that is called when the user initiates dragging */
	FOnNotifyNodeDragStarted	OnNodeDragStarted;


	/** Delegate to signal selection changing */
	FOnTrackNodeSelectChanged	OnTrackNodeSelectChanged;

	/** Delegate to redraw the notify panel */
	FOnUpdatePanel				OnUpdatePanel;

	/** Cached owning track geometry */
	FGeometry CachedTrackGeometry;
};
