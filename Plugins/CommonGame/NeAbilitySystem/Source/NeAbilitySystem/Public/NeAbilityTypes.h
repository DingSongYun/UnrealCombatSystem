// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "Misc/EnumRange.h"
#include "Abilities/GameplayAbility.h"
#include "NeAbilityTypes.generated.h"

/**
 * ESegmentTriggerMode
 * 子Segment的触发模式
 */
UENUM(BlueprintType, Meta = (Bitflags))
enum class ESegmentTriggerMode : uint8
{
	Immediately = 0					UMETA(DisplayName = "条件成立->立即触发"),
	FollowTimeline					UMETA(DisplayName = "条件成立->按时间轴触发"),
};

/**
 * EAbilitySegmentDurationType
 *
 * AbilitySegment的duration类型
 */
UENUM()
enum class EAbilityDurationPolicy : uint8
{
	Instant,
	Infinite,
	HasDuration
};

/**
 * 技能输入同步策略
 */
UENUM(BlueprintType, Meta = (Bitflags))
enum class EAbilityInputReplicatePolicy : uint8
{
	Pressed,
	Released,
	Holding,
};

/** EBeamEffectiveType: 技能节点的生效期 */
UENUM()
enum class EBeamEffectiveType : uint8
{
	/** 默认: 仅在时间轴内生效，当前时间轴结束，节点结束执行 */
	WithinTimeline	= 0			UMETA(DisplayName = "时间轴内生效"),
	/** 仅在技能时间轴内生效，技能时间轴结束，节点结束执行。比如可以让Section内的节点在当前Section结束后继续保持执行 */
	WithinAbility	= 1			UMETA(DisplayName = "技能内生效"),
	/** 节点自己控制， 技能时间轴跑完后也可能继续生效, 这个慎用 */
	OnDemand		= 2			UMETA(DisplayName = "技能节点自己控制"),
};

/**
 * 结束原因
 */
UENUM(BlueprintType)
enum class EAbilityBeamEndReason : uint8
{
	Finished			= 0				UMETA(DisplayName = "正常执行结束"),
	SectionEnd							UMETA(DisplayName = "Section执行结束"),
	AbilityEnd							UMETA(DisplayName = "技能执行结束"),
	Interrupt							UMETA(DisplayName = "被外部事件中断"),
	PredictionFailure					UMETA(DisplayName = "预测失败"),
	PreviewOver							UMETA(DisplayName = "预览拖拽")
};

/** 施放技能检测策略 */
UENUM(BlueprintType)
enum class EActivateAbilityCheckMethod : uint8
{
	/** 默认，根据技能施放条件判断是否可以进行施放. */
	Undefined		= 0		UMETA(DisplayName = "Default"),
	/** 始终施放 */
	AlwaysActivate			UMETA(DisplayName = "Always Activate Ability, Ignore Condition Check"),
	/** 如果有其他技能在施放，始终打断其他技能 */
	AlwaysInterrupt			UMETA(DisplayName = "Always interrupt other abilities"),
	/** 不进行技能条件检测，但是会受当前正在施放的技能的可打断情况影响 */
	IgnoreConditionCheck	UMETA(DisplayName = "Ignore Ability Activate Condition")
};

/** Beam上目标策略选择 */
UENUM(BlueprintType, Meta = (Bitflags))
enum class EBeamTargetPolicy : uint8
{
	Owner					= 0					UMETA(DisplayName = "技能的持有者"),
	LockTargets									UMETA(DisplayName = "锁定的目标"),
	CollisionResult								UMETA(DisplayName = "碰撞检测结果"),
	Instigator									UMETA(DisplayName = "技能的始作俑者"),
	CustomTarget								UMETA(DisplayName = "自定义目标"),
	Count UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EBeamTargetPolicy, EBeamTargetPolicy::Count);

/** Beam的网络执行策略 */
UENUM(BlueprintType)
enum class EBeamReplicationPolicy : uint8
{
	// We don't replicate the beam of the ability to anyone.
	ClientOnly				UMETA(DisplayName = "仅客户端执行"),
	ClientPredicted			UMETA(DisplayName = "客户端预测执行"),
	ServerOnly				UMETA(DisplayName = "仅在服务器执行"),
	ServerExec				UMETA(DisplayName = "服务器执行, 同步到客户端"),
};

UENUM(BlueprintType)
enum class EBeamEvalSpace : uint8
{
	// We don't replicate the beam of the ability to anyone.
	Local					UMETA(DisplayName = "本地执行"),
	Remote					UMETA(DisplayName = "远端执行"),
	Abort					UMETA(DisplayName = "不执行"),
};

/**
 * FNeCollisionQueryResult
 * 技能过程种的碰撞检测结果
 */
USTRUCT(BlueprintType)
struct FNeCollisionQueryResult
{
	GENERATED_USTRUCT_BODY()

	FNeCollisionQueryResult()
	{
		ResultList.Empty(10);
	}

	/** 碰撞检测起始点 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector TraceOrigin = FVector::ZeroVector;

	/** 碰撞体朝向 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FQuat TraceRotation = FQuat::Identity;

	/** 碰撞体大概大小 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector TraceBound = FVector::ZeroVector;

	/** 碰撞检测方向 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector TraceDirection = FVector::ZeroVector;

	// 碰撞信息列表
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FHitResult> ResultList;
};

/**
 * FNeSlottedAbility
 * 可关联tag的技能配置
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeSlottedAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(Categories="TriggerTagCategory"))
	FGameplayTag SlotTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel = 0;
};
