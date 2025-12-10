// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Input/NeInputAction.h"

#include "Input/NeEnhancedPlayerInput.h"

UNeInputAction::UNeInputAction(const FObjectInitializer& Initializer) : Super(Initializer)
{
	UNeEnhancedPlayerInput::UpdateActionTagMapping(this, ActionTag);
}

void UNeInputAction::PostLoad()
{
	Super::PostLoad();
	UNeEnhancedPlayerInput::UpdateActionTagMapping(this, ActionTag);
}

#if WITH_EDITOR
void UNeInputAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UNeInputAction, ActionTag))
	{
		UNeEnhancedPlayerInput::UpdateActionTagMapping(this, ActionTag);
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

