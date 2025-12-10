// Copyright NetEase Games, Inc. All Rights Reserved.


#include "Physics/NeBlueprintPhysicsVolume.h"

float ANeBlueprintPhysicsVolume::GetGravityZ() const
{
	return Super::GetGravityZ();
}

void ANeBlueprintPhysicsVolume::ActorEnteredVolume(AActor* Other)
{
	Super::ActorEnteredVolume(Other);
	ReceiveActorEnteredVolume(Other);
}

void ANeBlueprintPhysicsVolume::ActorLeavingVolume(AActor* Other)
{
	Super::ActorLeavingVolume(Other);
	ReceiveActorLeavingVolume(Other);
}

bool ANeBlueprintPhysicsVolume::IsOverlapInVolume(const USceneComponent& TestComponent) const
{
	return Super::IsOverlapInVolume(TestComponent);
}

void ANeBlueprintDefaultPhysicsVolume::ActorEnteredVolume(AActor* Other)
{
	Super::ActorEnteredVolume(Other);
	ReceiveActorEnteredVolume(Other);
}

void ANeBlueprintDefaultPhysicsVolume::ActorLeavingVolume(AActor* Other)
{
	Super::ActorLeavingVolume(Other);
	ReceiveActorLeavingVolume(Other);
}
