// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "Styling/ISlateStyle.h"

class FNeAbilityEditorCommands : public TCommands<FNeAbilityEditorCommands>
{
public:
	FNeAbilityEditorCommands();
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> Reset;
	TSharedPtr<FUICommandInfo> Step;
	TSharedPtr<FUICommandInfo> ToggleCollision;
};

