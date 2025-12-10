// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimatedRange.h"
#include "ITimeSlider.h"

class FStructOnScope;
class FNeTimelineTrack;
class FUICommandList;
typedef TSharedRef<FNeTimelineTrack> FTimelineTrackPtr;

/**
 * FNeTimelineMode
 * Timeline数据
 */
class NEEDITORFRAMEWORK_API FNeTimelineMode : public TSharedFromThis<FNeTimelineMode>
{
public:
	FNeTimelineMode(const TSharedRef<FUICommandList>& InCommandList);
	virtual ~FNeTimelineMode() {}
	virtual void Initialize() {}

	//~=============================================================================
	// Timeline Common

	/** Get the current view range */
	FAnimatedRange GetViewRange() const { return ViewRange; }

	/** Set the current view range */
	void SetViewRange(const TRange<double>& InRange);

	/** Get the working range of the model's data */
	FAnimatedRange GetWorkingRange() const { return WorkingRange; }

	/** Get the playback range of the model's data */
	TRange<FFrameNumber> GetPlaybackRange() const;

	/** Get the playback length of the model's data */
	virtual float GetPlayLength() const = 0;

	/** Get the current scrub position */
	FFrameNumber GetScrubPosition() const;

	/** Get the current scrub time */
	virtual float GetScrubTime() const = 0;

	/** Set the current scrub position */
	void SetScrubPosition(FFrameTime NewScrubPosition);

	/** Set the current scrub time*/
	virtual void SetScrubTime(float NewTime) = 0;

	/** Get the framerate specified by the anim sequence */
	virtual double GetFrameRate() const;

	/** Get the tick resolution we are displaying at */
	virtual int32 GetTickResolution() const;

	/** Snap value used to determine scrub resolution of the anim timeline */
	float GetScrubSnapValue() const { return ScrubSnapValue; }

	/** Handle the view range being changed */
	void HandleViewRangeChanged(TRange<double> InRange, EViewRangeInterpolation InInterpolation);

	/** Handle the working range being changed */
	void HandleWorkingRangeChanged(TRange<double> InRange);

	//~=============================================================================
	// Timeline Track oprations

	/** Get the root tracks representing the tree */
	TArray<TSharedRef<FNeTimelineTrack>>& GetRootTracks() { return RootTracks; }
	const TArray<TSharedRef<FNeTimelineTrack>>& GetRootTracks() const { return RootTracks; }

	/** 获取当前鼠标覆盖的Track */
	TSharedPtr<FNeTimelineTrack> GetHoveredTrack() const;

	/**
	 * 递归检测Track&子Track是否处于Hovered
	 * @return 是否Hovered
	 */
	bool IsTrackHovered(TSharedPtr<FNeTimelineTrack> InTrack) const;

	/** Refresh the tracks we have using our underlying asset */
	virtual void RefreshTracks();

	/** Get whether a track is selected */
	bool IsTrackSelected(const TSharedRef<FNeTimelineTrack>& InTrack) const;

	/** Clear all track selection */
	virtual void ClearTrackSelection();

	/** Set whether a track is selected */
	void SetTrackSelected(const TSharedRef<FNeTimelineTrack>& InTrack, bool bIsSelected);

	/** Sort Tracks */
	virtual void SortTrackByStartTime() {}

	/** Update the displayed range if the length of the sequence could have changed */
	virtual void UpdateRange() = 0;

	//~=============================================================================
	// Others

	/** 拖拽到Timeline上 */
	virtual bool OnDropAction(const FDragDropEvent& DragDropEvent) { return false; }

	/** 'Selects' objects and shows them in the details view */
	virtual void SelectObjects(const TArray<UObject*>& Objects);

	virtual void SelectStructs(const TArray<TSharedPtr<FStructOnScope>>& Structs);

	/** In Debugger mode */
	bool IsDebuggerMode() { return bDebuggerMode; }
	void SetDebuggerMode(bool bNewDebuggerMode);

	/** Get the command list */
	TSharedRef<FUICommandList> GetCommandList() const { return CommandList.Pin().ToSharedRef(); }

	/** Delegate fired when tracks have changed */
	DECLARE_EVENT(FNeTimelineMode, FOnTracksChanged)
	FOnTracksChanged& OnTracksChanged() { return OnTracksChangedDelegate; }

	/** Delegate fired when objects have been selected */
	DECLARE_EVENT_OneParam(FNeTimelineMode, FOnHandleObjectsSelected, const TArray<UObject*>& /*InObjects*/)
	FOnHandleObjectsSelected& OnHandleObjectsSelected() { return OnHandleObjectsSelectedDelegate; }

	DECLARE_EVENT_OneParam(FNeTimelineMode, FOnHandleStructsSelected, const TArray<TSharedPtr<FStructOnScope>>& /*InObjects*/)
	FOnHandleStructsSelected& OnHandleStructsSelected() { return OnHandleStructsSelectedDelegate; }

	/** Build a context menu for selected items */
	virtual void BuildContextMenu(class FMenuBuilder& InMenuBuilder);

protected:
	/** Tracks used to generate a tree */
	TArray<TSharedRef<FNeTimelineTrack>> RootTracks;

	/** Tracks that are selected */
	TSet<TSharedRef<FNeTimelineTrack>> SelectedTracks;

	/** The range we are currently viewing */
	FAnimatedRange ViewRange;

	/** The working range of this model, encompassing the view range */
	FAnimatedRange WorkingRange;

	/** The playback range of this model for each timeframe */
	FAnimatedRange PlaybackRange;

	/** Snap value used to determine scrub resolution of the anim timeline */
	float ScrubSnapValue;

	/** Delegate fired when tracks change */
	FOnTracksChanged OnTracksChangedDelegate;

	/** Delegate fired when selection changes */
	FOnHandleObjectsSelected OnHandleObjectsSelectedDelegate;

	FOnHandleStructsSelected OnHandleStructsSelectedDelegate;

	/** Recursion guard for selection */
	bool bIsSelecting;

	/** Is debugger mode */
	bool bDebuggerMode;

	/** The command list we bind to */
	TWeakPtr<FUICommandList> CommandList;
};
