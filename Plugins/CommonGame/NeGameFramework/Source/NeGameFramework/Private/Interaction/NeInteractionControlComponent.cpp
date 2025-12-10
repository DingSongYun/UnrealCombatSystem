// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Interaction/NeInteractionControlComponent.h"
#include "Interaction/INeInteractableInterface.h"
#include "Interaction/NeInteractAction.h"
#include "Components/PrimitiveComponent.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interaction/NeInteractionScanner.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeInteractionControlComponent)

UNeInteractionControlComponent::UNeInteractionControlComponent(const FObjectInitializer& Initializer) : Super(Initializer)
{
	InteractionOptions.Reserve(10);
}

void UNeInteractionControlComponent::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	for (const auto& ScanConfig : ScanConfigs)
	{
		FTimerHandle& TimerHandle = ScanTimerHandles.AddDefaulted_GetRef();
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, &ScanConfig]()
		{
			return ScanInteractions(ScanConfig);
		}), ScanConfig.Interval, true);
	}
}

void UNeInteractionControlComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		for (auto& TimerHandle : ScanTimerHandles)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
	Super::EndPlay(EndPlayReason);
}

TSoftClassPtr<UUserWidget> UNeInteractionControlComponent::GetOptionIndicatorWidget(const FNeInteractionOption& Option) const
{
	if (Option.OverrideWidgetClass.IsValid())
	{
		return Option.OverrideWidgetClass;
	}

	if (Option.InteractionAction)
	{
		return Option.InteractionAction->WidgetClass;
	}

	return nullptr;
}

void UNeInteractionControlComponent::ScanInteractions(const FNeInteractionScanConfig& ScanConfig)
{
	UWorld* World = GetWorld();
	AActor* OwnerPawn = GetPawn<APawn>();

	check(OwnerPawn && World);

	if (World == nullptr || OwnerPawn == nullptr)
	{
		return ;
	}

	InteractionOptions.Empty();

	const UClass* ScannerType = ScanConfig.ScannerType ? ScanConfig.ScannerType.Get() : UNeInteractionDefaultScanner::StaticClass();
	check(ScannerType);
	UNeInteractionScanner* Scanner = ScannerType->GetDefaultObject<UNeInteractionScanner>();
	TArray<TScriptInterface<INeInteractableInterface>> InteractableTargets;
	Scanner->ScanInteractions(World, this, OwnerPawn, ScanConfig, InteractableTargets);
	TArray<UObject*> InteractableTargetObjects;
	Scanner->ReceiveScanInteractions(World, this, OwnerPawn, ScanConfig, InteractableTargetObjects);
	for (UObject* InteractableObject : InteractableTargetObjects)
	{
		TScriptInterface<INeInteractableInterface> Interactable(InteractableObject);
		if (Interactable)
		{
			InteractableTargets.AddUnique(Interactable);
		}
	}

	if (InteractableTargets.Num() > 0)
	{
		FNeInteractionQuery InteractionQuery;
		InteractionQuery.QueryActor = OwnerPawn;

		for (TScriptInterface<INeInteractableInterface>& InteractiveTarget : InteractableTargets)
		{
			FInteractionOptionBuilder OptionBuilder(InteractiveTarget, InteractionOptions);
			InteractiveTarget->GatherInteractionOptions(InteractionQuery, OptionBuilder);
		}
	}

	HandleInteractionOptions();
}

void UNeInteractionControlComponent::HandleInteractionOptions()
{
	ReceivePostHandleInteractionOptions();
}

void UNeInteractionControlComponent::TriggerInteraction(const FNeInteractionOption& Option)
{
	if (Option.InteractionAction)
	{
		AController* Controller = GetController<AController>();
		check(Controller);
		Option.InteractionAction->TriggerInteraction(Controller->GetPawn(), Option.InteractableActor);
	}
}

