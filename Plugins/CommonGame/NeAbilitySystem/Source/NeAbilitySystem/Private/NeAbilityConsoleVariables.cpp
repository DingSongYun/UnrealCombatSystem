// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityConsoleVariables.h"
#include "HAL/IConsoleManager.h"

FAutoConsoleVariableRef CVarEnableDebugMode(
	TEXT("AbilitySystem.EnableDebugMode"),
	AbilityConsoleVars::bEnableDebugMode,
	TEXT("Whether or not to enable the Ability Debug mode.")
);

FAutoConsoleVariableRef CVarDrawMotionWarpingPoint(
	TEXT("AbilitySystem.DrawMotionWarpingPoint"),
	AbilityConsoleVars::bDrawMotionWarpingPoint,
	TEXT("Whether to draw AnimationWarppingPoint DebugBox"),
	ECVF_Default);

FAutoConsoleVariableRef CVarMotionWarpingPointDrawTime(
	TEXT("AbilitySystem.SetMotionWarpingPointDrawTime"),
	AbilityConsoleVars::MotionWarpingPointDrawTime,
	TEXT("Set Time to Draw AnimationWarppingPoint DebugBox"),
	ECVF_Default);
