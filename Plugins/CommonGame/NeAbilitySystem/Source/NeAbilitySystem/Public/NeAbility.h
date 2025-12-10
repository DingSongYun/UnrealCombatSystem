// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NeAbilitySection.h"
#include "NeAbilitySegmentEvalQueue.h"
#include "NeAbilityTargetData.h"
#include "NeAbilityTimeController.h"
#include "Misc/NeAbilityPreviewActor.h"
#include "NeAbilityPropertyBinding.h"
#include "DataBoard/NeAbilityDataBoard.h"
#include "NeAbility.generated.h"

class UNeAbilitySegment;
class UNeAbilityBeam;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSectionEndDelegate, int32 /* SectionIndex */);

/**
 * UNeAbility
 *
 * 技能对象
 * 我们期望构建一个内建Timeline的分段式技能对象
 */
UCLASS()
class NEABILITYSYSTEM_API UNeAbility : public UGameplayAbility
{
	GENERATED_UCLASS_BODY()

	friend class FNeAbilityBlueprintEditor;
	friend class FNeAbilityPropertyBindingEditor;
	friend class FNeAbilityTimelineMode;
	friend class SNeAbilityEditorTab_Timeline;
	friend struct FNeAbilityTimeController;

public:
	//~BEGIN: UGameplayAbility interface
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~END: UGameplayAbility interface

	//~BEGIN: UObject interface
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~END: UObject interface

	/**
	 * 估算播放时长，这个不准确，会受一些条件片段的影响
	 *
	 * @return 返回预估的播放时长，-1表示无限时长, 0表示瞬时
	 */
	float EstimatePlayLength() const;
	virtual bool ShouldTick() const;
	virtual void Tick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	class ACharacter* GetOwningCharacter() const;

	/** Whether the ability is timeline based. */
	FORCEINLINE bool IsTimelineBasedAbility() const { return bTimelineBasedAbility; }

	/** Get total section num */
	FORCEINLINE int32 GetSectionNums() const { return Sections.Num(); }

	/** Get Ability section at index */
	virtual FNeAbilitySection& GetSection(int32 Index)
	{
		check(Sections.IsValidIndex(Index));
		return Sections[Index];
	}

	/** Section Index是否有效 */
	virtual bool IsValidSection(int32 Index) const { return Index != INDEX_NONE && Sections.IsValidIndex(Index); }

	/** Get Ability section at index */
	virtual const FNeAbilitySection& GetSection(int32 Index) const
	{
		check(Sections.IsValidIndex(Index));
		return Sections[Index];
	}

	/** 第一个执行的Section */
	virtual int32 GetStartupSectionIndex() const;

	/** 当前播放的位置 */
	float GetPlayingPosition() const;

	/** 当前播放的Section和位置 */
	int32 GetPlayingSection() const;
	void GetPlayingSection(int32& SectionIndex, float& PlayingPosition) const;

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnGiveAbility", meta=(ScriptName = "OnGiveAbility"))
	void ReceiveOnGiveAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "PreActivateAbility", meta=(ScriptName = "PreActivateAbility"))
	void ReceivePreActivate(const FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "PreActivateAbilityFromEvent", meta=(ScriptName = "PreActivateAbilityFromEvent"))
	void ReceivePreActivateFromEvent(const FGameplayAbilitySpecHandle Handle, const FGameplayEventData& EventData);

	//~=============================================================================
	// Ability Context
public:
	AActor* GetInstigator() const { return Instigator.Get(); }

	/** 执行子Segment */
	UFUNCTION()
	void TriggerSegmentChildren(const FNeAbilitySegmentEvalContext& SegmentEvalContext);

	/**
	 * 计算属性绑定值
	 *
	 * 这里期望是一种push mode，Beam按需去更新属性绑定，而非技能层主动每帧推送
	 */
	virtual void EvaluatePropertyBindings(FNeAbilitySegmentEvalContext& SegmentEvalContext);

