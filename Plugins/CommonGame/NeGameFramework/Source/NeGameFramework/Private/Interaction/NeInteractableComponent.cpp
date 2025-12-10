// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Interaction/NeInteractableComponent.h"

#include "Interaction/NeInteractAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeInteractableComponent)

UNeInteractableComponent::UNeInteractableComponent(const FObjectInitializer& Initializer)
{
}

void UNeInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	for (const auto& Option : InteractionOptions)
	{
		if (Option.InteractionAction)
		{
			Option.InteractionAction->Initialize(this, OwnerActor);
		}
	}
}

void UNeInteractableComponent::StartInteraction(AActor* Instigator)
{
	InteractionWith = Instigator;
}

void UNeInteractableComponent::EndInteraction()
{
	InteractionWith = nullptr;
}

void UNeInteractableComponent::GatherInteractionOptions(const FNeInteractionQuery& InteractQuery, FInteractionOptionBuilder& Options)
{
	for (const auto& Option : InteractionOptions)
	{
		if (Option.IsValid())
		{
			Options.AddInteractionOption(Option);
		}
	}
}
