// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameplayAbilitiesBlueprintFactory.h"
#include "NeAbility.h"
#include "NeAbilitySystemSettings.h"
#include "Misc/NeAbilityPreviewActor.h"
#include "GameplayAbilityBlueprint.h"
#include "NeAbilityBlueprintFactory.generated.h"

/**
 * UNeAbilityBlueprintFactory
 *
 * 本来想直接继承UGameplayAbilitiesBlueprintFactory，但是这个类没有添加dllexport，继承会有问题
 * 改成组合的方式
 */
UCLASS(HideCategories=Object, MinimalAPI)
class UNeAbilityBlueprintFactory : public UFactory
{
	GENERATED_BODY()

public:
	UNeAbilityBlueprintFactory(const FObjectInitializer& Initializer) : Super(Initializer)
	{
		bCreateNew = false;
		bEditAfterNew = true;
		SupportedClass = UGameplayAbilityBlueprint::StaticClass();
		GameplayAbilitiesBlueprintFactory = CreateDefaultSubobject<UGameplayAbilitiesBlueprintFactory>("AbilityFactory");
	}

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override
	{
		return GameplayAbilitiesBlueprintFactory->ConfigureProperties();
	}

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
	}

	 virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override
	 {
	 	UObject* ObjectNew = GameplayAbilitiesBlueprintFactory->FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, CallingContext);

	 	if (UNeAbility* Ability = Cast<UNeAbility>(ObjectNew))
	 	{
	 		Ability->PostAssetCreate();
	 	}

	 	return ObjectNew;
	 }

	void SetParent(UClass* ParentClass)
	{
		Cast<UGameplayAbilitiesBlueprintFactory>(GameplayAbilitiesBlueprintFactory)->ParentClass = ParentClass;
	}
	//~ Begin UFactory Interface
public:
	UPROPERTY()
	UFactory* GameplayAbilitiesBlueprintFactory = nullptr;
};
