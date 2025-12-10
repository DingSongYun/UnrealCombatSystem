// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "NeAbilityPropertyBinding.h"
#include "PropertyHandle.h"

class UNeAbility;
class UNeAbilityBeam;
class FNeAbilityBlueprintEditor;
struct FNeAbilityPropertyBinding;

class FNeAbilityPropertyBindingEditor
{
public:
	/** Create binding widgets */
	static TSharedRef<SWidget> MakePropertyBindingWidget(TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle);

	static FNeAbilityPropertyBinding* GetExistPropertyBinding(UNeAbility* InAbility, UNeAbilityBeam* InBeam, const FName& InPropertyName);

	static FNeAbilityPropertyBinding& AddNewPropertyBinding(UNeAbility* InAbility, UNeAbilityBeam* InBeam, const FName& InPropertyName, TSharedRef<IPropertyHandle> PropertyHandle, ENeAbilityPropertyBindingType BindingType = ENeAbilityPropertyBindingType::DataBoard);

	static void RemovePropertyBinding(UNeAbility* InAbility, UNeAbilityBeam* InBeam, const FName& InPropertyName);

	static void ChangePropertyBindingType(UNeAbility* InAbility, FNeAbilityPropertyBinding* InBinding, ENeAbilityPropertyBindingType NewBindingType);

	static void InitializeBindingCurveData(FNeAbilityPropertyBinding* InBinding, TSharedRef<IPropertyHandle> InPropertyHandle);
};
