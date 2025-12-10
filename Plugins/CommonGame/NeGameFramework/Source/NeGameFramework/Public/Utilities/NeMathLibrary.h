// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NeMathLibrary.generated.h"

/**
 * 一些数学库
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr FORCEINLINE_DEBUGGABLE float CubicInterp( const float& P0, const float& T0, const float& P1, const float& T1, const float& A )
	{
		return FMath::CubicInterp<float>(P0, T0, P1, T1, A);
	}
};
