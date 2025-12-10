// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Interaction/NeInteractionOption.h"

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Interaction/NeInteractAction.h"

bool FNeInteractionOption::IsAvailable() const
{
	return true;
}

bool FNeInteractionOption::IsValid() const
{
	if (InteractionAction)
	{
		return InteractionAction->IsValid();
	}

	return false;
}

FInteractionOptionBuilder::FInteractionOptionBuilder(TScriptInterface<INeInteractableInterface> InterfaceTargetScope, TArray<FNeInteractionOption>& InteractOptions)
	: Scope(InterfaceTargetScope)
	, Options(InteractOptions)
{
	UObject* InterfaceObject = InterfaceTargetScope.GetObject();
	if (UActorComponent* Component = Cast<UActorComponent>(InterfaceObject))
	{
		TargetComponent = Component;
		TargetActor = Component->GetOwner();
	}
	else if(AActor* Actor = Cast<AActor>(InterfaceObject))
	{
		TargetComponent = nullptr;
		TargetActor = Actor;
	}
}

void FInteractionOptionBuilder::AddInteractionOption(const FNeInteractionOption& Option) const
{
	FNeInteractionOption& OptionEntry = Options.Add_GetRef(Option);
	OptionEntry.InteractableTarget = Scope;
	OptionEntry.InteractableActor = TargetActor;
	OptionEntry.InteractableComponent = TargetComponent;
}
