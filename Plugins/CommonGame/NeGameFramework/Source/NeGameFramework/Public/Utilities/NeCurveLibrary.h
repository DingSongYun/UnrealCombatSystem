// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Curves/CurveFloat.h"
#include "NeCurveLibrary.generated.h"

/**
 * 一些曲线操作的库
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeCurveLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static float GetFloatValue(const FRuntimeFloatCurve& Curve, float Time);
};
