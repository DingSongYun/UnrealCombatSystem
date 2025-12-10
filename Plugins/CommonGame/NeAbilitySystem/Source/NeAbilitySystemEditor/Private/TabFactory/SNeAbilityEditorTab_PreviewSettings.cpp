// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityEditorTab_PreviewSettings.h"
#include "AdvancedPreviewSceneModule.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityPreviewSettings.h"
#include "NeAbilityPreviewScene.h"
#include "Modules/ModuleManager.h"

void SNeAbilityEditorTab_PreviewSettings::Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityBlueprintEditor>& InAssetEditorToolkit)
{
	TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo> DetailsCustomizations;
	TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo> PropertyTypeCustomizations;

	FAdvancedPreviewSceneModule& AdvancedPreviewSceneModule = FModuleManager::LoadModuleChecked<FAdvancedPreviewSceneModule>("AdvancedPreviewScene");
	ChildSlot
	[
		AdvancedPreviewSceneModule.CreateAdvancedPreviewSceneSettingsWidget(
			InAssetEditorToolkit->GetAbilityPreviewScene().ToSharedRef(),
			InAssetEditorToolkit->GetPreviewSettings(),
			DetailsCustomizations, PropertyTypeCustomizations)
	];
}
