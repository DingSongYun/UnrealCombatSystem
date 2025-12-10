// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonPreLoadScreen.h"
#include "SCommonPreLoadingScreenWidget.h"
#include "Misc/App.h"
#include "CoreGlobals.h"
#include "CommonStartupLoadingScreenSettings.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "CommonPreLoadingScreen"

void FCommonPreLoadScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		UTexture2D* Image = Cast<UTexture2D>(GetDefault<UCommonStartupLoadingScreenSettings>()->EngineLoadingScreenImage.TryLoad());
		LoadingWidget = SNew(SCommonPreLoadingScreenWidget).Image(Image);
	}
}

void FCommonSplashScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		UTexture2D* Image = Cast<UTexture2D>(GetDefault<UCommonStartupLoadingScreenSettings>()->CustomSplashScreenImage.TryLoad());
		LoadingWidget = SNew(SCommonPreLoadingScreenWidget).Image(Image);
	}
}

void FCommonEarlyStartupScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		UTexture2D* Image = Cast<UTexture2D>(GetDefault<UCommonStartupLoadingScreenSettings>()->EarlyStartupScreenImage.TryLoad());
		LoadingWidget = SNew(SCommonPreLoadingScreenWidget).Image(Image);
	}
}

#undef LOCTEXT_NAMESPACE
