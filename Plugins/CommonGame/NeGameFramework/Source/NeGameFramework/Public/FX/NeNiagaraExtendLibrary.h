// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NeNiagaraTypes.h"
#include "NeNiagaraExtendLibrary.generated.h"

/**
 * UNeNiagaraExtendLibrary
 * Niagara 扩展方法
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeNiagaraExtendLibrary : public UNiagaraFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Common spawn function with parameters */
	UFUNCTION(BlueprintCallable)
	static UNiagaraComponent* SpawnNiagaraWithParams(const FNiagaraSpawnParameters& SpawnParams);
};
