// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeamLinkage.h"
#include "NeAbilityBeam_GameplayEffect.generated.h"

/**
 * UNeAbilityBeam_GameplayCue
 * 桥接GE
 */
UCLASS(HideDropdown)
class NEABILITYSYSTEM_API UNeAbilityBeam_GameplayEffect : public UNeAbilityBeamLinkage
{
	GENERATED_BODY()

public:
	virtual UClass* GetSupportClass() const override;

};
