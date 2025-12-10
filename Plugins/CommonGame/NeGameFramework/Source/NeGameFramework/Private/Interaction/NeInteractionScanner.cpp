// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Interaction/NeInteractionScanner.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace InteractionUtils
{
	void AppendInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<INeInteractableInterface>>& OutInteractableTargets)
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (const AActor* OverlapActor = Overlap.GetActor())
			{
				TArray<UActorComponent*> InteractableComponents = OverlapActor->GetComponentsByInterface(UNeInteractableInterface::StaticClass());
				for (UActorComponent* Component : InteractableComponents)
				{
					TScriptInterface<INeInteractableInterface> InteractableComponent(Component);
					if (InteractableComponent)
					{
						OutInteractableTargets.AddUnique(InteractableComponent);
					}
				}
			}
		}
	}
}

void UNeInteractionDefaultScanner::ScanInteractions(UWorld* World, UNeInteractionControlComponent* Querier, AActor* Instigator, const FNeInteractionScanConfig& ScanConfig, TArray<TScriptInterface<INeInteractableInterface>>& OutInteractions)
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionScanner_Default), false);

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(OUT OverlapResults, Instigator->GetActorLocation(), FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(ScanConfig.Radius), Params);

	if (OverlapResults.Num() > 0)
	{
		InteractionUtils::AppendInteractableTargetsFromOverlapResults(OverlapResults, OUT OutInteractions);

	}
}
