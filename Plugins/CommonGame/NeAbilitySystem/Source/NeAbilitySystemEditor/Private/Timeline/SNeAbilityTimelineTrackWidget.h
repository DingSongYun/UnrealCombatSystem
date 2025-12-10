// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "SNodePanel.h"
#include "SCurveEditor.h"

class SOverlay;
class SScrollBar;
class SBorder;
class SNeAbilityTimelineTrackNode;
class FNeAbilityTimelineMode;
class FNeAbilityTimelineTrack;
struct FNeAbilityTrack;

DECLARE_DELEGATE_RetVal_FiveParams(FReply, FOnNotifyNodesDragStarted, TArray<TSharedPtr<SNeAbilityTimelineTrackNode>>, TSharedRef<SWidget>, const FVector2D&, const FVector2D&, const bool)
DECLARE_DELEGATE_TwoParams(FOnTrackNodeSelectChanged, int32 /**SelectedNode*/, int32 /** SelectedChild*/)
DECLARE_DELEGATE(FOnUpdatePanel)
DECLARE_DELEGATE(FOnDeselectAllTrackNodes)
DECLARE_DELEGATE_TwoParams(FOnAddNewChildTrack, UClass*, int32)
DECLARE_DELEGATE_TwoParams(FOnAddNewNode, UClass*, float)
DECLARE_DELEGATE_OneParam(FOnDeleteNode, int32)

class SNeAbilityTimelineTrackWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityTimelineTrackWidget)
		: _ViewInputMin()
		, _ViewInputMax()
		, _TimelinePlayLength()
		, _OnTrackNodeSelectChanged()
		, _OnUpdatePanel()
		, _OnSetInputViewRange()
		, _OnAddNewChildTrack()
		, _OnAddNewNode()
		, _OnDeleteNode()
	{}

		SLATE_ARGUMENT(const FNeAbilityTrack*, TrackData)
		SLATE_ATTRIBUTE(float, ViewInputMin)
		SLATE_ATTRIBUTE(float, ViewInputMax)
		SLATE_ATTRIBUTE(float, InputMin)
		SLATE_ATTRIBUTE(float, InputMax)
		SLATE_ATTRIBUTE(float, TimelinePlayLength)
		SLATE_ATTRIBUTE(double, FrameRate)
		SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandList)
		SLATE_ARGUMENT(int32, TrackIndex)
		SLATE_EVENT(FOnTrackNodeSelectChanged, OnTrackNodeSelectChanged)
		SLATE_EVENT(FOnUpdatePanel, OnUpdatePanel)
		SLATE_EVENT(FOnSetInputViewRange, OnSetInputViewRange)
		SLATE_EVENT(FOnAddNewChildTrack, OnAddNewChildTrack)
		SLATE_EVENT(FOnAddNewNode, OnAddNewNode)
		SLATE_EVENT(FOnDeleteNode, OnDeleteNode)
		SLATE_EVENT(FOnDeselectAllTrackNodes, OnDeselectAllTrackNodes)
		SLATE_END_ARGS()
public:

	/** Type used for list widget of tracks */
	void Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityTimelineTrack>& InTimelineTrack, const TWeakPtr<FNeAbilityTimelineTrack>& InParentTrack);

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override { UpdateCachedGeometry(AllottedGeometry); }
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	// End of SWidget interface

	/**
	 * Update the nodes to match the data that the panel is observing
	 */
	void RefreshTrackWidget();

	/** Returns the cached rendering geometry of this track */
	const FGeometry& GetCachedGeometry() const { return CachedGeometry; }

	FTrackScaleInfo GetCachedScaleInfo() const { return FTrackScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0.f, 0.f, CachedGeometry.GetLocalSize()); }

	/** Updates sequences when a notify node has been successfully dragged to a new position
	 *	@param Offset - Offset from the widget to the time handle
	 */
	void OffsetNodePosition(TSharedPtr<SNeAbilityTimelineTrackNode> Node, float Offset = 0.0f);

	// Time at the position of the last mouseclick
	float GetLastClickedTime() const { return LastClickedTime; }

	void ClearNodeTooltips();

	const TArray<TSharedPtr<SNeAbilityTimelineTrackNode>>& GetTrackNodes() const { return TrackNodes; }

	/**  Send To Controller for best do */
	void ClearSelections();

