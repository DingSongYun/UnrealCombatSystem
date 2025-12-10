// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/Timeline/NeTimelineTrack.h"
#include "Types/SlateEnums.h"

class SInlineEditableTextBlock;
class FNeAbilityTimelineMode;
class SNeTimelineOutlinerItem;
struct FNeAbilityTrackGroup;

/**
 * FNeAbilityTimelineGroupTrack
 *
 * TrackGroup 对应的时间轴轨道
 */
class FNeAbilityTimelineGroupTrack : public FNeTimelineTrack
{
	ANIMTIMELINE_DECLARE_TRACK(FNeAbilityTimelineGroupTrack, FNeTimelineTrack);

public:
	FNeAbilityTimelineGroupTrack(const TSharedPtr<FNeAbilityTimelineMode>& InModel, const FNeAbilityTrackGroup& InGroupData, const FText& InDisplayName, const FText& InToolTipText);

	//~BEGIN: FNeTimelineTrack interface
	virtual TSharedRef<SWidget> GenerateContainerWidgetForOutliner(const TSharedRef<SNeTimelineOutlinerItem>& InRow) override;
	virtual bool CanRename() const override { return true; }
	virtual void RequestRename() override;
	virtual bool SupportsSelection() const override { return true; }
	//~END: FNeTimelineTrack interface

	/** 技能轨道组数据 */
	const FNeAbilityTrackGroup& GetGroupData() const { return GroupData; }

private:
	TSharedPtr<FNeAbilityTimelineMode> GetAbilityTimelineMode() const;

	/** 组下拉菜单 */
	TSharedRef<SWidget> BuildGroupSubMenu();

	/** 添加新的技能Track组 */
	void AddTrackGroup() const;

	/** 删除本技能Track组*/
	void DeleteTrackGroup() const;

	/** 在当前组内添加新的轨道 */
	void AddNewTrack(UClass* InActionClass) const;

	/** 修改组名称 */
	void OnCommitGroupName(const FText& InText, ETextCommit::Type CommitInfo) const;

	/** 组名称*/
	virtual FText GetLabel() const override;

private:
	const FNeAbilityTrackGroup& GroupData;

	TSharedPtr<SInlineEditableTextBlock> EditableGroupName;
};