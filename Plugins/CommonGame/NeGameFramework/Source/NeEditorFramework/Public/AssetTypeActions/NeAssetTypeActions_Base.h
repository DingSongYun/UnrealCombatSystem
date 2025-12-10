// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

/**
 * FNeAssetTypeActions_Base
 */
class NEEDITORFRAMEWORK_API FNeAssetTypeActions_Base : public FAssetTypeActions_Base
{
public:
	FNeAssetTypeActions_Base(EAssetTypeCategories::Type InAssetCategory, UClass* InSupportClass, FColor InAssetColor) :
		AssetCategory(InAssetCategory), SupportClass(InSupportClass),AssetColor(InAssetColor)
	{}

	// ~Begin: IAssetTypeActions interface
	virtual uint32 GetCategories() override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	// ~End: IAssetTypeActions interface

protected:
	EAssetTypeCategories::Type AssetCategory;
	UClass* SupportClass;
	FColor AssetColor;
};