protected:
	/** 添加Node菜单 */
	void FillNewNodeMenu(FMenuBuilder& MenuBuilder);
	void CreateNewNodeAtCursor(UClass* NotifyClass);

	/** 添加子节点菜单 */
	void FillNewChildNodeMenu(class FMenuBuilder& MenuBuilder, int32 TrackNodeIndex);
	void AddNewChildNodeTrack(UClass* InClass, int32 TrackNodeIndex);

	/** Attach to 菜单 */
	void FillAttachToNodeMenu(class FMenuBuilder& MenuBuilder, int32 TrackNodeIndex);

	void StartAttachNodeTo(int32 TrackNodeIndex) const;

	void DetachNode(int32 TrackNodeIndex) const;

	void PasteNode(int32 TrackNodeIndex);

	void SortTrack();

	void AlignSectionTimeWithAnimation();

	void AddBreakPoint(int32 NodeIndex);
	void RemoveBreakPoint(int32 NodeIndex);

	/**
	 * Selects a node on the track. Supports multi selection 
	 *
	 * @param TrackNodeIndex		TrackNode的索引
	 * @param InnerNodeIndex		对于Composite的Node，选择的内部Node的序号
	 */
	void SelectTrackObjectNode(int32 TrackNodeIndex, int32 InnerNodeIndex = -1);

	/**  Deselects requested notify node */
	void DeselectTrackObjectNode(int32 TrackNodeIndex);

	/** Handler for delete command */
	void OnDeletePressed();

	/** Deletes all currently selected notifies in the panel */
	void DeleteSelectedNodeObjects();
	void DeleteNodeObjects(int32 TrackNodeIndex);

	int32 GetHitNotifyNode(const FGeometry& MyGeometry, const FVector2D& Position, int32& OutHitedInnerNodeIndex);

	TSharedPtr<SWidget> SummonContextMenu(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	float CalculateTime(const FGeometry& MyGeometry, FVector2D NodePos, bool bInputIsAbsolute = true);

	// Handler that is called when the user starts dragging a node
	FReply OnNotifyNodeDragStarted(TSharedRef<SNeAbilityTimelineTrackNode> NotifyNode, const FPointerEvent& MouseEvent, const FVector2D& ScreenNodePosition, const bool bDragOnMarker, int32 NotifyIndex);

private:
	/** Get Ability timeline mode */
	TSharedRef<FNeAbilityTimelineMode> GetAbilityTimelineMode() const;

	/** 当前track相关的数据 */
	FNeAbilityTrack& GetTrackData() const;

	/** 是否是子Track */
	bool IsChildTrack() const;

	// Store the tracks geometry for later use
	void UpdateCachedGeometry(const FGeometry& InGeometry);

	FMargin GetNotifyTrackPadding(int32 NotifyIndex) const;

	float CalculateDraggedNodePos() const { return CurrentDragXPosition;}

protected:
	/***/
	TWeakPtr<class FNeAbilityTimelineTrack> TimelineTrack;

	/** 如果是子Track，这里不为空 */
	TWeakPtr<class FNeAbilityTimelineTrack> ParentTimelineTrack;

	/** Track上的Node集 */
	TArray<TSharedPtr<SNeAbilityTimelineTrackNode>> TrackNodes;

	TAttribute<float>	ViewInputMin;
	TAttribute<float>	ViewInputMax;
	TAttribute<float>	InputMin;
	TAttribute<float>	InputMax;
	float TimelinePlayLength = 0;
	double FrameRate = 0;

	FOnTrackNodeSelectChanged OnTrackNodeSelectChanged;
	FOnUpdatePanel	 OnUpdatePanel;

	float LastClickedTime = 0;
	float CurrentDragXPosition = 0;

	FOnSetInputViewRange OnSetInputViewRange;
	FOnAddNewChildTrack OnAddChildTrack;
	FOnAddNewNode OnAddNewNode;
	FOnDeleteNode OnDeleteNode;
	FOnDeselectAllTrackNodes OnDeselectAllTrackNodes;

	TSharedPtr<SBorder>		TrackArea;

	/** Cache the SOverlay used to store all this tracks nodes */
	TSharedPtr<SOverlay> NodeSlots;

	/** Cached for drag drop handling code */
	FGeometry CachedGeometry;

	/** Nodes that are currently selected */
	TArray<int32> SelectedNodeIndices;

	/** 选择Node的内部节点时，需要双击选中，防止跟单击冲突，加一个锁 */
	bool bDoubleClickSelectionLock = false;

	TSharedPtr<FUICommandList> CommandList;
};