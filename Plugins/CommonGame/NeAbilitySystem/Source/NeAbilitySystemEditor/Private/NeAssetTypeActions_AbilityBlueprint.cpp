// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAssetTypeActions_AbilityBlueprint.h"

#include "GameplayAbilitiesEditor.h"
#include "GameplayAbilityBlueprint.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityBlueprintFactory.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FNeAssetTypeActions_AbilityBlueprint::GetSupportedClass() const
{
	return UGameplayAbilityBlueprint::StaticClass();
}

void FNeAssetTypeActions_AbilityBlueprint::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Blueprint = Cast<UBlueprint>(*ObjIt);
		if (Blueprint && Blueprint->SkeletonGeneratedClass && Blueprint->GeneratedClass )
		{
			TSharedRef< FNeAbilityBlueprintEditor > NewBlueprintEditor(new FNeAbilityBlueprintEditor());

			TArray<UBlueprint*> Blueprints;
			Blueprints.Add(Blueprint);
			NewBlueprintEditor->InitAbilityBlueprintEditor(Mode, EditWithinLevelEditor, Blueprints, /*bShouldOpenInDefaultsMode = */ false);
		}
		else
		{
			FMessageDialog::Open( EAppMsgType::Ok, LOCTEXT("FailedToLoadAbilityBlueprint", "Ability Blueprint could not be loaded because it derives from an invalid class.\nCheck to make sure the parent class for this blueprint is valid!"));
		}
	}
	// FAssetTypeActions_Blueprint::OpenAssetEditor(InObjects, EditWithinLevelEditor);
}

void FNeAssetTypeActions_AbilityBlueprint::PerformAssetDiff(UObject* Asset1, UObject* Asset2, const FRevisionInfo& OldRevision, const FRevisionInfo& NewRevision) const
{
	FAssetTypeActions_Blueprint::PerformAssetDiff(Asset1, Asset2, OldRevision, NewRevision);
}

FText FNeAssetTypeActions_AbilityBlueprint::GetAssetDescription(const FAssetData& AssetData) const
{
	return FAssetTypeActions_Blueprint::GetAssetDescription(AssetData);
}

UFactory* FNeAssetTypeActions_AbilityBlueprint::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UNeAbilityBlueprintFactory* GameplayAbilitiesBlueprintFactory = NewObject<UNeAbilityBlueprintFactory>();
	GameplayAbilitiesBlueprintFactory->SetParent(TSubclassOf<UGameplayAbility>(*InBlueprint->GeneratedClass));
	return GameplayAbilitiesBlueprintFactory;
}

bool FNeAssetTypeActions_AbilityBlueprint::ShouldUseDataOnlyEditor(const UBlueprint* Blueprint) const
{
	return FBlueprintEditorUtils::IsDataOnlyBlueprint(Blueprint)
		&& !FBlueprintEditorUtils::IsLevelScriptBlueprint(Blueprint)
		&& !FBlueprintEditorUtils::IsInterfaceBlueprint(Blueprint)
		&& !Blueprint->bForceFullEditor
		&& !Blueprint->bIsNewlyCreated;
}

#undef LOCTEXT_NAMESPACE