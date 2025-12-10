// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "AssetTypeActions/NeAssetTypeActions_Base.h"

uint32 FNeAssetTypeActions_Base::GetCategories()
{
	return AssetCategory;
}

bool FNeAssetTypeActions_Base::HasActions(const TArray<UObject*>& InObjects) const
{
	return false;
}

UClass* FNeAssetTypeActions_Base::GetSupportedClass() const
{
	return SupportClass;
}

FColor FNeAssetTypeActions_Base::GetTypeColor() const
{
	return AssetColor;
}