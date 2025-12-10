// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "NeInputRegistration.generated.h"

class UInputAction;

/**
 * FNeInputData
 *
 * InputAction <-> FGameplayTag
 */
USTRUCT(BlueprintType)
struct NEGAMEFRAMEWORK_API FNeTaggedInputAction
{
	GENERATED_BODY()
public:
	FORCEINLINE bool IsValid() const { return InputTag.IsValid() && InputAction; }
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	/** For Debug */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bMute : 1;

	static FNeTaggedInputAction INVALID;
};

/**
 * UNeInputRegistration
 * InputAction注册文件，全局只需要一个
 */
UCLASS(BlueprintType)
class NEGAMEFRAMEWORK_API UNeInputRegistration : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FNeTaggedInputAction> InputData;
};
