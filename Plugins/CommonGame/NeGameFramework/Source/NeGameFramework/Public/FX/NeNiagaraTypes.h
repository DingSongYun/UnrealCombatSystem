// Copyright NetEase Games, Inc. All Rights Reserved.
// 本文件存放项目扩展Niagara时所用的一些类型

#pragma once
#include "CoreMinimal.h"
#include "Particles/ParticleSystemComponent.h"
#include "NeNiagaraTypes.generated.h"

/** EFxAttachPolicy: 跟随策略 */
UENUM(BlueprintType)
enum class EFxAttachPolicy : uint8
{
	LocationAndRotation			= 0					UMETA(DisplayName = "Location和Rotation都跟随"),
	OnlyLocation									UMETA(DisplayName = "只做Location跟随"),
	OnlyRotation									UMETA(DisplayName = "只做Rotation跟随"),
};

/** Parameters for niagara spawn*/
USTRUCT(Blueprintable)
struct NEGAMEFRAMEWORK_API FNiagaraSpawnParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Spawner = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlomoWithSpawner = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNeedAttach = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFxAttachPolicy AttachPolicy = EFxAttachPolicy::LocationAndRotation;

	/** FX spawn parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFXSystemSpawnParameters FxSpawnInfo;
};

