#pragma once

#include "Widgets/Timeline/NeTimelineTrack.h"
#include "Types/SlateEnums.h"
#include "Widgets/SWidget.h"

class SNeAbilityTimelineTrackWidget;
struct FNeAbilityTrack;
class FNeAbilityTimelineMode;
class SVerticalBox;
class SInlineEditableTextBlock;

/**
 * FNeAbilityTimelineTrack
 *
 * 技能时间轴轨道
 */
class FNeAbilityTimelineTrack final : public FNeTimelineTrack
{
	ANIMTIMELINE_DECLARE_TRACK(FNeAbilityTimelineTrack, FNeTimelineTrack);

public:
	FNeAbilityTimelineTrack(const TSharedPtr<FNeAbilityTimelineMode>& InModel, FNeAbilityTrack& InTrackData,
		const FText& InDisplayName, const FText& InToolTipText);

	//~BEGIN: FNeTimelineTrack interface
	virtual TSharedRef<SWidget> GenerateContainerWidgetForTimeline() override;
	virtual TSharedRef<SWidget> GenerateContainerWidgetForOutliner(const TSharedRef<SNeTimelineOutlinerItem>& InRow) override;
	virtual bool SupportsFiltering() const override { return false; }
	virtual bool SupportsSelection() const override { return true; }
	//~END: FNeTimelineTrack interface

	const FNeAbilityTrack& GetTrackData() const { return TrackData; }
	FNeAbilityTrack& GetTrackData() { return TrackData; }
	const TSharedPtr<SNeAbilityTimelineTrackWidget>& GetTrackWidget() { return TrackWidget; }
	bool IsChildTrack() const { return AttachedParent.IsValid(); }
	void AttachToTrack(const TWeakPtr<FNeAbilityTimelineTrack>& TrackPanel);

	TSharedRef<FNeAbilityTimelineMode> GetAbilityTimelineMode() const;

	/** 处理Track上的Drop事件 */
	FReply OnDropToTrack(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent);

	/** 是否有复合式的节点 */
	bool HasAnyCompoundNode() const;

	void AddNewChildTrackByIndex(UClass* InClass, int32 NodeIndex);
	void AddNewChildTrack(UClass* InClass);
	void RemoveTrack();

	/** Get track display name */
	virtual FText GetLabel() const override;

	/** change the name of track */
	void ChangeTrackName(const FString& NewName);

private:
	/** 刷新Track UI */
	void RefreshTrack();


	void FillNewChildTrackMenu(class FMenuBuilder& MenuBuilder);

	void RefreshOutlinerWidget();

	void InputViewRangeChanged(float ViewMin, float ViewMax);

	void HandleObjectsSelected(const TArray<UObject*>& InObjects);
	// Called when a track changes it's selection; iterates all tracks collecting selected items
	void OnTrackNodeSelectChanged(int32 NodeIndex, int32 InnerNodeIndex);
	void OnDeselectAllTrackNodes();

	void AddNewSegment(UClass* InClass, float InStartTime);
	void RemoveSegment(int32 NodeIndex);

	float GetDesiredHeight() const;


private:
	/** Track Data*/
	FNeAbilityTrack& TrackData;

	/** 当前Track挂接的父Track, 可能是空的*/
	TWeakPtr<FNeAbilityTimelineTrack> AttachedParent = nullptr;

	/** Main widget for track */
	TSharedPtr<SNeAbilityTimelineTrackWidget> TrackWidget;

	/**
	 * 当前Track是否被选中
	 * 由于我们支持单一Track上可以有多个节点
	 * 只要有一个节点被选择则认为Track被选中
	 */
	bool bIsSelecting = false;
};