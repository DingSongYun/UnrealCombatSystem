// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeGameFramworkGlobalFunction.h"

#include "Editor.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Editor/EditorEngine.h"

//=============================================================================
/**
 * GlobalFunctions
 */

NEGAMEFRAMEWORK_API bool IsPIE()
{
#if WITH_EDITOR
	if (GEditor)
	{
		if (GEditor->IsPlayingSessionInEditor())
		{
			return true;
		}
	}
#endif
	return false;
}

NEGAMEFRAMEWORK_API bool IsDS(const UObject* WorldContext)
{
	check(WorldContext)
	return UKismetSystemLibrary::IsDedicatedServer(WorldContext);
}

NEGAMEFRAMEWORK_API bool IsStandalone(const UObject* WorldContext)
{
	check(WorldContext)
	return UKismetSystemLibrary::IsStandalone(WorldContext);
}

NEGAMEFRAMEWORK_API bool IsClient(const UObject* WorldContext)
{
	return !IsStandaloneOrClient(WorldContext);
}

NEGAMEFRAMEWORK_API bool IsStandaloneOrClient(const UObject* WorldContext)
{
	return IsStandalone(WorldContext) || IsClient(WorldContext);
}

NEGAMEFRAMEWORK_API bool IsStandaloneOrDS(const UObject* WorldContext)
{
	return IsStandalone(WorldContext) || IsDS(WorldContext);
}

NEGAMEFRAMEWORK_API bool IsDS(const AActor* WorldContext)
{
	check(WorldContext)
	const ENetMode NetMode = WorldContext->GetNetMode();
	return NetMode == NM_DedicatedServer;
}

NEGAMEFRAMEWORK_API bool IsStandalone(const AActor* WorldContext)
{
	check(WorldContext)
	const ENetMode NetMode = WorldContext->GetNetMode();
	return NetMode == NM_Standalone;
}

NEGAMEFRAMEWORK_API bool IsClient(const AActor* WorldContext)
{
	check(WorldContext)
	const ENetMode NetMode = WorldContext->GetNetMode();
	return NetMode == NM_Client;
}

NEGAMEFRAMEWORK_API bool IsStandaloneOrClient(const AActor* WorldContext)
{
	return IsStandalone(WorldContext) || IsClient(WorldContext);
}

NEGAMEFRAMEWORK_API bool IsStandaloneOrDS(const AActor* WorldContext)
{
	return IsStandalone(WorldContext) || IsDS(WorldContext);
}

NEGAMEFRAMEWORK_API bool IsDS(const UActorComponent* WorldContext)
{
	const AActor* Actor = IsValid(WorldContext) ? WorldContext->GetOwner() : nullptr;
	return IsDS(Actor);
}

NEGAMEFRAMEWORK_API bool IsStandalone(const UActorComponent* WorldContext)
{
	const AActor* Actor = IsValid(WorldContext) ? WorldContext->GetOwner() : nullptr;
	return IsStandalone(Actor);
}

NEGAMEFRAMEWORK_API bool IsClient(const UActorComponent* WorldContext)
{
	const AActor* Actor = IsValid(WorldContext) ? WorldContext->GetOwner() : nullptr;
	return IsClient(Actor);
}

NEGAMEFRAMEWORK_API bool IsStandaloneOrClient(const UActorComponent* WorldContext)
{
	const AActor* Actor = IsValid(WorldContext) ? WorldContext->GetOwner() : nullptr;
	return IsStandaloneOrClient(Actor);
}

NEGAMEFRAMEWORK_API bool IsStandaloneOrDS(const UActorComponent* WorldContext)
{
	const AActor* Actor = IsValid(WorldContext) ? WorldContext->GetOwner() : nullptr;
	return IsStandaloneOrDS(Actor);
}
