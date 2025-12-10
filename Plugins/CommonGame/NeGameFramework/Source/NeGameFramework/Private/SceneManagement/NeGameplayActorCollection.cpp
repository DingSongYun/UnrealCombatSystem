// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SceneManagement/NeGameplayActorCollection.h"

#include "GameFramework/Actor.h"

void UNeGameplayActorCollectionGeneric::Add(AActor* InActor)
{
	const UClass* ActorClass = InActor->GetClass();
	const uint32 ClassID = ActorClass->GetUniqueID();
	FGameplayActorArray& ActorArray = ClassifiedActors.FindOrAdd(ClassID);
	check(ActorArray.Actors.Contains(InActor) == false);
	ActorArray.Actors.Add(InActor);
}

void UNeGameplayActorCollectionGeneric::Remove(AActor* InActor)
{
	const UClass* ActorClass = InActor->GetClass();
	const uint32 ClassID = ActorClass->GetUniqueID();
	check(ClassifiedActors.Contains(ClassID));
	FGameplayActorArray& ActorArray = ClassifiedActors[ClassID];
	check(ActorArray.Actors.Contains(InActor));
	ActorArray.Actors.Remove(InActor);
}

bool UNeGameplayActorCollectionGeneric::FindSceneActorOfClass(UClass* ActorClass, TArray<AActor*>& OutActors) const
{
	const uint32 ClassID = ActorClass->GetUniqueID();
	const FGameplayActorArray* ActorArray = ClassifiedActors.Find(ClassID);
	if (ActorArray == nullptr)
	{
		return false;
	}

	for (const TObjectPtr<AActor>& ActorItem : ActorArray->Actors)
	{
		OutActors.Add(ActorItem);
	}

	return true;
}
