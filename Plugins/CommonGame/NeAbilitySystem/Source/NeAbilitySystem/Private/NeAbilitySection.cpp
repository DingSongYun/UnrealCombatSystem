// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilitySection.h"
#include "AbilitySystemLog.h"
#include "NeAbilitySegment.h"
#include "NeAbility.h"

int32 FNeAbilitySection::FindSegmentIndexByID(uint32 SegID) const
{
	return Segments.IndexOfByPredicate([SegID](const FNeAbilitySegment& Ele) { return Ele.GetID() == SegID; });
}

FNeAbilitySegment* FNeAbilitySection::GetSegmentByID(uint32 SegID)
{
	if (const int32 Index = FindSegmentIndexByID(SegID); Index != INDEX_NONE)
	{
		return &Segments[Index];
	}
	return nullptr;
}

const FNeAbilitySegment* FNeAbilitySection::GetSegmentByID(uint32 SegID) const
{
	if (const int32 Index = FindSegmentIndexByID(SegID); Index != INDEX_NONE)
	{
		return &Segments[Index];
	}
	return nullptr;
}

bool FNeAbilitySection::IsValidSegmentID(uint32 SegmentID) const
{
	return SegmentID > FNeAbilitySegment::INVALID_ID && FindSegmentIndexByID(SegmentID) != INDEX_NONE;
}

#if WITH_EDITOR
FNeAbilitySegment& FNeAbilitySection::AddNewSegment(uint32 NewSegID)
{
	check(NewSegID);
	FNeAbilitySegment& NewSeg = Segments.Add_GetRef(FNeAbilitySegment(NewSegID));
	return NewSeg;

}

void FNeAbilitySection::RemoveSegment(const FNeAbilitySegment& Seg)
{
	const uint32 SegID = Seg.GetID();
	check(SegID);
	for (auto& Track : Tracks)
	{
		if (const int32 Index = Track.Segments.Find(SegID); Index != INDEX_NONE)
		{
			Track.Segments.RemoveAt(Index);
			break;
		}
	}

	const int32 IndexToRemove = Segments.IndexOfByPredicate([&Seg](const FNeAbilitySegment& Ele){return Ele.GetID() == Seg.GetID();});
	if (IndexToRemove != INDEX_NONE)
	{
		Segments.RemoveAt(IndexToRemove);
	}
	else
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Failed remove ability segment from section %s"), *SectionName.ToString());
	}
}

FNeAbilityTrack& FNeAbilitySection::AddNewTrack()
{
	FNeAbilityTrack& NewTrack = Tracks.AddDefaulted_GetRef();
	return NewTrack;
}

FNeAbilityTrackGroup& FNeAbilitySection::AddDefaultTrackGroup()
{
	FNeAbilityTrackGroup& DefaultGrup = TrackGroups.AddDefaulted_GetRef();
	DefaultGrup.GroupName = "Default";
	return DefaultGrup;
}

FNeAbilityTrackGroup& FNeAbilitySection::AddNewTrackGroup()
{
	FNeAbilityTrackGroup& NewTrackGroup = TrackGroups.AddDefaulted_GetRef();
	return NewTrackGroup;
}

const FNeAbilityTrackGroup* FNeAbilitySection::FindGroupOfTrack(const FNeAbilityTrack& InTrack) const
{
	for (const FNeAbilityTrackGroup& Group : TrackGroups)
	{
		if (IsInGroup(Group, InTrack)) return &Group;
	}

	return nullptr;
}

const FNeAbilityTrackGroup* FNeAbilitySection::FindGroupOfName(const FName& InGroupName) const
{
	if (InGroupName.IsNone()) return nullptr;
	for (const FNeAbilityTrackGroup& Group : TrackGroups)
	{
		if (Group.GroupName == InGroupName) return &Group;
	}

	return nullptr;
}

TArray<FNeAbilityTrack*> FNeAbilitySection::GetTracks(const FNeAbilityTrackGroup& InGroup)
{
	TArray<FNeAbilityTrack*> OutTracks;
	for (FNeAbilityTrack& Track : Tracks)
	{
		if (IsInGroup(InGroup, Track))
		{
			OutTracks.Add(&Track);
		}
	}

	return OutTracks;
}

