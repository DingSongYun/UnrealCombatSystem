// Fill out your copyright notice in the Description page of Project Settings.

#include "NeAbilitySetAsset.h"
#include "AbilitySystemLog.h"

void FNeAbilitySetGrantedHandle::PutAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

bool UNeAbilitySetAsset::GiveToAbilitySystem(UAbilitySystemComponent* AbilityComponent, FNeAbilitySetGrantedHandle& OutGrantedHandles, UObject* SourceObject) const
{
	return UNeAbilitySetLibrary::GrantAbilities(AbilityComponent, Abilities, OutGrantedHandles, SourceObject);
}

bool FNeAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* AbilityComponent, FNeAbilitySetGrantedHandle& OutGrantedHandles, UObject* SourceObject) const
{
	bool Result = true;
	for (const UNeAbilitySetAsset* AbilitySetAsset : AbilitySets)
	{
		Result = Result && AbilitySetAsset->GiveToAbilitySystem(AbilityComponent, OutGrantedHandles, SourceObject);
	}

	Result = Result && UNeAbilitySetLibrary::GrantAbilities(AbilityComponent, Abilities, OutGrantedHandles, SourceObject);
	return Result;
}

bool UNeAbilitySetLibrary::K2_GiveToAbilitySystem(UAbilitySystemComponent* AbilityComponent,
												const FNeAbilitySet& InAbilitySet,
												FNeAbilitySetGrantedHandle& OutGrantedHandles,
												UObject* SourceObject)
{
	return InAbilitySet.GiveToAbilitySystem(AbilityComponent, OutGrantedHandles, SourceObject);
}

bool UNeAbilitySetLibrary::GrantAbilities(UAbilitySystemComponent* ASC, const TArray<FNeSlottedAbility>& InAbilities,
	FNeAbilitySetGrantedHandle& OutGrantedHandles, UObject* SourceObject)
{
	check(ASC);

	if (!ASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return false;
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < InAbilities.Num(); ++AbilityIndex)
	{
		const FNeSlottedAbility& AbilityToGrant = InAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogAbilitySystem, Error, TEXT("GrantedGameplayAbilities[%d] on ability set is not valid."), AbilityIndex);
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.SlotTag);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);

		OutGrantedHandles.PutAbilitySpecHandle(AbilitySpecHandle);
	}

	return true;
}
