// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NeAbilityTypes.h"
#include "DataBoard/NeAbilityDataBoard.h"
#include "Templates/SubclassOf.h"
#include "NeAbilityBeam.generated.h"

class UNeAbility;
class UNeAbilitySystemComponent;
struct FNeAbilitySegmentEvalContext;

/** Beam 运行状态 */
UENUM(BlueprintType)
enum class EAbilityBeamState : uint8
{
	Idle,
	Running,
	Finished,
	Cancelled
};

/**
 * UNeAbilityBeam
 *
 * 技能节点, 主要放一些技能的模块化逻辑
 *
 */
UCLASS(config=Ability, abstract, Blueprintable, const, hidecategories=Object, collapsecategories)
class NEABILITYSYSTEM_API UNeAbilityBeam : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~BEGIN: UObject interface
	virtual void PostLoad() override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	//~END: UObject interface

	EAbilityDurationPolicy GetDurationPolicy() const { return DurationType; }

	UFUNCTION(BlueprintPure)
	float GetDuration() const { return Duration; }

	UFUNCTION(BlueprintCallable)
	void SetDuration(float NewDuration);

	/** BeamInstance 运行时间 */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetRunningTime() const { return RunningTime; }

	UFUNCTION(BlueprintPure)
	bool IsInstant() const;

	UFUNCTION(BlueprintPure)
	bool IsCompound() const { return bCompound; }

	virtual void InitInstanceFor(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& EvalContext);

	/** 开始执行Beam */
	void Active(FNeAbilitySegmentEvalContext& EvalContext);

	/** Update */
	void Update(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext);

	/** 结束执行 */
	void End(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason);

	/** For Preview */
	virtual void SamplePosition(const float Position, const float PreviousPosition);

	UFUNCTION(BlueprintPure)
	bool ShouldFinish() const;

	UFUNCTION(BlueprintPure)
	bool HasFinished() const;

	UFUNCTION(BlueprintCallable)
	void RequestEnd() { bRequestFinished = true; }

	/** 执行实例是否需要复制 */
	UFUNCTION(BlueprintCallable)
	bool ShouldReplicate() const;

	/** 获取Beam的执行域: 本地/远端/不执行 */
	EBeamEvalSpace GetBeamEvalSpace(const struct FGameplayAbilityActorInfo& ActorInfo) const;

	/** Debug String */
	virtual FString GetDebugString() const;

	/** function for getting UWorld */
	virtual UWorld* GetWorld() const override;

	/** Get instigator actor from ability */
	UFUNCTION(BlueprintPure)
	AActor* GetInstigator() const;

	/** Get owner actor from ability */
	UFUNCTION(BlueprintPure)
	AActor* GetOwnerActor() const;

	/** Get avatar actor if exist*/
	UFUNCTION(BlueprintPure)
	AActor* GetAvatarActor() const;

	/** 获取目标Actor */
	UFUNCTION(BlueprintPure)
	TArray<AActor*> GetTargetActors(const FNeAbilitySegmentEvalContext& EvalContext) const;

	UFUNCTION(BlueprintPure)
	AActor* GetTargetActor(const FNeAbilitySegmentEvalContext& EvalContext) const;

	/** 获取目标信息 */
	UFUNCTION(BlueprintPure)
	const TArray<struct FNeAbilityTargetingInfo>& GetTargetInfos(const FNeAbilitySegmentEvalContext& EvalContext) const;

	UFUNCTION(BlueprintCallable)
	void DeclareOutDataSlots(int32 OutDataNum);

	UFUNCTION(BlueprintCallable)
	void SetOutDataSlot(int32 Index, const FGameplayTag& SlotTag);

	UFUNCTION(BlueprintPure)
	const FNeAbilityDataBoardKey& GetOutDataEntry(int32 Index = 0)
	{
		return OutDataEntries.Num() > 0 ? OutDataEntries[Index] : FNeAbilityDataBoardKey::Invalid;
	}

protected:
	virtual void OnActive(FNeAbilitySegmentEvalContext& EvalContext) {}
	virtual void OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext) {}
	virtual void OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason) {}

#if WITH_EDITOR
public:
	/** 技能节点显示文字 */
	virtual FText GetDisplayText() const;

	virtual void ValidateAsset() {}
#endif

	//~=============================================================================
	// 配置项

public:
	UPROPERTY()
	uint8 bHasPropertyBinding : 1;

	UPROPERTY(EditAnywhere, Category = "Beam")
	EAbilityDurationPolicy DurationType = EAbilityDurationPolicy::HasDuration;

	UPROPERTY(EditAnywhere, Category = "Beam")
	EBeamEffectiveType EffectiveType = EBeamEffectiveType::WithinTimeline;

	UPROPERTY(EditAnywhere, Category = "Beam")
	float Duration = 0.33f;

	/** Beam目标策略 */
	UPROPERTY(EditAnywhere, Category = "Beam | Target", Meta = (Bitmask, BitmaskEnum = "/Script/NeAbilitySystem.EBeamTargetPolicy"))
	int32 TargetPolicy = static_cast<int32>(EBeamTargetPolicy::LockTargets);

	/**
	 * 如果目标类型包含"自定义目标"
	 * 则根据技能数据板里的信息填充数据
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Beam | Target", meta=(Categories ="Ability.Data"))
	FGameplayTagContainer CustomTargetKeys;

	/** Beam同步策略 */
	UPROPERTY(EditAnywhere, Category = "Beam", AdvancedDisplay)
	EBeamReplicationPolicy ReplicationPolicy = EBeamReplicationPolicy::ClientOnly;

	/**
	 * 是否可以关联子Task
	 */
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "Beam")
	uint8 bCompound : 1;

	/** 指定Beam输出数据到数据板 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Beam", EditFixedSize)
	TArray<FNeAbilityDataBoardKey> OutDataEntries;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "Gizmo")
	uint8 bNeedGizmo : 1;

	/** Class for create gizmo */
	UPROPERTY(config, EditDefaultsOnly, AdvancedDisplay, Category = "Gizmo")
	TSubclassOf<class ANeAbilityGizmoActor>		GizmoType;
#endif

	//~=============================================================================
	// 运行时变量
public:
	UPROPERTY(Transient)
	TObjectPtr<UNeAbility> OwnerAbility = nullptr;

protected:
	/** 运行状态 */
	UPROPERTY(Transient)
	EAbilityBeamState State = EAbilityBeamState::Idle;

	/** 请求结束 */
	UPROPERTY(Transient)
	uint8 bRequestFinished : 1;

	UPROPERTY(Transient)
	float RunningTime = 0;
};