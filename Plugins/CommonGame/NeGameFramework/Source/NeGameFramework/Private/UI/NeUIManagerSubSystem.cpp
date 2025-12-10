// Copyright NetEase Games, Inc. All Rights Reserved.

#include "UI/NeUIManagerSubSystem.h"
#include "UI/NeGameUIPolicy.h"
#include "CommonInputSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Player/NeLocalPlayer.h"
#include "UI/NePrimaryGameLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeUIManagerSubSystem)

int32 UNeUIManagerSubsystem::InputSuspensionsCount = 0;

void UNeUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (!CurrentPolicy && !DefaultUIPolicyClass.IsNull())
	{
		TSubclassOf<UNeGameUIPolicy> PolicyClass = DefaultUIPolicyClass.LoadSynchronous();
		SetUIPolicy(NewObject<UNeGameUIPolicy>(this, PolicyClass));
	}

	UGameInstance* GameIsntance = GetGameInstance();
	check(GameIsntance);
	GameIsntance->OnLocalPlayerAddedEvent.AddUObject(this, &UNeUIManagerSubsystem::OnPlayerAdded);
	GameIsntance->OnLocalPlayerRemovedEvent.AddUObject(this, &UNeUIManagerSubsystem::OnPlayerRemoved);

	ReceiveInitialize();
}

void UNeUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	SetUIPolicy(nullptr);

	if (UGameInstance* GameIsntance = GetGameInstance())
	{
		GameIsntance->OnLocalPlayerAddedEvent.RemoveAll(this);
		GameIsntance->OnLocalPlayerRemovedEvent.RemoveAll(this);
	}
	ReceiveDeinitialize();
}

bool UNeUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// Only create an instance if there is no override implementation defined elsewhere
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UNeUIManagerSubsystem::SetGameUIVisibility(bool bVisible, ULocalPlayer* Player)
{
	SetGameUIVisibility(Player ? Player : GetGameInstance()->GetFirstGamePlayer(),
		bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UNeUIManagerSubsystem::SetGameUIVisibility(ULocalPlayer* Player, ESlateVisibility Visibility) const
{
	check(Player);
	if (UNePrimaryGameLayout* RootLayout = CurrentPolicy->GetRootLayout(Player))
	{
		RootLayout->SetVisibility(Visibility);
	}
}

void UNeUIManagerSubsystem::SetUIPolicy(UNeGameUIPolicy* InPolicy)
{
	if (CurrentPolicy != InPolicy)
	{
		CurrentPolicy = InPolicy;

		//TODO: Notify policy changed?
	}
}

UCommonActivatableWidget* UNeUIManagerSubsystem::AddWidgetToLayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (IsValid(CurrentPolicy))
	{
		if (UNePrimaryGameLayout* RootLayout = CurrentPolicy->GetRootLayout(LocalPlayer))
		{
			return RootLayout->PushWidgetToLayerStack(LayerName, WidgetClass);
		}
	}

	return nullptr;
}

void UNeUIManagerSubsystem::RemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget)
	{
		// Ignore request to pop an already deleted widget
		return;
	}

	if (const ULocalPlayer* LocalPlayer = ActivatableWidget->GetOwningLocalPlayer())
	{
		if (IsValid(CurrentPolicy))
		{
			if (UNePrimaryGameLayout* RootLayout = CurrentPolicy->GetRootLayout(LocalPlayer))
			{
				RootLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
			}
		}
	}
}

void UNeUIManagerSubsystem::OnPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetGameInstance()->GetWorld()))
		{
			CurrentPolicy->CreateLayoutWidget(LocalPlayer);
		}
		else
		{
			if (UNeLocalPlayer* MyLocalPlayer = Cast<UNeLocalPlayer>(LocalPlayer))
			{
				MyLocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UNeLocalPlayer*, APlayerController* PlayerController)
				{
					CurrentPolicy->CreateLayoutWidget(PlayerController->GetLocalPlayer());
				});
			}
		}

	}
}

void UNeUIManagerSubsystem::OnPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->DestroyLayoutWidget(LocalPlayer);
	}
}

FName UNeUIManagerSubsystem::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName UNeUIManagerSubsystem::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensionsCount++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensionsCount);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UNeUIManagerSubsystem::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UNeUIManagerSubsystem::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None)
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}
