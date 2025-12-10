// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeamLinkage.h"
#include "StructUtils/PropertyBag.h"
#include "NeAbilityBeam_GameplayTask.generated.h"

class UGameplayAbility;
class UAbilityTask;

UCLASS(HideDropdown)
class NEABILITYSYSTEM_API UNeAbilityBeam_GameplayTask : public UNeAbilityBeamLinkage
{
	GENERATED_UCLASS_BODY()

public:
	//~BEGIN: UNeAbilityBeamLinkage interface
	virtual UClass* GetSupportClass() const override;
	//~END: UNeAbilityBeamLinkage interface

	//~BEGIN: UNeAbilityBeam interface
	virtual void OnActive(FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason) override;
	virtual void SamplePosition(const float Position, const float PreviousPosition) override;
	//~BEGIN: UNeAbilityBeam interface

protected:
	/** 创建Task Instance */
	void CreateTaskInstance(UGameplayAbility* Ability);

	/** 从模板创建Task Instance */
	void CreateTaskInstanceFromTemplate(UGameplayAbility* Ability);

	/** 从属性集创建Task Instance */
	void CreateTaskInstanceFromPropertySet(UGameplayAbility* Ability);

#if WITH_EDITOR
	/** 技能节点显示文字 */
	virtual FText GetDisplayText() const override;
#endif

public:
	UPROPERTY(EditAnywhere, Category="Beam|GameplayTask")
	FName InstanceName = NAME_None;

	/**
	 * 是否使用紧凑型的参数组织方式
	 * true: 将参数提取放到一个通用的泛型参数集里管理
	 * false: 使用Instanced的对象当作参数配置和创建实例化的模板
	 * */
	UPROPERTY(EditDefaultsOnly)
	uint8 bCompactParams : 1;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bCompactParams", EditConditionHides))
	FInstancedPropertyBag TaskParameters;

	UPROPERTY(EditAnywhere, Instanced, meta=(EditCondition="!bCompactParams", EditConditionHides))
	UAbilityTask* TaskTemplate = nullptr;

public:
	UPROPERTY()
	TArray<FNeAbilityTargetingInfo> TargetingInfos;

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask> TaskInstance = nullptr;
};
