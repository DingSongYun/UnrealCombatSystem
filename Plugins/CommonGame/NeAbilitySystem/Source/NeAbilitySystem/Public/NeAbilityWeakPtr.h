// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilitySegment.h"
#include "NeAbilitySection.h"
#include "NeAbilityWeakPtr.generated.h"

/************************************************************/
/**
 * FWeakAbilitySectionPtr
 */
struct NEABILITYSYSTEM_API FWeakAbilitySectionPtr
{
protected:
	TWeakObjectPtr<class UNeAbility> OuterAbility = nullptr;

	int32 SectionIndex = INDEX_NONE;

public:
	static FWeakAbilitySectionPtr Create(UNeAbility* Ability, int32 SectionID);

	bool IsValid() const;

	FORCEINLINE int32 GetSectionIndex() const { return SectionIndex; }

	UNeAbility* GetOutter() const { return OuterAbility.Get(); }

	const FNeAbilitySection& Get() const;
		  FNeAbilitySection& Get();

	const FNeAbilitySection* operator->() const { return &Get(); }
		  FNeAbilitySection* operator->()	   { return &Get(); }

	const FNeAbilitySection& operator*() const { return Get(); }
		  FNeAbilitySection& operator*()		  { return Get(); }

	friend bool operator==(const FWeakAbilitySectionPtr& LHS, const FWeakAbilitySectionPtr& RHS)
	{
		return LHS.OuterAbility == RHS.OuterAbility && LHS.SectionIndex == RHS.SectionIndex;
	}
};
typedef FWeakAbilitySectionPtr FNeAbilitySectionPtr;

/************************************************************/
/**
 * FWeakAbilitySegmentPtr
 */
struct NEABILITYSYSTEM_API FWeakAbilitySegmentPtr
{
protected:
	FNeAbilitySectionPtr SectionPtr;
	uint32 SegmentID = 0;

public:
	static FWeakAbilitySegmentPtr Create(const FNeAbilitySectionPtr& Section, uint32 SegmentID);

	bool IsValid() const;
	const FNeAbilitySegment& Get() const;
		  FNeAbilitySegment& Get();

	int32 GetSectionIndex() const { return SectionPtr.GetSectionIndex(); }
	int32 GetSegmentID() const { return SegmentID; }
	FWeakAbilitySegmentPtr GetParentSegment() const;
	const FNeAbilitySectionPtr& GetSectionPtr() const { return SectionPtr; }

	const FNeAbilitySegment* operator->() const { return &Get(); }
		  FNeAbilitySegment* operator->()	   { return &Get(); }

	const FNeAbilitySegment& operator*() const { return Get(); }
		  FNeAbilitySegment& operator*()		  { return Get(); }

	friend bool operator==(const FWeakAbilitySegmentPtr& LHS, const FWeakAbilitySegmentPtr& RHS)
	{
		return LHS.SectionPtr == RHS.SectionPtr && LHS.SegmentID == RHS.SegmentID;
	}

	UNeAbility* GetOutter() const { return SectionPtr.GetOutter(); }

};

typedef FWeakAbilitySegmentPtr FNeAbilitySegmentPtr;

#define MakeWeakSectionPtr(Ability, SectionIndex) FWeakAbilitySectionPtr::Create(Ability, SectionIndex)
#define MakeWeakSegmentPtr(SectionPtr, SegmentID) FNeAbilitySegmentPtr::Create(SectionPtr, SegmentID)

//~=============================================================================
/**
 * FAbilitySegmentReference
 *
 * 存储Segment的索引关系
 * 注意: 这个索引关系在编辑器环境不保证正确性，编辑器环境下请使用 FWeakAbilitySegmentPtr
 */
USTRUCT()
struct NEABILITYSYSTEM_API FNeAbilitySegmentReference
{
	GENERATED_BODY()

public:
	FNeAbilitySegmentReference() : SectionIndex(INDEX_NONE), SegmentIndex(INDEX_NONE) {}
	FNeAbilitySegmentReference(int32 InSectionIndex, int32 InSegmentIndex) : SectionIndex(InSectionIndex), SegmentIndex(InSegmentIndex) {}

	FNeAbilitySegment& Resolve(UNeAbility* Ability);
	const FNeAbilitySegment& Resolve(UNeAbility* Ability) const;

	FORCEINLINE int32 GetSectionIndex() const { return SectionIndex; }
	FORCEINLINE  int32 GetSegmentIndex() const { return SegmentIndex; }

	bool IsValid() const { return SectionIndex != INDEX_NONE && SegmentIndex != INDEX_NONE; }

	friend bool operator==(const FNeAbilitySegmentReference& LHS, const FNeAbilitySegmentReference& RHS)
	{
		return LHS.SectionIndex == RHS.SectionIndex && LHS.SegmentIndex == RHS.SegmentIndex;
	}

private:
	int32 SectionIndex;
	int32 SegmentIndex;
};
