// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "INeInteractableInterface.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "NeInteractionScanner.generated.h"

class UWorld;
class UNeInteractionControlComponent;
class AActor;
class UNeInteractionScanner;

/**
 * FNeInteractionScanConfig
 */
USTRUCT(Blueprintable)
struct FNeInteractionScanConfig
{
	GENERATED_BODY()

	/** 交互查询半径 */
	UPROPERTY(EditDefaultsOnly)
	float Radius = 600;

	/** 交互查询间隔 */
	UPROPERTY(EditDefaultsOnly)
	float Interval = 0.1f;

	/** 查询器 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNeInteractionScanner> ScannerType;
};

/**
 * UNeInteractionScanner
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class NEGAMEFRAMEWORK_API UNeInteractionScanner : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void ScanInteractions(UWorld* World, UNeInteractionControlComponent* Querier, AActor* Instigator, const FNeInteractionScanConfig& ScanConfig, TArray<TScriptInterface<INeInteractableInterface>>& OutInteractions) {}

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveScanInteractions(UWorld* World, UNeInteractionControlComponent* Querier, AActor* Instigator, const FNeInteractionScanConfig& ScanConfig, TArray<UObject*>& OutInteractions);
};

UCLASS(Abstract, Blueprintable)
class NEGAMEFRAMEWORK_API UNeInteractionDefaultScanner : public UNeInteractionScanner
{
	GENERATED_BODY()

public:
	virtual void ScanInteractions(UWorld* World, UNeInteractionControlComponent* Querier, AActor* Instigator, const FNeInteractionScanConfig& ScanConfig, TArray<TScriptInterface<INeInteractableInterface>>& OutInteractions) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> TraceChannel;
};