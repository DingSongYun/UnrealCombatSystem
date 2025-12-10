// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Interaction/NeInteractAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeInteractAction)

void UNeInteractAction::Initialize(UObject* InContext, AActor* InOwnerActor)
{
	OwnerActor = InOwnerActor;
	ContextObject = InContext;
	ReceiveInitialize(InContext, OwnerActor);
}

void UNeInteractAction::TriggerInteraction(AActor* Instigator, AActor* Target)
{
	ReceiveTriggerInteraction(Instigator, Target);
}

void UNeInteractAction::CompleteInteraction()
{
	ReceiveCompleteInteraction();
}
