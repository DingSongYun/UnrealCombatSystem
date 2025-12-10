// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeam.h"
#include "NeAbilityBeam_Trigger.generated.h"

//=============================================================================
/**
 * UNeAbilityBeam_Trigger
 * 技能节点的分支功能
 */
UCLASS()
class NEABILITYSYSTEM_API UNeAbilityBeam_Trigger : public UNeAbilityBeam
{
	GENERATED_UCLASS_BODY()
public:
	/** 执行触发 */
	virtual void ExecuteTrigger(FNeAbilitySegmentEvalContext& EvalContext);

	/** 对于Repeat的Trigger，触发之后重置状态 */
	virtual void ResetState();

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trigger", meta=(EditCondition="Mode==ETriggerMode::Immediately", DisplayName="可重复触发", Tooltip="是否可重复触发"))
	uint8 bRepeat : 1;

#if WITH_EDITORONLY_DATA
	/** 编辑器中模拟运行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient,  Category = "Trigger|Simulator", meta = (DisplayName = "模拟运行", Tooltip = "模拟运行"))
	bool bOpenForSimulate = false;

	/** 模拟时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "Trigger|Simulator", meta = (EditCondition = "bOpenForSimulate==true", DisplayName = "模拟开始时间", Tooltip = "模拟开始时间"))
	float SimulatorStartTime = 0.0f;
#endif //
};
