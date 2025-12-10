// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilitySystemEditorModule.h"

#include "AssetToolsModule.h"
#include "ISettingsModule.h"
#include "NeAbility.h"
#include "NeAbilityBlueprintEditor.h"
#include "GameplayAbilityBlueprint.h"
#include "NeAbilitySystemSettings.h"
#include "NeAssetTypeActions_AbilityBlueprint.h"
#include "PropertyEditorModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CustomLayout/NeAbilityDataBoardCustomization.h"
#include "CustomLayout/NeAbilityDataBoardKeyCustomization.h"
#include "CustomLayout/NeAbilityFuncRefCustomization.h"
#include "CustomLayout/NeAbilitySegmentCustomization.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Thumbnails/NeAbilityThumbnailRenderer.h"

#define LOCTEXT_NAMESPACE "FNeAbilitySystemEditor"

INeAbiitySystemEditorModule& INeAbiitySystemEditorModule::Get()
{
	INeAbiitySystemEditorModule& Module = FModuleManager::LoadModuleChecked<INeAbiitySystemEditorModule>("NeAbilitySystemEditor");

	return Module;
}

class FNeAbilitySystemEditorModule : public INeAbiitySystemEditorModule
{
public:
	virtual void StartupModule() override
	{
		MenuExtensibilityManager = MakeShared<FExtensibilityManager>();
		ToolBarExtensibilityManager = MakeShared<FExtensibilityManager>();

		// Register asset types
		RegisterAssetTypes();

		// Register Custom layouts
		RegisterCustomLayouts();

		// Register Settings
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->RegisterSettings(
				"Project", "Plugins",
				"Ability System", LOCTEXT("NeAbilitySystem", "Ability System"),
				LOCTEXT("AbilitySystem_SettingDesc", "Settings for ability system"),
				GetMutableDefault<UNeAbilitySystemSettings>());
		}

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().OnInMemoryAssetCreated().AddRaw(this, &FNeAbilitySystemEditorModule::OnNewAsset);

		// Thumbnail
		UThumbnailManager::Get().UnregisterCustomRenderer(UGameplayAbilityBlueprint::StaticClass());
		UThumbnailManager::Get().RegisterCustomRenderer(UGameplayAbilityBlueprint::StaticClass(), UNeAbilityThumbnailRenderer::StaticClass());
	}

	virtual void ShutdownModule() override
	{
		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();

		// Unregister asset types
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");
		if (AssetToolsModule)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();
			for ( int32 Index = 0; Index < RegistedAssetTypeActions.Num(); ++Index )
			{
				AssetTools.UnregisterAssetTypeActions(RegistedAssetTypeActions[Index].ToSharedRef());
			}
		}
	}

	void RegisterAssetTypes()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FNeAssetTypeActions_AbilityBlueprint()));
	}

	void RegisterCustomLayouts() const
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout( FNeAbilitySegmentCustomization::GetTypeName(), FOnGetDetailCustomizationInstance::CreateStatic(&FNeAbilitySegmentCustomization::MakeInstance));
		// PropertyModule.RegisterCustomPropertyTypeLayout(FNeAbilityDataBoardCustomization::GetTypeName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNeAbilityDataBoardCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FNeAbilityDataBoardKeyCustomization::GetTypeName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNeAbilityDataBoardKeyCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FNeAbilityFuncRefCustomization::GetTypeName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNeAbilityFuncRefCustomization::MakeInstance));
		// PropertyModule.RegisterCustomPropertyTypeLayout(FNeAbilitySegmentCustomization::GetTypeName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNeAbilitySegmentCustomization::MakeInstance));
	}

	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override
	{
		return MenuExtensibilityManager;
	}

	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override
	{
		return ToolBarExtensibilityManager;
	}

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegistedAssetTypeActions.Add(Action);
	}

	void OnNewAsset(UObject* NewAsset) const
	{
		if (UGameplayAbilityBlueprint* AbilityBlueprint = Cast<UGameplayAbilityBlueprint>(NewAsset))
		{
			if ( UNeAbility* Ability = Cast<UNeAbility>(AbilityBlueprint->GeneratedClass.GetDefaultObject()) )
			{
	 			Ability->PostAssetCreate();
				// 修复这里设置的Ability属性在未保持情况下第一次运行不会复制给instance的情况
				if (UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(AbilityBlueprint->GeneratedClass))
				{
					BPGC->UpdateCustomPropertyListForPostConstruction();
				}
			}
		}
	}


private:
	TArray< TSharedPtr<IAssetTypeActions> > RegistedAssetTypeActions;
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNeAbilitySystemEditorModule, NeAbilitySystemEditor)
