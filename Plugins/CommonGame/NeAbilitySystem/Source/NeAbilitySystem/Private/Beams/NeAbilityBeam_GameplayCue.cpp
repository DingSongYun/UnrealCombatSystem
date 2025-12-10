// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_GameplayCue.h"

#include "GameplayCueNotify_Burst.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAbilityBeam_GameplayCue)

UClass* UNeAbilityBeam_GameplayCue::GetSupportClass() const
{
	return UGameplayCueNotify_Static::StaticClass();
}
