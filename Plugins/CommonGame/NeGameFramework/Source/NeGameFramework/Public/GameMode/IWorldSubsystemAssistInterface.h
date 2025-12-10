// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Subsystems/WorldSubsystem.h"
#include "IWorldSubsystemAssistInterface.generated.h"

UINTERFACE(BlueprintType)
class NEGAMEFRAMEWORK_API UWorldSubsystemAssistInterface : public UInterface
{
	GENERATED_BODY()
};

class IWorldSubsystemAssistInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	virtual bool SupportSubsystem(TSubclassOf<UWorldSubsystem> SubsystemClass) const { return false; }
};