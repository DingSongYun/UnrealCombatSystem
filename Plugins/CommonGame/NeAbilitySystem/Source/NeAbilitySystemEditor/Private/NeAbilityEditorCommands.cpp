// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityEditorCommands.h"

#define LOCTEXT_NAMESPACE "NeAbilityEditorCommands"

FNeAbilityEditorCommands::FNeAbilityEditorCommands() : TCommands<FNeAbilityEditorCommands> (
	TEXT("NeAbilityEditor"),
	NSLOCTEXT("Contexts", "AbilityEditor", "Ability Editor"),
	NAME_None,
	TEXT("Waiting")
	)
{}

void FNeAbilityEditorCommands::RegisterCommands()
{
	UI_COMMAND(Reset, "Reset World", "Resets the preview World, it Will Stop Playing Ability.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt | EModifierKey::Shift, EKeys::R));
	UI_COMMAND(Step, "Step Preview", "Step preview.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt | EModifierKey::Shift, EKeys::S));
	UI_COMMAND(ToggleCollision, "Toggle Collision", "Toggle show Collision.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt, EKeys::C));
}

#undef LOCTEXT_NAMESPACE