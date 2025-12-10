// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeComponentPath.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

//=============================================================================
/**
 * FNeComponentPath
 */
void FNeComponentPath::SetComponent(const UActorComponent* InComponent)
{
	if (InComponent)
	{
		AActor* OwnerActor = InComponent->GetOwner();
		check(OwnerActor);
		UClass* ActorClass = OwnerActor->GetClass();
		for (TFieldIterator<FProperty> PropIt(ActorClass); PropIt; ++PropIt)
		{
			if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(*PropIt))
			{
				UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue_InContainer(OwnerActor);
				if (ObjectValue == InComponent)
				{
					Path = PropIt->GetFName();
					return;
				}
			}
		}
		TArray<UActorComponent*> Components;
		OwnerActor->GetComponents(Components);
		for (auto Component : Components)
		{
			if (Component == InComponent)
			{
				Path = Component->GetFName();
				return;
			}
		}
	}
	else
	{
		Path = NAME_None;
	}
}

UActorComponent* FNeComponentPath::ResolveComponent(AActor* OwnerActor) const
{
	if (OwnerActor)
	{
		FProperty* Property = OwnerActor->GetClass()->FindPropertyByName(Path);
		if (Property)
		{
			UObject* ObjectValue = CastField<FObjectProperty>(Property)->GetObjectPropertyValue_InContainer(OwnerActor);
			return Cast<UActorComponent>(ObjectValue);
		}
		TArray<UActorComponent*> Components;
		OwnerActor->GetComponents(Components);
		for (auto Component : Components)
		{
			if (Component->GetFName() == Path)
				return Component;
		}
	}
	return nullptr;
}

#if WITH_EDITOR
void FNeComponentSocketName::SetHostComponent(AActor* HostActor, const FNeComponentPath& Path)
{
	HostComponent = Cast<USceneComponent>(Path.ResolveComponent(HostActor));

	if (HostComponentPath == Path)
		return;
	HostComponentPath = Path;
	if (HostComponent == nullptr)
	{
		SocketName = NAME_None;
	}
	else
	{
		TArray<FName> AllSocketNames = HostComponent->GetAllSocketNames();
		if (AllSocketNames.Num() > 0)
		{
			SocketName = AllSocketNames[0];
		}
		else
			SocketName = NAME_None;
	}
	OnHostComponentChange.ExecuteIfBound();
}
#endif
