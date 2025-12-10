// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "EditorUndoClient.h"
#include "NeAbilityEditorTypes.h"
#include "NeAbilitySection.h"
#include "NeAbilitySegment.h"
#include "NeAbilityWeakPtr.h"
#include "Widgets/Timeline/NeTimelineMode.h"

class FNeAbilityTimelineTrack;
struct FNeAbilitySegmentDef;
struct FNeAbilityTrackGroup;
class UNeAbility;
struct FNeAbilityTrack;
class FUICommandList;
class FNeAbilityBlueprintEditor;

class FNeAbilityTimelineMode : public FNeTimelineMode, public FEditorUndoClient /*, public FGCObject*/
{
public:
	FNeAbilityTimelineMode(const TSharedPtr<FNeAbilityBlueprintEditor>& InAssetEditorToolkit, int32 InSectionIndex, const TSharedRef<FUICommandList>& InCommandList);
	virtual ~FNeAbilityTimelineMode() override;

	//~BEGIN: FNeTimelineMode interface
	virtual void Initialize() override;
	virtual float GetPlayLength() const override;
	virtual double GetFrameRate() const override;
	virtual float GetScrubTime() const override;
	virtual void SetScrubTime(float NewTime) override;
	virtual void SortTrackByStartTime() override;
	virtual void UpdateRange() override;
	virtual void RefreshTracks() override;
	virtual void ClearTrackSelection() override;
	virtual void SelectObjects(const TArray<UObject*>& Objects) override;
	virtual bool OnDropAction(const FDragDropEvent& DragDropEvent) override;
	//~END: FNeTimelineMode interface

	int32 GetPlayFrameCount() const;

	/** FEditorUndoClient interface */
	virtual void PostUndo(bool bSuccess) override { HandleUndoRedo(); }
	virtual void PostRedo(bool bSuccess) override { HandleUndoRedo(); }

	//~=============================================================================
	// 技能相关操作
	UNeAbility* GetEditingAbility() const;

	FNeAbilitySection& GetAbilitySection() const;
	FNeAbilitySegment& GetAbilitySegment(uint32 SegmentID) const;
	FNeAbilitySectionPtr GetAbilitySectionPtr() const;
	FNeAbilitySegmentPtr GetAbilitySegmentPtr(uint32 SegmentID) const;
	TSharedPtr<FNeAbilityBlueprintEditor> GetAbilityEditor() const { return AbilityEditor.Pin(); }
	TSharedPtr<class FNeAbilityEditorPlayerBase> GetPreviewPlayer() const;

	void SetSectionDuration(float NewDuration);
	void SetSectionName(FName NewName);
	bool GetSectionLoop() const;
	void SetSectionLoop(bool bLoop);


	/** 标记修改 */
	void Modify();

	//~=============================================================================
	// Segment操作

	/** 进入鼠标选择TrackNode的模式 */
	DECLARE_DELEGATE_OneParam(FOnPickNodeDelegate, FNeAbilitySegmentPtr&)
	void EnterNodePickMode(FOnPickNodeDelegate OnPickCallback);

	void ExitNodePickMode();

	/** 是否处于鼠标选择TrackNode的模式 */
	bool IsNodePickMode() const { return bNodePickMode; }

	//~=============================================================================
	// Timeline Track 相关操作

	/** Get all timeline tracks */
	TArray<TSharedRef<FNeAbilityTimelineTrack>> GetAllTimelineTracks() const { return TimelineTracks;}

	/** 获取指定Ability Track对应的timeline track */
	TSharedPtr<FNeAbilityTimelineTrack> GetTimelineTrack(const FNeAbilityTrack& Track) const;

	/** change the name of track group */
	void ChangeTrackGroupName(const FNeAbilityTrackGroup& TrackGroup, FName Name) const;

	/** Change the name of specific track */
	void ChangeTrackName(const FNeAbilityTrack& TrackData, FName Name) const;

	/** 新加一个Group */
	FNeAbilityTrackGroup& AddNewTrackGroup(const FName& GroupName = NAME_None);

	/** 删除指定Group */
	void DeleteTrackGroup(const FNeAbilityTrackGroup& TrackGroup);

	/** Track Drag to new Group */
	void SwapTrackToGroup(FNeAbilityTrack& SrcTrack, const FNeAbilityTrackGroup& DestGroupData);

