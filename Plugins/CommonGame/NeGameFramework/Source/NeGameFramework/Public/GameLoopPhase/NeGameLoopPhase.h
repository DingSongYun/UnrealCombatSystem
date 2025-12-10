// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NeGameLoopPhase.generated.h"

/**
 * UNeGameLoopPhase
 * 游戏循环阶段
 */
UCLASS(Abstract, Blueprintable)
class NEGAMEFRAMEWORK_API UNeGameLoopPhase : public UObject
{
	GENERATED_BODY()

public:
	void OnBeginPhase();
	void TickPhase(float Delta);
	void OnEndPhase();


	UFUNCTION(BlueprintPure)
	FORCEINLINE FGameplayTag GetGameLoopPhase() const { return PhaseTag; }

	FORCEINLINE bool IsTickEnable() const { return bEnableTick; }

	//~BEGIN: Object Interface
	virtual class UWorld* GetWorld() const override;
	//~END: Object Interface

protected:
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "Begin Phase"))
	void ReceiveOnBeginPhase();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "Tick"))
	void ReceiveTickPhase(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "End Phase"))
	void ReceiveOnEndPhase();

protected:
	UPROPERTY(EditDefaultsOnly, Category="GameLoopPhase")
	FGameplayTag PhaseTag;

	UPROPERTY(EditDefaultsOnly, Category="GameLoopPhase")
	uint8 bEnableTick : 1;
};
