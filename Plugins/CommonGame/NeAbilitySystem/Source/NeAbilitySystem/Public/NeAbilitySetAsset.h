// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Engine/DataAsset.h"
#include "NeAbilityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NeAbilitySetAsset.generated.h"

/** AbilitySet向技能组件授权的句柄 */
USTRUCT(BlueprintType)
struct FNeAbilitySetGrantedHandle
{
	GENERATED_BODY()

	void PutAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);

protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
};

/**
 * UNeAbilitiesAsset
 * 技能配置集合
 */
UCLASS(BlueprintType)
class NEABILITYSYSTEM_API UNeAbilitySetAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 向技能组件中注册 */
	UFUNCTION(BlueprintCallable, Category="AbilitySet")
	bool GiveToAbilitySystem(UAbilitySystemComponent* AbilityComponent, FNeAbilitySetGrantedHandle& OutGrantedHandles, UObject* SourceObject = nullptr) const;

	UPROPERTY(EditDefaultsOnly, Category="AbilitySet", meta=(ShowOnlyInnerProperties=true))
	TArray<FNeSlottedAbility> Abilities;
};


/**
 * FNeAbilitySet
 * 技能集, 可配置技能集资源，也可以单独配置技能
 */
USTRUCT(BlueprintType)
struct FNeAbilitySet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="AbilitySet", meta=(ShowOnlyInnerProperties=true))
	TArray<FNeSlottedAbility> Abilities;

	UPROPERTY(EditDefaultsOnly, Category="AbilitySet")
	TArray<UNeAbilitySetAsset*> AbilitySets;

	/** 向技能组件中注册 */
	bool GiveToAbilitySystem(UAbilitySystemComponent* AbilityComponent, FNeAbilitySetGrantedHandle& OutGrantedHandles, UObject* SourceObject = nullptr) const;
};


UCLASS()
class UNeAbilitySetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 蓝图用方法: 蓝图里向技能组件授权FNeAbilitySet中的技能 */
	UFUNCTION(BlueprintCallable, Category="AbilitySet", DisplayName="Give To Ability System")
	static bool K2_GiveToAbilitySystem(UAbilitySystemComponent* AbilityComponent, const FNeAbilitySet& InAbilitySet,
								FNeAbilitySetGrantedHandle& OutGrantedHandles,
								UObject* SourceObject = nullptr);

private:
	friend struct FNeAbilitySet;
	friend class UNeAbilitySetAsset;

	/** 向技能组件中注册技能 */
	static bool GrantAbilities(UAbilitySystemComponent* AbilityComponent, const TArray<FNeSlottedAbility>& InAbilities,
						FNeAbilitySetGrantedHandle& OutGrantedHandles,
						UObject* SourceObject = nullptr);
};
