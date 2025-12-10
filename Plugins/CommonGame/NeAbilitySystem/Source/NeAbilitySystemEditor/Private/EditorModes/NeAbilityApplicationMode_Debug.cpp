 // Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityApplicationMode_Debug.h"
#include "NeAbilityBlueprintEditor.h"

#define LOCTEXT_NAMESPACE "AbilityDebugMode"

FAbilityApplicationMode_Debug::FAbilityApplicationMode_Debug(TSharedPtr<FNeAbilityBlueprintEditor> InAbilityEditor)
	: FNeAbilityEditorApplicationMode(InAbilityEditor, FNeAbilityBlueprintEditorModes::DebugMode)
{
}

void FAbilityApplicationMode_Debug::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FNeAbilityBlueprintEditor> BlueprintEditorPtr = GetBlueprintEditor();

	BlueprintEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());
	BlueprintEditorPtr->PushTabFactories(TabFactories);

	FNeAbilityEditorApplicationMode::RegisterTabFactories(InTabManager);
}

#undef LOCTEXT_NAMESPACE
