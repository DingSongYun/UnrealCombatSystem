// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "NeInteractionOption.generated.h"

class INeInteractableInterface;

/** FNeInteractionOption */
USTRUCT(BlueprintType)
struct FNeInteractionOption
{
	GENERATED_BODY()

public:
	/** The interactable target */
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<INeInteractableInterface> InteractableTarget;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AActor> InteractableActor;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UActorComponent> InteractableComponent;

	/** Simple text the interaction might return */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	/** Simple sub-text the interaction might return */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SubText;

	/** 交互行为 */
	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<class UNeInteractAction> InteractionAction;

	/** 交互组件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<class UUserWidget> OverrideWidgetClass;

public:
	/** 是否可交互 */
	bool IsAvailable() const;

	/** 是否显示该交互选项 */
	bool IsValid() const;

	FORCEINLINE bool operator==(const FNeInteractionOption& Other) const
	{
		return InteractableTarget == Other.InteractableTarget &&
			OverrideWidgetClass == Other.OverrideWidgetClass &&
			InteractionAction == Other.InteractionAction &&
			Text.IdenticalTo(Other.Text) &&
			SubText.IdenticalTo(Other.SubText);
	}

	FORCEINLINE bool operator!=(const FNeInteractionOption& Other) const
	{
		return !operator==(Other);
	}

	FORCEINLINE bool operator<(const FNeInteractionOption& Other) const
	{
		return InteractableTarget.GetInterface() < Other.InteractableTarget.GetInterface();
	}
};

/** FInteractionOptionBuilder */
class FInteractionOptionBuilder
{
public:
	FInteractionOptionBuilder(TScriptInterface<INeInteractableInterface> InterfaceTargetScope, TArray<FNeInteractionOption>& InteractOptions);
	void AddInteractionOption(const FNeInteractionOption& Option) const;

private:
	TScriptInterface<INeInteractableInterface> Scope;
	TArray<FNeInteractionOption>& Options;
	TObjectPtr<AActor> TargetActor;
	TObjectPtr<UActorComponent> TargetComponent;
};