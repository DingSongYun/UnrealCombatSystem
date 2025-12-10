// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "NeInputAction.generated.h"

/**
 *
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeInputAction : public UInputAction
{
	GENERATED_BODY()

public:
	UNeInputAction(const FObjectInitializer& Initializer = FObjectInitializer::Get());

	//~BEGIN: UObject interface
	virtual void PostLoad() override;
	//~END: UObject interface

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UPROPERTY(EditDefaultsOnly, Category="Input Tag", meta = (Categories = "InputAction"))
	FGameplayTag ActionTag;
};
