// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Input/NeEnhancedPlayerInput.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Engine/Canvas.h"

static TMap<FGameplayTag, TWeakObjectPtr<UInputAction>> sInputActionTagMapping;

UNeEnhancedPlayerInput::UNeEnhancedPlayerInput() : Super()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		BuildExternalInputRegistration();
	}
}

void UNeEnhancedPlayerInput::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString("");
	DisplayDebugManager.DrawString("");
	DisplayDebugManager.DrawString(FString::Printf(TEXT("InputAction Tag Mapping")));
	DisplayDebugManager.DrawString("");
	DisplayDebugManager.SetDrawColor(FColor::Green);
	for (auto Pair : sInputActionTagMapping)
	{
		const FGameplayTag& Tag = Pair.Key;
		UInputAction* Action = Pair.Value.Get();
		DisplayDebugManager.DrawString(FString::Printf(TEXT("	%s : %s"), *Tag.ToString(), Action ? *Action->GetName() : TEXT("None")));
	}
}

void UNeEnhancedPlayerInput::BuildExternalInputRegistration()
{
	UNeInputRegistration* InputRegistration = GetDefault<UNeEnhancedPlayerInput>()->ExternalInputRegistration.LoadSynchronous();
	if (InputRegistration == nullptr) return;
	for (const FNeTaggedInputAction& InputData : InputRegistration->InputData)
	{
		if (!InputData.InputTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid input tag bind with action %s"), *GetNameSafe(InputData.InputAction));
			continue;
		}

		UpdateActionTagMapping(InputData.InputAction.Get(), InputData.InputTag, true);
	}
}

void UNeEnhancedPlayerInput::UpdateActionTagMapping(const UInputAction* InAction, const FGameplayTag& InTag, bool bTagUpdate)
{
	if (!InTag.IsValid() || InAction == nullptr) return;

	if (bTagUpdate)
	{
		for (auto It = sInputActionTagMapping.CreateIterator(); It; ++ It)
		{
			const FGameplayTag& Tag = It.Key();
			UInputAction* Action = It.Value().Get();
			if (Action == InAction && Tag == InTag)
			{
				// 无需更新
				return ;
			}
			else if (Action == InAction && Tag != InTag)
			{
				// 删除，之后重新添加
				It.RemoveCurrent();
				break;
			}
		}
	}
	sInputActionTagMapping.Add(InTag, const_cast<UInputAction*>(InAction));
}

UInputAction* UNeEnhancedPlayerInput::FindActionOfTag(const FGameplayTag& InputTag)
{
	if (sInputActionTagMapping.Contains(InputTag))
	{
		return sInputActionTagMapping[InputTag].Get();
	}
	return nullptr;
}
