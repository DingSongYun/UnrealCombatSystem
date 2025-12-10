// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeInteractionOption.h"
#include "UObject/Interface.h"
#include "NeInteractionTypes.h"
#include "INeInteractableInterface.generated.h"

/**
 * UNeInteractableInterface
 * 可交互对象继承这个接口提供交互选项
 */
UINTERFACE(MinimalAPI)
class UNeInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**  */
class INeInteractableInterface
{
	GENERATED_BODY()

public:
	/**  */
	virtual void GatherInteractionOptions(const FNeInteractionQuery& InteractQuery, FInteractionOptionBuilder& Options) { }
};
