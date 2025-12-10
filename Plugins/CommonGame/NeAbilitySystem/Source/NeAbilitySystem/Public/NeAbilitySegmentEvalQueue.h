// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "NeAbilityWeakPtr.h"
#include "NeAbilityTargetData.h"
#include "NeAbilitySegmentEvalQueue.generated.h"

//~=============================================================================
/**
 * FNeAbilitySegmentEval
 * Segment执行体
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilitySegmentEvalContext : public FFastArraySerializerItem
{
	GENERATED_BODY()
public:
	FNeAbilitySegmentEvalContext() {}
	FNeAbilitySegmentEvalContext(const FNeAbilitySegmentReference& InSegment): Segment(InSegment) {}
	~FNeAbilitySegmentEvalContext() {}

	//~BEGIN: FFastArraySerializerItem interface
	void PreReplicatedRemove(const struct FAbilitySegmentQueue& InArraySerializer);
	void PostReplicatedAdd(const struct FAbilitySegmentQueue& InArraySerializer);
	//~END: FFastArraySerializerItem interface

	bool IsValid() const { return Segment.IsValid(); }
	const FNeAbilitySegment& GetSegment() const;

	/** 是否该同步 */
	bool ShouldReplicate() const;

	class UAbilitySystemComponent* GetAbilitySystemComponent() const;
	AActor* GetOwningActor() const;
	AActor* GetInstigator() const;
	bool IsLocallyControlled() const;
	bool IsNetAuthority() const;
	void NotifySegmentChildrenTriggered();

	void EvaluatePropertyBindings();

	FORCEINLINE FNeAbilitySegmentEvalContext& operator=(const FNeAbilitySegmentEvalContext& Other)
	{
		Segment = Other.Segment;
		Ability = Other.Ability;
		BeamInstance = Other.BeamInstance;
		TargetingInfos = Other.TargetingInfos;
		Segment = Other.Segment;
		LastEvalPropertyBindingFrame = Other.LastEvalPropertyBindingFrame;
		RuntimeBindings = Other.RuntimeBindings;
#if WITH_EDITOR
		GizmoActor = Other.GizmoActor;
#endif

		return *this;
	}

	FString ToString() const;

	friend bool operator==(const FNeAbilitySegmentEvalContext& LHS, const FNeAbilitySegmentEvalContext& RHS)
	{
		return LHS.Segment == RHS.Segment
			&& LHS.Ability == RHS.Ability
			&& LHS.BeamInstance == RHS.BeamInstance;
	}

public:
	UPROPERTY()
	TObjectPtr<UNeAbility> Ability;

	UPROPERTY()
	TObjectPtr<UNeAbilityBeam> BeamInstance;

	UPROPERTY()
	TArray<FNeAbilityTargetingInfo> TargetingInfos;

	UPROPERTY()
	FNeAbilitySegmentReference Segment;

	UPROPERTY()
	uint32	LastEvalPropertyBindingFrame = -1;

	/** 属性绑定信息 */
	TArray<TSharedRef<class FNeAbilityPropertyBindingEval>> RuntimeBindings;

#if WITH_EDITOR
	/** Gizmo Actor when preview in editor */
	TWeakObjectPtr<class ANeAbilityGizmoActor> GizmoActor;
#endif
};

//~=============================================================================
/**
 * FAbilitySegmentQueue
 * Segment执行队列
 */
USTRUCT()
struct FAbilitySegmentQueue : public FFastArraySerializer
{
	GENERATED_BODY()
public:
	/** 添加Segment到执行队列 */
	void AddSegment(int32 SectionIndex, int32 SegmentIndex);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FNeAbilitySegmentEvalContext, FAbilitySegmentQueue>(Evaluating, DeltaParms, *this);
	}

	template< typename Type, typename SerializerType >
	bool ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient)
	{
		if (!Item.ShouldReplicate())
		{
			return false;
		}

		if (bIsWritingOnClient)
		{
			return Item.ReplicationID != INDEX_NONE;
		}

		return true;
	}

	FORCEINLINE void IncrementSegmentReadyListLock() { SegmentReadyScopeLockCount++; }
	void DecrementSegmentReadyListLock();

public:
	int32 SegmentReadyScopeLockCount;

	/** 正在执行的Segment */
	UPROPERTY()
	TArray<FNeAbilitySegmentEvalContext> Evaluating;

	/** 当帧需要执行的Segment队列 */
	UPROPERTY(NotReplicated)
	TArray<FNeAbilitySegmentReference> Ready;

	UPROPERTY(NotReplicated)
	TArray<FNeAbilitySegmentReference> PendingReady;
};

template<>
struct TStructOpsTypeTraits< FAbilitySegmentQueue > : public TStructOpsTypeTraitsBase2< FAbilitySegmentQueue >
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};