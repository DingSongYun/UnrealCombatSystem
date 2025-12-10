// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AlphaBlend.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AlphaBlendHelper.generated.h"

/**
 * UAlphaBlendHelper
 *
 * 提供给蓝图和脚本(Python)层使用
 */
UCLASS()
class UAlphaBlendHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "AlphaBlend", meta=(DisplayName="GetBlendedValue", DefaultToSelf = Target))
	static float GetBlendedValue(const FAlphaBlend& Target)
	{
		return Target.GetBlendedValue();

	}

	UFUNCTION(BlueprintPure, Category = "AlphaBlend", meta=(DefaultToSelf = Target))
	static float GetBlendTimeRemaining(const FAlphaBlend& Target)
	{
		return Target.GetBlendTimeRemaining();
	}

	UFUNCTION(BlueprintPure, Category = "AlphaBlend", meta=(DefaultToSelf = Target))
	static float GetBlendTime(const FAlphaBlend& Target)
	{
		return Target.GetBlendTime();
	}

	UFUNCTION(BlueprintPure, Category = "AlphaBlend", meta=(DefaultToSelf = Target))
	static EAlphaBlendOption GetBlendOption(const FAlphaBlend& Target)
	{
		return Target.GetBlendOption();
	}

	UFUNCTION(BlueprintCallable, Category = "AlphaBlend", meta=(DisplayName="Update", DefaultToSelf = Target))
	static const FAlphaBlend& Update(const FAlphaBlend& Target, float DeltaTime)
	{
		FAlphaBlend* Blender = const_cast<FAlphaBlend*>(&Target);
		Blender->Update(DeltaTime);
		return Target;
	}

	UFUNCTION(BlueprintCallable, Category = "AlphaBlend", meta=(DisplayName="Reset", DefaultToSelf = Target))
	static const FAlphaBlend& Reset(const FAlphaBlend& Target)
	{
		FAlphaBlend* Blender = const_cast<FAlphaBlend*>(&Target);
		Blender->Reset();
		return Target;
	}

	UFUNCTION(BlueprintCallable, Category = "AlphaBlend", meta=(DisplayName="SetValueRange", DefaultToSelf = Target))
	static const FAlphaBlend& SetValueRange(const FAlphaBlend& Target, float Begin, float Desired)
	{
		FAlphaBlend* Blender = const_cast<FAlphaBlend*>(&Target);
		Blender->SetValueRange(Begin, Desired);
		return Target;
	}

	UFUNCTION(BlueprintCallable, Category = "AlphaBlend", meta=(DisplayName="SetAlpha", DefaultToSelf = Target))
	static const FAlphaBlend& SetAlpha(const FAlphaBlend& Target, float DeltaTime)
	{
		FAlphaBlend* Blender = const_cast<FAlphaBlend*>(&Target);
		Blender->SetAlpha(DeltaTime);
		return Target;
	}

	UFUNCTION(BlueprintCallable, Category = "AlphaBlend", meta=(DisplayName="SetAlpha", DefaultToSelf = Target))
	static float GetAlpha(const FAlphaBlend& Target)
	{
		return Target.GetAlpha();
	}

	UFUNCTION(BlueprintPure, Category = "AlphaBlend", meta=(DisplayName="IsComplete", DefaultToSelf = Target))
	static bool IsComplete(const FAlphaBlend& Target)
	{
		return Target.IsComplete();
	}
};