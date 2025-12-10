// Copyright NetEase Games, Inc. All Rights Reserved.
// 游戏启动阶段得LoadScreen

#pragma once

#include "PreLoadScreenBase.h"

//=============================================================================
/**
 * FCommonPreLoadScreen
 */
class FCommonPreLoadScreen : public FPreLoadScreenBase
{
public:
	virtual void Init() override;
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }
	virtual TSharedPtr<SWidget> GetWidget() override { return LoadingWidget; }

private:
	TSharedPtr<SWidget> LoadingWidget;
};

//=============================================================================
/**
 * FCommonSplashScreen
 */
class FCommonSplashScreen : public FPreLoadScreenBase
{
public:
	
	virtual void Init() override;
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::CustomSplashScreen; }
	virtual TSharedPtr<SWidget> GetWidget() override { return LoadingWidget; }

private:
	TSharedPtr<SWidget> LoadingWidget;
};

//=============================================================================
/**
 * FCommonEarlyStartupScreen
 */
class FCommonEarlyStartupScreen : public FPreLoadScreenBase
{
public:
	
	virtual void Init() override;
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EarlyStartupScreen; }
	virtual TSharedPtr<SWidget> GetWidget() override { return LoadingWidget; }

private:
	TSharedPtr<SWidget> LoadingWidget;
};