protected:
	/** 更新时间轴 */
	void Advance(float DeltaTime);

	/** 更新Section时间轴 */
	void UpdateSectionTimeCursor(float CurrentPosition, float PreviousPosition, float DeltaTime, int32 SectionIndex);

	/** Section播放到结束 */
	void OnReachSectionEnd(int32 SectionIndex);

	/** 当帧处理Segments */
	void EvaluateSegments(float DeltaTime);

	/** Segment加入等待执行队列 */
	virtual void SegmentReadyToActive(int32 SectionIndex, int32 SegmentIndex);

	/** 激活Segment */
	virtual void ActivateSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext);

	/** 更新Segment执行 */
	virtual void AdvanceSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, float DeltaTime);

	/** 结束Segment */
	virtual void EndSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, EAbilityBeamEndReason EndReason);

	/** 创建Segment的运行时实例 */
	virtual UNeAbilityBeam* CreateBeamEvalInstance(FNeAbilitySegmentEvalContext& SegmentEvalContext, UNeAbilityBeam* ArcheType);

	/** 执行指定的Segment */
	void EvaluateSingleSegment(int32 Section, int32 SegmentID);

	//~=============================================================================
	// Ability Data
public:
	/** 获取技能绑定的数据板 */
	UFUNCTION(BlueprintPure)
	const FNeAbilityDataBoard& GetDataBoard() const { return DataBoard; }
	FNeAbilityDataBoard& GetMutableDataBoard() { return DataBoard; }
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataInteger(const FNeAbilityDataBoardKey& Key, int64& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataFloat(const FNeAbilityDataBoardKey& Key, float& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataDouble(const FNeAbilityDataBoardKey& Key, double& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataBool(const FNeAbilityDataBoardKey& Key, bool& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataVector(const FNeAbilityDataBoardKey& Key, FVector& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataLinearColor(const FNeAbilityDataBoardKey& Key, FLinearColor& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataRotator(const FNeAbilityDataBoardKey& Key, FRotator& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataObject(const FNeAbilityDataBoardKey& Key, UObject*& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataHitResult(const FNeAbilityDataBoardKey& Key, FHitResult& OutValue) const;
	UFUNCTION(BlueprintPure)
	EAbilityDataAccessResult GetDataStruct(const FNeAbilityDataBoardKey& Key, FInstancedStruct& OutValue) const;
	UFUNCTION(BlueprintCallable)
	void SetDataInteger(const FNeAbilityDataBoardKey& Key, int64 InValue);
	UFUNCTION(BlueprintCallable)
	void SetDataFloat(const FNeAbilityDataBoardKey& Key, float InValue);
	UFUNCTION(BlueprintCallable)
	void SetDataDouble(const FNeAbilityDataBoardKey& Key, double InValue);
	UFUNCTION(BlueprintCallable)
	void SetDataBool(const FNeAbilityDataBoardKey& Key, bool InValue);
	UFUNCTION(BlueprintCallable)
	void SetDataVector(const FNeAbilityDataBoardKey& Key, const FVector& InValue);
	UFUNCTION(BlueprintCallable)
	void SetDataRotator(const FNeAbilityDataBoardKey& Key, const FRotator& OutValue);
	UFUNCTION(BlueprintCallable)
	void SetDataObject(const FNeAbilityDataBoardKey& Key, UObject* InValue);
	UFUNCTION(BlueprintCallable)
	void SetDataHitResult(const FNeAbilityDataBoardKey& Key, const FHitResult& InValue);
	UFUNCTION(BlueprintCallable)
	void SetDataStruct(const FNeAbilityDataBoardKey& Key, const FInstancedStruct& InValue);

protected:
	/**
	 * 在技能域内搜索Segment的执行目标
	 *
	 * @param TargetPolicyFlags			目标种类
	 * @param OutTargets				结果目标集
	 * @param CustomTargetKeys			
	 */
	void AssembleTargets(int32 TargetPolicyFlags, TArray<FNeAbilityTargetingInfo>& OutTargets, const FGameplayTagContainer& CustomTargetKeys) const;

	/** 获取缓存的碰撞检测结果 */
	UFUNCTION(BlueprintImplementableEvent)
	const TArray<FHitResult> GetCachedCollisionQueryResults() const;

#if WITH_EDITOR
public:
	void PostAssetCreate();
	void AddMainSection();
	void LinkSection(int32 SectionIndex, int32 NextSectionIndex);
	void LinkSection(int32 SectionIndex, FName NextSectionName);

	void JumpToSectionForPIE(int32 SectionIndex);

	/** 新加技能Segment */
	FNeAbilitySegment& AddNewSegment(const FNeAbilitySegmentDef& SegmentDef, int32 SectionIndex = 0);

	/** 删除节能功能Segment */
	void RemoveSegment(FNeAbilitySegment& Seg);
	void RemoveSegment(int32 SectionIndex, const FNeAbilitySegment& Seg);

	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;

	/** 尝试修复资源 */
	void TryRepairAsset();

	/**
	 * 属性绑定变化时通知
	 * 预览时尤其需求这个通知，用来更新属性绑定计算context
	 */
	virtual void NotifyPropertyBindingChanged(const FNeAbilityPropertyBinding& BindingChanged);
#endif

	//~=============================================================================
	// 配置项
public:
#if WITH_EDITORONLY_DATA
	/** 技能名称*/
	UPROPERTY(EditDefaultsOnly, Category=Ability)
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Ability)
	FText Tips;
#endif

	UPROPERTY(EditDefaultsOnly, Category = "Input", Meta = (Bitmask, BitmaskEnum = "/Script/NeAbilitySystem.EAbilityInputReplicatePolicy"))
	int32 InputReplicatePolicy = 0;

protected:
	/** 技能是否是基于Timeline的， 默认True */
	UPROPERTY(EditDefaultsOnly, Category=Ability)
	uint8 bTimelineBasedAbility : 1;

	/** 时间轴结束自动结束技能 */
	UPROPERTY(EditDefaultsOnly, Category=Ability)
	uint8 bAutoEndWithTimeline : 1;

	/** 技能分段 */
	UPROPERTY(VisibleDefaultsOnly, AdvancedDisplay, Category=Ability)
	TArray<FNeAbilitySection> Sections;

	UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category=Ability)
	FNeAbilityDataBoard DataBoard;

	/** 属性绑定 */
	UPROPERTY(EditAnywhere)
	TArray<FNeAbilityPropertyBinding> PropertyBindings;

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(EditAnywhere, DuplicateTransient, Category=Preview)
	FNeAbilityPreviewActors AbilityPreviewActors;

protected:
	UPROPERTY(VisibleDefaultsOnly, AdvancedDisplay, Category=Ability)
	uint32 SegmentIDGenerator = 0;
#endif

	//~=============================================================================
	// 运行时变量

protected:
	/** 改技能的始作俑者， 比如 玩家命令召唤物施放某个技能，玩家则是这个技能的始作俑者，召唤我是技能的施放者 */
	UPROPERTY(transient, DuplicateTransient)
	TObjectPtr<AActor> Instigator = nullptr;

	/** 技能目标对象 */
	UPROPERTY(transient, DuplicateTransient)
	TArray<FNeAbilityTargetingInfo> Targets;

	/** 播放时间轴控制 */
	UPROPERTY(transient, DuplicateTransient)
	FNeAbilityTimeController TimeController;

	/** 执行过的Section */
	UPROPERTY(transient, DuplicateTransient)
	TArray<int32> PassedSection;

	/** Segment的执行队列 */
	UPROPERTY(transient, DuplicateTransient, Replicated)
	FAbilitySegmentQueue SegmentQueue;
};
