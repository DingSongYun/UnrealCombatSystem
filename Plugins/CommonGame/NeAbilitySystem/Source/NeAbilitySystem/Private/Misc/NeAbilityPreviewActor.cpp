// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/NeAbilityPreviewActor.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAbilityPreviewActor)

UNeAbilityPreviewActorCommon::UNeAbilityPreviewActorCommon(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	ActorClass = ACharacter::StaticClass();
}

AActor* UNeAbilityPreviewActorCommon::CreateActor(class UWorld* World, const FTransform& Transform)
{
	if (ActorClass == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	//SpawnParams.Owner = GetAbleOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActorAbsolute(ActorClass.Get(), Transform, SpawnParams);
	return NewActor;
}

AActor* UNeAbilityPreviewActorBlueprintType::SpawnActorDeferred(UClass* ActorClass, const FTransform& InTransform)
{
	return CachedWorld->SpawnActorDeferred<AActor>(ActorClass, InTransform);
}
