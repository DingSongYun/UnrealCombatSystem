// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeamLinkage.h"
#include "NeAbilityBeam_GameplayCue.generated.h"

/**
 * UNeAbilityBeam_GameplayCue
 * 桥接GC
 */
UCLASS(HideDropdown)
class NEABILITYSYSTEM_API UNeAbilityBeam_GameplayCue: public UNeAbilityBeamLinkage
{
	GENERATED_BODY()

public:
	virtual UClass* GetSupportClass() const override;
};
