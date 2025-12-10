// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NeGameLoopPhaseSet.generated.h"

/**
 * UNeGameLoopPhaseSet
 *
 * 游戏内GameLoopPhase注册表
 */
UCLASS(Blueprintable)
class NEGAMEFRAMEWORK_API UNeGameLoopPhaseSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class UNeGameLoopPhase>> GameLoopPhases;
};