FNeAbilityTrack* FNeAbilitySection::FindTrackOfSegment(const FNeAbilitySegment& InSegment)
{
	for (FNeAbilityTrack& Track : Tracks)
	{
		if (Track.Segments.Contains(InSegment.GetID()))
		{
			return &Track;
		}
	}

	return nullptr;
}

const FNeAbilityTrack* FNeAbilitySection::FindTrackOfSegment(const FNeAbilitySegment& InSegment) const
{
	for (const FNeAbilityTrack& Track : Tracks)
	{
		if (Track.Segments.Contains(InSegment.GetID()))
		{
			return &Track;
		}
	}

	return nullptr;
}

int32 FNeAbilitySection::IndexOfTrack(const FNeAbilityTrack& InTrack) const
{
	return Tracks.IndexOfByPredicate([&] (const FNeAbilityTrack& Ele) { return &Ele == &InTrack; });
}

const FNeAbilityTrack* FNeAbilitySection::GetTrackAttachedParent(const FNeAbilityTrack& ChildTrack) const
{
	check(ChildTrack.bIsChildTrack && ChildTrack.Segments.Num() == 1);
	const FNeAbilitySegment* ChildSegment = GetSegmentByID(ChildTrack.Segments[0]);
	check(ChildSegment && ChildSegment->HasParent());
	const uint32 ParentSegmentID = ChildSegment->GetParentSegment();
	for (const FNeAbilityTrack& Track : Tracks)
	{
		if (Track.Segments.Contains(ParentSegmentID))
		{
			return &Track;
		}
	}

	checkf(false, TEXT("Can not found parent track, something should be wrong."));
	return nullptr;
}
#endif

/************************************************************/
/**
 * FWeakAbilitySectionPtr
 */
bool FWeakAbilitySectionPtr::IsValid() const
{
	return OuterAbility.IsValid() && (SectionIndex >= 0) && SectionIndex < OuterAbility->GetSectionNums();
}

const FNeAbilitySection& FWeakAbilitySectionPtr::Get() const
{
	checkf(IsValid(), TEXT("It is an error to call Get() on an invalid FWeakAbilitySectionPtr. Please either check IsValid()."));
	return OuterAbility->GetSection(SectionIndex);
}

FNeAbilitySection& FWeakAbilitySectionPtr::Get()
{
	checkf(IsValid(), TEXT("It is an error to call Get() on an invalid FWeakAbilitySectionPtr. Please either check IsValid()."));
	return OuterAbility->GetSection(SectionIndex);
}

/************************************************************/
/**
 * FWeakAbilitySegmentPtr
 */
bool FWeakAbilitySegmentPtr::IsValid() const
{
	return SectionPtr.IsValid() && SegmentID > 0 && SectionPtr->FindSegmentIndexByID(SegmentID) > INDEX_NONE;
}

const FNeAbilitySegment& FWeakAbilitySegmentPtr::Get() const
{
	checkf(IsValid(), TEXT("It is an error to call Get() on an invalid FWeakAbilitySegmentPtr. Please either check IsValid()."));
	return *SectionPtr->GetSegmentByID(SegmentID);
}

FNeAbilitySegment& FWeakAbilitySegmentPtr::Get()
{
	checkf(IsValid(), TEXT("It is an error to call Get() on an invalid FWeakAbilitySegmentPtr. Please either check IsValid()."));
	return *SectionPtr->GetSegmentByID(SegmentID);
}

FWeakAbilitySegmentPtr FWeakAbilitySegmentPtr::GetParentSegment() const
{
	check(Get().HasParent());
	return MakeWeakSegmentPtr(SectionPtr, Get().GetParentSegment());
}

FWeakAbilitySectionPtr FWeakAbilitySectionPtr::Create(UNeAbility* Ability, int32 SectionID)
{
	FWeakAbilitySectionPtr SectionPtr;
	SectionPtr.OuterAbility = Ability;
	SectionPtr.SectionIndex = SectionID;

	return SectionPtr;
}

FWeakAbilitySegmentPtr FWeakAbilitySegmentPtr::Create(const FNeAbilitySectionPtr& Section, uint32 SegmentID)
{
	FWeakAbilitySegmentPtr SegmentPtr;
	SegmentPtr.SectionPtr = Section;
	SegmentPtr.SegmentID = SegmentID;
	return SegmentPtr;
}