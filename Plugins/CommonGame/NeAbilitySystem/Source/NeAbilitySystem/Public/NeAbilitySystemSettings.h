// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/NeAbilityPreviewActor.h"
#include "UObject/Object.h"
#include "NeAbilitySystemSettings.generated.h"

/**
 *
 */
UCLASS(config = Ability, defaultconfig)
class NEABILITYSYSTEM_API UNeAbilitySystemSettings : public UObject
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(Config, EditAnywhere, Category = "Preview", meta=(Tooltip="Player type for ability preview"))
	TSoftClassPtr<class APlayerController> PlayerControllerClass;

	UPROPERTY(Config, EditAnywhere, Category = "Preview", meta=(Tooltip="Player type for ability preview"))
	TSoftClassPtr<class APlayerCameraManager> CameraManagerClass;

	UPROPERTY(Config, EditAnywhere, Category = "Preview", meta=(Tooltip="Player type for ability preview"))
	TSoftClassPtr<UNeAbilityPreviewActorType> PlayerType = UNeAbilityPreviewActorCommon::StaticClass();

	UPROPERTY(Config, EditAnywhere, Category = "Preview", meta=(Tooltip="Target type for ability preview"))
	TSoftClassPtr<UNeAbilityPreviewActorType> TargetType = UNeAbilityPreviewActorCommon::StaticClass();
#endif
};
