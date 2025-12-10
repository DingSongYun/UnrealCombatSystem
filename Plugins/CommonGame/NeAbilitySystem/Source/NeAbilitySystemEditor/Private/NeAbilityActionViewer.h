// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"

class FNeAbilityActionViewItem
{
	FAssetData BlueprintAction;
	UClass* ActionClass = nullptr;
public:
	FNeAbilityActionViewItem() {}
	FNeAbilityActionViewItem(const FAssetData& InAsset);
	FNeAbilityActionViewItem(UClass* InClass);

	bool operator==(const FNeAbilityActionViewItem&) const;

	FText GetDisplayNameText() const;

	FString GetClassName() const;

	FText GetToolTips() const;

	FTopLevelAssetPath GetClassPath() const;

	FString GetCategory() const;

	UClass* GetActionClass() const;
};

class FNeAbilityActionViewer
{
public:
	/**
	 * 获取Segment支持的所有Action类型，
	 * 包括几种:
	 *	1. 直接使用AbilityBeam
	 *	2. Linkage类型的Beam支持的类型
	 *	3. 待增加
	 */
	static const TArray<UClass*>& GetAbilityActionSupportTypes();

	/**
	 * 获取所有技能可用的Action
	 */
	static TArray<FNeAbilityActionViewItem> GetAllAbilityActions();

	static bool IsChildOfActionType(const UClass* InClass);

private:
	static bool ValidateAbilityBlueprintAction(const FAssetData& ActionAsset);
};
