// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "NeGameFramworkGlobalFunction.generated.h"

/**
 * NeGameFramworkGlobalFunction
 * 用来存放GameFramwork中的全局接口
 */
class AActor;
class UActorComponent;

NEGAMEFRAMEWORK_API bool IsPIE();

NEGAMEFRAMEWORK_API bool IsDS(const UObject* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandalone(const UObject* WorldContext);
NEGAMEFRAMEWORK_API bool IsClient(const UObject* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandaloneOrClient(const UObject* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandaloneOrDS(const UObject* WorldContext);

NEGAMEFRAMEWORK_API bool IsDS(const AActor* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandalone(const AActor* WorldContext);
NEGAMEFRAMEWORK_API bool IsClient(const AActor* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandaloneOrClient(const AActor* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandaloneOrDS(const AActor* WorldContext);

NEGAMEFRAMEWORK_API bool IsDS(const UActorComponent* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandalone(const UActorComponent* WorldContext);
NEGAMEFRAMEWORK_API bool IsClient(const UActorComponent* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandaloneOrClient(const UActorComponent* WorldContext);
NEGAMEFRAMEWORK_API bool IsStandaloneOrDS(const UActorComponent* WorldContext);