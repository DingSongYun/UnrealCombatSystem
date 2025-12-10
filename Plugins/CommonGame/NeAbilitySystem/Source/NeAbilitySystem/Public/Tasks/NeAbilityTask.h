// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "NeAbilityTask.generated.h"

/**
 *
 */
UCLASS(Abstract, HideDropdown)
class NEABILITYSYSTEM_API UNeAbilityTask : public UAbilityTask
{
	GENERATED_BODY()

public:
	//~BEGIN: UAbilityTask interface
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	//~END: UAbilityTask interface

	/**
	 * 主要提供给预览使用
	 * 指定位置采样
	 */
	virtual void SamplePosition(const float Position, const float PreviousPosition);

	/** Get Character who owns the ability and task */
	UFUNCTION(BlueprintPure)
	class ACharacter* GetCharacter() const;

	UFUNCTION(BlueprintPure)
	float GetRunningTime() const;

	UFUNCTION(BlueprintPure)
	float GetRequiredDuration() const;

protected:
	/** Called when task is end. */
	virtual void OnEndTask();

#if WITH_EDITOR
public:
	/**
	 * 如果是动画相关的Task
	 *  骨骼动画、特效、等都可以算是动画相关性
	 */
	virtual void NotifyAnimRelevanceChanged() const;
	virtual float EvalAnimRelevanceDuration() const { return 0.f; }

	/** 获取显示文本 */
	virtual FText GetDisplayText() const;
#endif

public:
	static UNeAbilityTask* NewAbilityTaskFromTemplate(UGameplayAbility* ThisAbility, UClass* TaskClass, UAbilityTask* Template, FName InstanceName = FName())
	{
		check(ThisAbility);

		UNeAbilityTask* NewTask = NewObject<UNeAbilityTask>(ThisAbility, TaskClass, NAME_None, RF_NoFlags, Template);
		NewTask->InitTask(*ThisAbility, ThisAbility->GetGameplayTaskDefaultPriority());
		NewTask->InstanceName = InstanceName;
		return NewTask;
	}

	/** Helper function for instantiating and initializing a new task */
	template <class T>
	static T* NewAbilityTaskFromTemplate(UGameplayAbility* ThisAbility, T* Template, FName InstanceName = FName())
	{
		return Cast<T>(NewAbilityTaskFromTemplate(ThisAbility, T::StaticClass(), Template, InstanceName));
	}

public:
	UPROPERTY()
	TObjectPtr<class UNeAbilityBeam> LinkedBeam;
};
