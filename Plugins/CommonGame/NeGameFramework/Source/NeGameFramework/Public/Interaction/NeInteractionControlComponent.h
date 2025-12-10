// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeInteractionScanner.h"
#include "Components/ControllerComponent.h"
#include "Engine/EngineTypes.h"
#include "NeInteractionControlComponent.generated.h"

class UNeInteractionScanner;
struct FNeInteractionOption;
class UIndicatorDescriptor;

/**
 * UNeInteractionControlComponent
 * 交互组件
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEGAMEFRAMEWORK_API UNeInteractionControlComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UNeInteractionControlComponent(const FObjectInitializer& Initializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 获取Option指示UI */
	UFUNCTION(Blueprintable)
	TSoftClassPtr<UUserWidget> GetOptionIndicatorWidget(const FNeInteractionOption& Option) const;

protected:
	void ScanInteractions(const FNeInteractionScanConfig& ScanConfig);
	void HandleInteractionOptions();

	UFUNCTION(Blueprintable)
	FORCEINLINE void TriggerInteractionByIndex(int32 OptionIndex)
	{
		if (InteractionOptions.IsValidIndex(OptionIndex))
		{
			TriggerInteraction(InteractionOptions[OptionIndex]);
		}
	}

	UFUNCTION(Blueprintable)
	void TriggerInteraction(const FNeInteractionOption& Option);

	UFUNCTION(BlueprintImplementableEvent)
	void ReceivePostHandleInteractionOptions() const ;

protected:
	/** 交互查询间隔 */
	UPROPERTY(EditDefaultsOnly)
	TArray<FNeInteractionScanConfig> ScanConfigs;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FTimerHandle> ScanTimerHandles;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FNeInteractionOption> InteractionOptions;
};
