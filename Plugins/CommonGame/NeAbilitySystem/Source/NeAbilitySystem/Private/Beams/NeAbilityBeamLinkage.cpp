// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeamLinkage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAbilityBeamLinkage)

#if WITH_EDITOR
TMap<TWeakObjectPtr<UClass>, UNeAbilityBeamLinkage::FBeamLinkageDesc> LinkageTypes;
#endif

UNeAbilityBeamLinkage::UNeAbilityBeamLinkage(const FObjectInitializer& Initializer) : Super(Initializer)
{
}

void UNeAbilityBeamLinkage::PostCDOContruct()
{
#if WITH_EDITOR
	RegisterLinkType(this);
#endif

	Super::PostCDOContruct();
}

void UNeAbilityBeamLinkage::InitialLink(UClass* LinkClass)
{
	LinkedClass = LinkClass;
}

#if WITH_EDITOR

void UNeAbilityBeamLinkage::RegisterLinkType(UNeAbilityBeamLinkage* BeamLinkage)
{
	check(BeamLinkage);

	UClass* LinkClass = BeamLinkage->GetSupportClass();

	checkf(LinkClass || BeamLinkage->GetClass() == UNeAbilityBeamLinkage::StaticClass(), TEXT("The beam(%s) linked to a null class."), *BeamLinkage->GetName());

	if (LinkClass)
	{
		LinkageTypes.Add(LinkClass, {BeamLinkage->GetClass()});
	}
}

TArray<UClass*> UNeAbilityBeamLinkage::GetRegisteredLinkTypes()
{
	TArray<UClass*> OutTypes;
	for (const auto& Type : LinkageTypes)
	{
		if (Type.Key.IsValid()) OutTypes.Add(Type.Key.Get());
	}

	return OutTypes;
}

bool UNeAbilityBeamLinkage::IsBeamLinkType(UClass* LinkType)
{
	if (LinkType == nullptr) return false;

	for (const auto& Pair : LinkageTypes)
	{
		if (LinkType->IsChildOf(Pair.Key.Get()))
		{
			return true;
		}
	}

	return false;
}

const UNeAbilityBeamLinkage::FBeamLinkageDesc* UNeAbilityBeamLinkage::GetBeamLinkageDesc(UClass* LinkType)
{
	if (LinkType == nullptr) return nullptr;

	for (const auto& Pair : LinkageTypes)
	{
		if (LinkType->IsChildOf(Pair.Key.Get()))
		{
			return &Pair.Value;
		}
	}

	return nullptr;
}

#endif
