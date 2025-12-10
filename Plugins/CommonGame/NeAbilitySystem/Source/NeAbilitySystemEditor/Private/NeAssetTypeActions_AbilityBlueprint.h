// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"

struct FAssetData;

class FNeAssetTypeActions_AbilityBlueprint: public FAssetTypeActions_Blueprint
{
public:
	//~BEGIN: IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_Ability Blueprint", "Ability Blueprint"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 96, 128); }
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Blueprint | EAssetTypeCategories::Gameplay;; }
	virtual bool CanLocalize() const override { return false; }
	virtual void PerformAssetDiff(UObject* Asset1, UObject* Asset2, const struct FRevisionInfo& OldRevision, const struct FRevisionInfo& NewRevision) const override;
	virtual FText GetAssetDescription( const FAssetData& AssetData ) const override;
	//~End IAssetTypeActions Implementation

	//~BEGIN: FAssetTypeActions_Blueprint Implementation
	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override;
	//~END: FAssetTypeActions_Blueprint Implementation

private:
	bool ShouldUseDataOnlyEditor(const UBlueprint* Blueprint) const;
};