	/** Track Drag to other Track Swap */
	void SwapTrackPosition(FNeAbilityTrack& SrcTrack, bool bSrcChildTrack, FNeAbilityTrack& DestTrack, bool bDestChildTrack);

	/** TrackNode Drag to new Track */
	void SwapNodeToTrack(const FNeAbilitySegmentPtr& SrcNode, bool bSrcChildNode, const FNeAbilityTrack& DestSrcTrack, bool bDestChildNode);

	/**
	 * 将Track插入到指定位置
	 *
	 * @param AtPosition	 在Group中的位置
	 */
	void MoveTrackTo(const FNeAbilityTrackGroup& AtGroup, FNeAbilityTrack& Track, int32 AtPosition);

	/** 将 Track 插入到 DestTrack 前面 */
	void MoveTrackBefore(FNeAbilityTrack& Track, FNeAbilityTrack& DestTrack);

	/** 将 SrcTrack 插入到 DestTrack 后面 */
	void MoveTrackAfter(FNeAbilityTrack& Track, FNeAbilityTrack& DestTrack);

	/** Add new empty track */
	FNeAbilityTrack& AddNewTrack(const FNeAbilityTrackGroup& AtGroup, const FName& InTrackName, int32 AtPosition = -1);

	/** Add new segment on track */
	FNeAbilitySegment& AddNewSegment(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentDef& SegmentDef);

	/** Remove segment from track */
	void RemoveSegment(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentPtr& InSegment);

	/** 在Group上新加Track，并添加Segment */
	FNeAbilityTrack& AddNewTrack(const FNeAbilityTrackGroup& AtGroup, const FNeAbilitySegmentDef& SegmentDef);

	/** Add sub Track */
	FNeAbilityTrack& AddSubTrack(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentDef& SegmentDef);

	/** 删除Track */
	bool RemoveTrack(FNeAbilityTrack& AtTrack);

	/** 删除Track内的节点 */
	bool RemoveTrackNode(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentPtr& SegmentPtr);

	/** 查询Segment的执行状态，Debugger时候有用 */
	EAbilityNodeExecutionState GetTrackNodeExecutionState(const FNeAbilitySegmentPtr& SegmentPtr) const;

	/**
	 * 将Segment作为子Segment挂接到另一个Track上
	 *
	 * @param InParent			新的父的Segment
	 * @param InSegment			被挂接的Segment
	 */
	void AttachTo(FNeAbilitySegmentPtr& InParent, FNeAbilitySegmentPtr& InSegment);

	/**
	 * 解除父挂接
	 */
	void Detach(FNeAbilitySegmentPtr& InSegment);

	/** 将Section时间与Segment对齐 */
	void AlignSectionTimeWithSegment(const FNeAbilitySegmentPtr& InSegmentPtr);

	/** 通过montage对齐section 时间 */
	void AlignSectionTimeWithAnimation(bool bAllSection);

	/** Snap ViewRange to PlayRange */
	void SnapViewToPlayRange();

	//~=============================================================================
	// Debugger Actions
public:
	/** 在指定Segment上添加断点 */
	void AddBreakPoint(const FNeAbilitySegmentPtr& InSegment) const;

	/** 移除Segment上的断点 */
	void RemoveBreakPoint(const FNeAbilitySegmentPtr& InSegment) const;

	/** Segment上是否有设置断点 */
	bool HasBreakpointToggled(const FNeAbilitySegmentPtr& SegmentPtr) const;

	/** Segment上断点是否触发 */
	bool IsBreakpointTriggered(const FNeAbilitySegmentPtr& SegmentPtr) const;

public:
	//~=============================================================================
	// Command Actions

	/** 拷贝当前选中的Segment */
	void CopySelectionSegments();

	/** 复制当前选中的Segment */
	void DuplicateSelectionSegment();

	/** 粘贴 */
	void DoPaste();

	/** 粘贴 */
	void DoPasteNodeToTrack(FNeAbilityTrack* TrackData, bool bChildTrack, float StartTime);

private:
	void HandleUndoRedo();

private:
	TWeakPtr<FNeAbilityBlueprintEditor> AbilityEditor;

	/** All tracks */
	TArray<TSharedRef<FNeAbilityTimelineTrack>> TimelineTracks;

	int32 SectionIndex;
	FNeAbilitySectionPtr SectionPtr;

	/** Node选择模式 */
	uint8 bNodePickMode : 1;
	/** 选中Node后回调 */
	FOnPickNodeDelegate OnPickNodeCallback;
};
