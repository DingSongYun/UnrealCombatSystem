// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Beams/NeAbilityBeam_Trigger.h"
#include "NeAbilityBeam_WaitInput.generated.h"

UENUM(BlueprintType)
enum class EWaitInputType : uint8
{
	Once				= 0							UMETA(DisplayName = "短按"),
	Holding											UMETA(DisplayName = "长按超过一定时间触发"),
	HoldingReleased									UMETA(DisplayName = "按压一定时长&抬起"),
};

/**
 * UNeAbilityBeam_WaitInput
 * 等待按键
 */
UCLASS(Category="Input", DisplayName="等待按键")
class NEABILITYSYSTEM_API UNeAbilityBeam_WaitInput : public UNeAbilityBeam_Trigger
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input", meta=(Tooltip="等待输入标签，如果InputAction存在，优先使用InputAction"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input", meta=(Tooltip="等待的InputAction"))
	class UInputAction* InputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	EWaitInputType InputType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Condition, meta = (DisplayName = "按键时长"))
	float HoldTime = 0;
};
