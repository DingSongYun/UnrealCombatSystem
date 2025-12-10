// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityActionViewer.h"

#include "GameplayEffect.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Beams/NeAbilityBeamLinkage.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "UObject/UObjectIterator.h"

FNeAbilityActionViewItem::FNeAbilityActionViewItem(const FAssetData& InAsset)
	: BlueprintAction(InAsset), ActionClass(nullptr)
{
}

FNeAbilityActionViewItem::FNeAbilityActionViewItem(UClass* InClass)
	: ActionClass(InClass)
{
}

bool FNeAbilityActionViewItem::operator==(const FNeAbilityActionViewItem& Other) const
{
	return GetClassPath() == Other.GetClassPath();
}

FText FNeAbilityActionViewItem::GetDisplayNameText() const
{
	if (ActionClass) return ActionClass->GetDisplayNameText();
	if (BlueprintAction.IsValid())
	{
		FString DisplayName;
		BlueprintAction.GetTagValue(FBlueprintTags::BlueprintDisplayName, DisplayName);
		return !DisplayName.IsEmpty() ? FText::FromString(DisplayName) : FText::FromName(BlueprintAction.AssetName);
	}

	return FText::GetEmpty();
}

FString FNeAbilityActionViewItem::GetClassName() const
{
	if (ActionClass) return ActionClass->GetName();

	if (BlueprintAction.IsValid())
	{
		return BlueprintAction.AssetClassPath.GetAssetName().ToString();
	}

	return TEXT("");
}

FText FNeAbilityActionViewItem::GetToolTips() const
{
	if (ActionClass) return ActionClass->GetToolTipText();

	if (BlueprintAction.IsValid())
	{
		if (UClass* BlueprintClass = BlueprintAction.GetClass())
		{
			if (BlueprintClass != UBlueprint::StaticClass())
			{
				return BlueprintClass->GetToolTipText();
			}
		}
	}

	return FText::GetEmpty();
}

FTopLevelAssetPath FNeAbilityActionViewItem::GetClassPath() const
{
	if (ActionClass) return ActionClass->GetClassPathName();

	return BlueprintAction.GetSoftObjectPath().GetAssetPath();
}

FString FNeAbilityActionViewItem::GetCategory() const
{
	FString Category;
	if (ActionClass)
	{
		Category = ActionClass->HasMetaData("Category") ? ActionClass->GetMetaData("Category") : "Common";
	}
	else
	{
		BlueprintAction.GetTagValue(FBlueprintTags::BlueprintCategory, Category);
	}
	return Category;
}

UClass* FNeAbilityActionViewItem::GetActionClass() const
{
	if (ActionClass) return ActionClass;

	const FString ClassPath = BlueprintAction.GetObjectPathString();
	const UBlueprint* Blueprint = LoadObject<UBlueprint>(NULL, *ClassPath);
	check(Blueprint);
	return Blueprint->GeneratedClass;
}

const TArray<UClass*>& FNeAbilityActionViewer::GetAbilityActionSupportTypes()
{
	struct FTypeCollection
	{
		TArray<UClass*> Types;
		FTypeCollection()
		{
			// Add base type
			Types.Add(UNeAbilityBeam::StaticClass());

			// Add linkage types
			Types.Append(UNeAbilityBeamLinkage::GetRegisteredLinkTypes());
		}
	};
	static FTypeCollection TypeCollection;
	return TypeCollection.Types;
}

TArray<FNeAbilityActionViewItem> FNeAbilityActionViewer::GetAllAbilityActions()
{
	TArray<FNeAbilityActionViewItem> OutActions;

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ ClassIt)
	{
		if (ClassIt->GetName().StartsWith(TEXT("SKEL_")) || ClassIt->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;;
		}

		if (ClassIt->HasAnyClassFlags(CLASS_Abstract | CLASS_HideDropDown | CLASS_Deprecated))
		{
			continue;
		}

		if (!IsChildOfActionType(*ClassIt))
		{
			continue;
		}

		OutActions.Add(*ClassIt);
	}

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> BlueprintList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());

	Filter.bRecursiveClasses = true;
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintList);
	for (int32 i = 0; i < BlueprintList.Num(); ++i)
	{
		FAssetData& AssetData = BlueprintList[i];
		if (ValidateAbilityBlueprintAction(AssetData))
		{
			OutActions.AddUnique(AssetData);
		}
	}

	return OutActions;
}

bool FNeAbilityActionViewer::IsChildOfActionType(const UClass* InClass)
{
	if (InClass == nullptr) return false;

	for (UClass* Type : GetAbilityActionSupportTypes())
	{
		if (InClass->IsChildOf(Type))
		{
			return true;
		}
	}

	return false;
}

bool FNeAbilityActionViewer::ValidateAbilityBlueprintAction(const FAssetData& ActionAsset)
{
	FString ParentClassPath;
	if (ActionAsset.GetTagValue(FBlueprintTags::NativeParentClassPath, ParentClassPath))
	{
		UClass* ParentClass = UClass::TryFindTypeSlow<UClass>(FPackageName::ExportTextPathToObjectPath(ParentClassPath));

		return IsChildOfActionType(ParentClass);
	}
	return false;
}
