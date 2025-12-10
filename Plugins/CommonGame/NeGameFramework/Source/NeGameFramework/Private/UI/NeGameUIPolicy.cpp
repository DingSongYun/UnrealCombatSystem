// Copyright Epic Games, Inc. All Rights Reserved.
// Copy from Lyra project

#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/NeGameUIPolicy.h"
#include "UI/NeUIManagerSubSystem.h"
#include "UI/NePrimaryGameLayout.h"
#include "Engine/Engine.h"
#include "NeLogGameFramework.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeGameUIPolicy)

// Static
UNeGameUIPolicy* UNeGameUIPolicy::GetGameUIPolicy(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UNeUIManagerSubsystem* UIManager = UGameInstance::GetSubsystem<UNeUIManagerSubsystem>(GameInstance))
			{
				return UIManager->GetCurrentUIPolicy();
			}
		}
	}

	return nullptr;
}

UNeUIManagerSubsystem* UNeGameUIPolicy::GetOwningUIManager() const
{
	return CastChecked<UNeUIManagerSubsystem>(GetOuter());
}

UWorld* UNeGameUIPolicy::GetWorld() const
{
	return GetOwningUIManager()->GetGameInstance()->GetWorld();
}

UNePrimaryGameLayout* UNeGameUIPolicy::GetRootLayout(const ULocalPlayer* LocalPlayer) const
{
	const FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}

void UNeGameUIPolicy::AddLayoutToViewport(ULocalPlayer* LocalPlayer, UNePrimaryGameLayout* Layout)
{
	UE_LOG(LogNeUI, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UNeGameUIPolicy::RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UNePrimaryGameLayout* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogNeUI, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogNeUI, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UNeGameUIPolicy::OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UNePrimaryGameLayout* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// So our controller will work in PIE without needing to click in the viewport
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

void UNeGameUIPolicy::OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UNePrimaryGameLayout* Layout)
{
	
}

void UNeGameUIPolicy::OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UNePrimaryGameLayout* Layout)
{
	
}

void UNeGameUIPolicy::RequestPrimaryControl(UNePrimaryGameLayout* Layout)
{
	if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && Layout->IsDormant())
	{
		for (const FRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UNePrimaryGameLayout* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}
}

void UNeGameUIPolicy::CreateLayoutWidget(ULocalPlayer* LocalPlayer)
{
	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		TSubclassOf<UNePrimaryGameLayout> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UNePrimaryGameLayout* NewLayoutObject = CreateWidget<UNePrimaryGameLayout>(PlayerController, LayoutWidgetClass);
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);
			
			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

void UNeGameUIPolicy::DestroyLayoutWidget(ULocalPlayer* LocalPlayer)
{
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		FRootViewportLayoutInfo& LayoutInfo = RootViewportLayouts[LayoutInfoIdx];
		UNePrimaryGameLayout* RootLayout = LayoutInfo.RootLayout;

		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo.RootLayout);
		LayoutInfo.bAddedToViewport = false;

		if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
		{
			if (RootLayout && !RootLayout->IsDormant())
			{
				// We're removing a secondary player's root while it's in control - transfer control back to the primary player's root
				RootLayout->SetIsDormant(true);
				for (const FRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
				{
					if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
					{
						if (UNePrimaryGameLayout* PrimaryRootLayout = RootLayoutInfo.RootLayout)
						{
							PrimaryRootLayout->SetIsDormant(false);
						}
					}
				}
			}
		}
		
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);
		OnRootLayoutReleased(LocalPlayer, RootLayout);
	}
}

TSubclassOf<UNePrimaryGameLayout> UNeGameUIPolicy::GetLayoutWidgetClass(ULocalPlayer* LocalPlayer)
{
	return LayoutClass.LoadSynchronous();
}
