// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilitySegment.h"
#include "NeAbilitySection.generated.h"

struct FNeAbilitySegment;

/**
 * FNeAbilityTrack
 *
 * 技能Track
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityTrack
{
	GENERATED_BODY()

public:
	FNeAbilityTrack() : TrackName(NAME_None) {}
	explicit FNeAbilityTrack(const FName& InName) : TrackName(InName) {}

	friend bool operator==(const FNeAbilityTrack& LHS, const FNeAbilityTrack& RHS) { return &LHS == &RHS; }

public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FName TrackName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName GroupName = NAME_None;

	/** 是否是子Track */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bIsChildTrack = false;

	/** Array of Segment IDs */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TArray<int32> Segments;
};

/**
 * FNeAbilityTrackGroup
 *
 * 技能Track组
 */
USTRUCT()
struct FNeAbilityTrackGroup
{
	GENERATED_BODY()

public:
	FNeAbilityTrackGroup() : GroupName(NAME_None) {}
	explicit FNeAbilityTrackGroup(const FName& InName) : GroupName(InName) {}

	friend bool operator==(const FNeAbilityTrackGroup& LHS, const FNeAbilityTrackGroup& RHS)
	{
		return LHS.GroupName == RHS.GroupName;
	}

	UPROPERTY()
	FName GroupName;
};

/**
 * FNeAbilitySection
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilitySection
{
	GENERATED_BODY()

	friend class UNeAbility;

public:
	FNeAbilitySection()
	{}

	FNeAbilitySection(FName InName): SectionName(InName)
	{
		FNeAbilitySection();
	}

	/** Get Segment Index of Segment ID*/
	int32 FindSegmentIndexByID(uint32 SegID) const;

	/** Get Segment of Segment ID*/
	FNeAbilitySegment* GetSegmentByID(uint32 SegID);

	/** Get Segment of Segment ID*/
	const FNeAbilitySegment* GetSegmentByID(uint32 SegID) const;

	/** Segment ID 是否有效 */
	bool IsValidSegmentID(uint32 SegmentID) const;

public:
	UPROPERTY(EditDefaultsOnly)
	float SectionDuration = 1.f;

	UPROPERTY(VisibleDefaultsOnly)
	FName SectionName;

	UPROPERTY(VisibleDefaultsOnly)
	int32 NextSection = -1;

	/** Section 中所有Segment */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TArray<FNeAbilitySegment> Segments;

#if WITH_EDITOR
public:
	FNeAbilitySegment& AddNewSegment(uint32 NewSegID);
	void RemoveSegment(const FNeAbilitySegment& Seg);

	FNeAbilityTrack& AddNewTrack();

	/** 添加默认的Group */
	FNeAbilityTrackGroup& AddDefaultTrackGroup();

	const FNeAbilityTrackGroup& GetDefaultTrackGroup() const { check(TrackGroups.Num() > 0); return TrackGroups[0]; }

	FNeAbilityTrackGroup& AddNewTrackGroup();

	/** 获取Track对应的Group */
	const FNeAbilityTrackGroup* FindGroupOfTrack(const FNeAbilityTrack& InTrack) const;

	/** 获取指定Name的Group */
	const FNeAbilityTrackGroup* FindGroupOfName(const FName& InGroupName) const;

	/** 获取Section内所有的Track */
	const TArray<FNeAbilityTrack>& GetAllTracks() const { return Tracks; }
	TArray<FNeAbilityTrack>& GetAllTracks() { return Tracks; }

	/** 获取Group内所有的Track */
	TArray<FNeAbilityTrack*> GetTracks(const FNeAbilityTrackGroup& InGroup);

	/** 获取Segment所在的Track */
	FNeAbilityTrack* FindTrackOfSegment(const FNeAbilitySegment& InSegment);

	/** 获取Segment所在的Track */
	const FNeAbilityTrack* FindTrackOfSegment(const FNeAbilitySegment& InSegment) const;

	/** 获取Track的Index */
	int32 IndexOfTrack(const FNeAbilityTrack& InTrack) const;

	inline bool IsInGroup(const FNeAbilityTrackGroup& Group, const FNeAbilityTrack& Track) const
	{
		return Group.GroupName == Track.GroupName;
	}

	/** 获取所有的Track Group */
	const TArray<FNeAbilityTrackGroup>& GetAllTrackGroups() const { return TrackGroups; }
	TArray<FNeAbilityTrackGroup>& GetAllTrackGroups() { return TrackGroups; }

	/** 查询指定ChildTrack的父Track */
	const FNeAbilityTrack* GetTrackAttachedParent(const FNeAbilityTrack& ChildTrack) const;

#endif

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(VisibleDefaultsOnly, Transient)
	bool bEditorLoop = false;

protected:
	/** 技能轨 */
	UPROPERTY(VisibleDefaultsOnly, AdvancedDisplay)
	TArray<FNeAbilityTrack> Tracks;

	/** 技能轨道组 */
	UPROPERTY(VisibleDefaultsOnly, AdvancedDisplay)
	TArray<FNeAbilityTrackGroup> TrackGroups;
#endif
};