// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"

class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IPropertyHandle;
class IDetailCategoryBuilder;

class FNeAbilityFuncRefCustomization : public IPropertyTypeCustomization
{
public:
	static FName GetTypeName();
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FNeAbilityFuncRefCustomization());
	}

	//~BEGIN: IPropertyTypeCustomization interface
	virtual void CustomizeHeader( TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;
	virtual void CustomizeChildren( TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;
	//~END: IPropertyTypeCustomization interface

protected:
	TSharedRef<class SWidget> MakeFunctionRefWidget(TSharedRef<IPropertyHandle> InFuncRefPropertyHandle);
	TSharedRef<class SWidget> MakeFunctionRefWidget2(TSharedRef<IPropertyHandle> InStructPropertyHandle, TSharedRef<IPropertyHandle> InFuncRefPropertyHandle);
	void OnSelectedFunction(const FName& InFuncName, const UFunction* InFunction, TSharedRef<IPropertyHandle> InStructPropertyHandle);
	TOptional<FName> GetCurrentFunctionName(TSharedRef<IPropertyHandle> InStructPropertyHandle) const;
};