 // Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityEditorApplicationMode.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityBlueprintEditorToolbar.h"
#include "NeAbilitySystemEditorModule.h"

FNeAbilityEditorApplicationMode::FNeAbilityEditorApplicationMode(TSharedPtr<FNeAbilityBlueprintEditor> InAbilityEditor, FName InModeName)
	: FBlueprintEditorApplicationMode(InAbilityEditor, InModeName, FNeAbilityBlueprintEditorModes::GetLocalizedMode, false, false)
	, MyBlueprintEditor(InAbilityEditor)
{
	ToolbarExtender = INeAbiitySystemEditorModule::Get().GetToolBarExtensibilityManager()->GetAllExtenders();
	InAbilityEditor->GetEditorToolbarBuilder()->AddEditorModesToolbar(ToolbarExtender);
}

void FNeAbilityEditorApplicationMode::PreDeactivateMode()
{
	FBlueprintEditorApplicationMode::PreDeactivateMode();
}

void FNeAbilityEditorApplicationMode::PostActivateMode()
{
	FBlueprintEditorApplicationMode::PostActivateMode();
}

UGameplayAbilityBlueprint* FNeAbilityEditorApplicationMode::GetBlueprint() const
{
	if ( FNeAbilityBlueprintEditor* Editor = MyBlueprintEditor.Pin().Get() )
	{
		return Editor->GetAbilityBlueprintObj();
	}

	return nullptr;
}
