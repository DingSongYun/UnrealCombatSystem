// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailPropertyExtensionHandler.h"

class FNeAbilityBlueprintEditor;

/**
 * FNeDetailViewExtensionHandler
 * 扩展详细面板，主要用来做属性绑定
 */
class FNeDetailViewExtensionHandler : public IDetailPropertyExtensionHandler
{
public:
	FNeDetailViewExtensionHandler(TSharedPtr<FNeAbilityBlueprintEditor> InBlueprintEditor);

	virtual bool IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& PropertyHandle) const override;

	virtual void ExtendWidgetRow(FDetailWidgetRow& InWidgetRow, const IDetailLayoutBuilder& InDetailBuilder, const UClass* InObjectClass, TSharedPtr<IPropertyHandle> PropertyHandle) override;

protected:
	bool CanBindingPropery(const UClass* InObjectClass, const IPropertyHandle& PropertyHandle) const;

private:
	TWeakPtr<FNeAbilityBlueprintEditor> BlueprintEditor;
};