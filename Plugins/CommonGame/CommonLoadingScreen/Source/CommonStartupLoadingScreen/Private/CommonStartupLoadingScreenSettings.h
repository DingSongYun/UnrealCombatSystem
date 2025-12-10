// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "CommonStartupLoadingScreenSettings.generated.h"

/**
 * Settings for a loading screen system.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Common Startup Loading Screen"))
class UCommonStartupLoadingScreenSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UCommonStartupLoadingScreenSettings(const FObjectInitializer& Initializer = FObjectInitializer::Get()) : Super(Initializer) {}

public:
	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/Engine.Texture2D"))
	FSoftObjectPath CustomSplashScreenImage;

	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/Engine.Texture2D"))
	FSoftObjectPath EarlyStartupScreenImage;
	
	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/Engine.Texture2D"))
	FSoftObjectPath EngineLoadingScreenImage;
};