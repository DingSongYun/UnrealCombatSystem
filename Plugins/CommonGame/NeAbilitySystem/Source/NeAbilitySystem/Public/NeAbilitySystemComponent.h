// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "NeAbilityTargetData.h"
#include "NeAbilitySegmentEvalQueue.h"
#include "NeAbilityTypes.h"
#include "NeAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAnyGameplayTagCountChangedDDG, const FGameplayTag&, Tag, int32, NewCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNeAbilityEndedDelegate, const FAbilityEndedData&, AbilityEndedData);

struct FNeScriptDelegateBindingStub
{
public:
	FNeScriptDelegateBindingStub(UObject* InBindingObject, const FDelegateHandle& InBindingHandler)
		: BindingObject(InBindingObject), BindingHandler(InBindingHandler)
	{}

	FORCEINLINE const FDelegateHandle& GetHandler() const { return BindingHandler; }

public:
	TWeakObjectPtr<UObject> BindingObject;
	FDelegateHandle BindingHandler;
};


/** 技能施放参数 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityActivateParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SlotName;

	/** 始作俑者 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Instigator = nullptr;

	/** 技能目标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNeAbilityTargetingInfo> Targets;

	/** Check Method */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EActivateAbilityCheckMethod ActivateCheckMethod = EActivateAbilityCheckMethod::AlwaysInterrupt;

	/** 技能的其他信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Transient)
	FNeAbilityDataBoard Data;

	FNeAbilityActivateParameters& operator=(const FNeAbilityActivateParameters& Other)
	{
		SlotName = Other.SlotName;
		Instigator = Other.Instigator;
		ActivateCheckMethod = Other.ActivateCheckMethod;

		for (int32 i = 0; i < Other.Targets.Num(); i++)
		{
			Targets.Add(Other.Targets[i]);
		}

		Data = Other.Data;

		return *this;
	}
};

/**
 * UNeAbilitySystemComponent
 *
 * Base ability component of this project
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The main ability system componen."))
class NEABILITYSYSTEM_API UNeAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	//~BEGIN: UAbilitySystemComponent interface
	virtual bool GetShouldTick() const override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	//~END: UAbilitySystemComponent interface

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool TryActivateAbilityWithData(FGameplayAbilitySpecHandle AbilityToActivate, const FGameplayEventData& GameplayEventData);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool EndAbility(const FGameplayAbilitySpecHandle& SpecHandle, bool bWasCancelled);

	/** 技能是否正在播放 */
	bool IsAbilityPlaying(const FGameplayAbilitySpec&) const;

	/** 技能是否正在播放 */
	UFUNCTION(BlueprintPure)
	bool IsAbilityPlaying(const FGameplayAbilitySpecHandle& InSpecHandle) const;

	UFUNCTION(BlueprintCallable)
	int32 SendGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& Payload, bool bTriggerAbility = false);

	/** 获取Spec中正在运行的技能实例，可能是CDO */
	TArray<UGameplayAbility*> GetPlayingInstance(const FGameplayAbilitySpec& InSpec) const;

	TArray<UGameplayAbility*> GetPlayingInstance(const FGameplayAbilitySpecHandle& InSpecHandle) const;

	UFUNCTION(BlueprintPure)
	UGameplayAbility* GetPlayingAbilityInstance(const FGameplayAbilitySpecHandle& InSpecHandle) const;

	/** 将执行队列添加到SegmentQueue*/
	void PushSegmentQueue(const FAbilitySegmentQueue& Queue);

	/** 执行挂在到Component上的Segment */
	void EvaluateSegments(float DeltaTime);

	/** 创建技能施放的参数 */
	FGameplayEventData MakeActivateEventData(AActor* Instigator, AActor* Target, EActivateAbilityCheckMethod InCheckMethod = EActivateAbilityCheckMethod::AlwaysActivate) const;

	/** 创建技能施放的参数 */
	FGameplayEventData MakeActivateEventData(AActor* Instigator, TArray<AActor*> Targets, EActivateAbilityCheckMethod InCheckMethod = EActivateAbilityCheckMethod::AlwaysActivate) const;

	/** 创建技能施放的参数 */
	FGameplayEventData MakeActivateEventData(const FNeAbilityActivateParameters& ActivateParameters) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Gameplay Abilities", meta = (DisplayName = "Give Ability", ScriptName = "GiveAbilityWithSpec"))
	FORCEINLINE FGameplayAbilitySpecHandle K2_GiveAbilityWithSpec(const FGameplayAbilitySpec& AbilitySpec)
	{
		return GiveAbility(AbilitySpec);
	}

	/**
	 * 外部通知技能对于Input动作
	 *
	 * @param AbilityTag: 暂时支持两种，技能Slot & Event
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Gameplay Abilities")
	void AbilityInputPressed(const FGameplayTag& AbilityTag);

	/**
	 * 外部通知技能对于Input动作
	 *
	 * @param AbilityTag: 暂时支持两种，技能Slot & Event
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Gameplay Abilities")
	void AbilityInputReleased(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Gameplay Abilities", meta = (DisplayName = "Give Ability", ScriptName = "InvokeReplicatedEvent"))
	bool K2_InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::Type EventType, FGameplayAbilitySpecHandle AbilityHandle);

	/** Calls local callbacks that are registered with the given Generic Replicated Event */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Gameplay Abilities", meta = (DisplayName = "Give Ability", ScriptName = "InvokeReplicatedEventWithPayload"))
	bool K2_InvokeReplicatedEventWithPayload(EAbilityGenericReplicatedEvent::Type EventType, FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize100 VectorPayload);

	/**
	 * 根据技能类型查找SpecHandle
	 *
	 * @param InAbilityClass		技能类
	 * @param bGiveIfNotFound		如果没有找到，是否调用Give进行注册
	 * @return 返回AbilitySpecHandle
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Gameplay Abilities", meta = (DisplayName = "Find Ability Spec", ScriptName = "FindAbilitySpecHandleFromClass"))
	FGameplayAbilitySpecHandle FindAbilitySpecHandleFromClass(TSubclassOf<UGameplayAbility> InAbilityClass, bool bGiveIfNotFound = false);

	/**
	 * 给注册的Ability添加动态标签
	 */
	UFUNCTION(BlueprintCallable)
	void AddDynamicAbilityTag(const FGameplayAbilitySpecHandle& AbilitySpecHandle, const FGameplayTag& Tag) const;

	/**
	 * 删除注册的Ability的动态标签
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveDynamicAbilityTag(const FGameplayAbilitySpecHandle& AbilitySpecHandle, const FGameplayTag& Tag) const;

	//~==============================================================================================
	// GameplayTag

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags", meta = (DisplayName = "HasAllMatchingGameplayTags", ScriptName = "HasAllMatchingGameplayTags"))
	FORCEINLINE bool K2_HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
	{
		return HasAllMatchingGameplayTags(TagContainer);
	}

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags", meta = (DisplayName = "HasAnyMatchingGameplayTags", ScriptName = "HasAnyMatchingGameplayTags"))
	FORCEINLINE bool K2_HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
	{
		return HasAnyMatchingGameplayTags(TagContainer);
	}

	/** Forcibly sets the number of instances of a given tag */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	FORCEINLINE bool SetGameplayTagCount(const FGameplayTag& Tag, int32 NewCount)
	{
		return GameplayTagCountContainer.SetTagCount(Tag, NewCount);
	}

	/** Update the number of instances of a given tag and calls callback */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	bool UpdateGameplayTagCount(const FGameplayTag& Tag, const int32 CountDelta)
	{
		const bool Ret = GameplayTagCountContainer.UpdateTagCount(Tag, CountDelta);
		if (Ret)
		{
			OnTagUpdated(Tag, CountDelta > 0);
		}

		return Ret;
	}

	/** Update the number of instances of a given tag and calls callback */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	void UpdateGameplayTagContainer(const FGameplayTagContainer& Container, int32 CountDelta)
	{
		UpdateTagMap(Container, CountDelta);
	}

	/** Register delegate that is invoked whenever a tag is added or removed (but not if just count is increased. Only for 'new' and 'removed' events) */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	void RegisterAnyTagChangedEvent(UObject* InBindingObject, const FOnAnyGameplayTagCountChangedDDG& Delegate);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	void UnregisterAnyTagChangedEvent(UObject* InBindingObject);

protected:
	UPROPERTY()
	FAbilitySegmentQueue SegmentQueue;

	/** 技能结束事件，暴露给蓝图/脚本使用 */
	UPROPERTY(BlueprintAssignable)
	FNeAbilityEndedDelegate AbilityEndedEvent;

	TArray<FNeScriptDelegateBindingStub> DelegateBindingStubs;
};
