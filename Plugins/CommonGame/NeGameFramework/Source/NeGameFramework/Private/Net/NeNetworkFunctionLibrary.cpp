// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Net/NeNetworkFunctionLibrary.h"
#include "GameFramework/Actor.h"

void UNeNetworkFunctionLibrary::AddActorReplicatedSubObject(AActor* Actor, UObject* SubObject, ELifetimeCondition NetCondition)
{
	check(Actor && SubObject);
	Actor->AddReplicatedSubObject(SubObject, NetCondition);
}

void UNeNetworkFunctionLibrary::RemoveActorReplicatedSubObject(AActor* Actor, UObject* SubObject)
{
	check(Actor && SubObject);
	Actor->RemoveReplicatedSubObject(SubObject);
}

void UNeNetworkFunctionLibrary::AddComponentReplicatedSubObject(UActorComponent* ActorComponent, UObject* SubObject, ELifetimeCondition NetCondition)
{
	check(ActorComponent && SubObject);

	ActorComponent->AddReplicatedSubObject(SubObject, NetCondition);
}

void UNeNetworkFunctionLibrary::RemoveComponentReplicatedSubObject(UActorComponent* ActorComponent, UObject* SubObject)
{
	check(ActorComponent && SubObject);

	ActorComponent->RemoveReplicatedSubObject(SubObject);
}

