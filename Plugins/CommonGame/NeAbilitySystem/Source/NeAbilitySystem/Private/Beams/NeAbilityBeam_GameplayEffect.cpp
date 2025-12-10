// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_GameplayEffect.h"

#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAbilityBeam_GameplayEffect)

UClass* UNeAbilityBeam_GameplayEffect::GetSupportClass() const
{
	return UGameplayEffect::StaticClass();
}
