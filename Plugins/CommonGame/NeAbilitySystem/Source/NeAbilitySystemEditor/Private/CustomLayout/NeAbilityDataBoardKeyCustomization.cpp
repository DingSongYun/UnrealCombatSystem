// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityDataBoardKeyCustomization.h"

#include "DetailWidgetRow.h"
#include "NiagaraCommon.h"
#include "PropertyHandle.h"
#include "SGameplayTagCombo.h"
#include "DataBoard/NeAbilityDataBoard.h"

FName FNeAbilityDataBoardKeyCustomization::GetTypeName()
{
	return  FNeAbilityDataBoardKey::StaticStruct()->GetFName();
}

void FNeAbilityDataBoardKeyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> EntryNamePropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FNeAbilityDataBoardKey, EntryName));
	HeaderRow
	.NameContent()
	[
		InStructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SGameplayTagCombo)
			.PropertyHandle(EntryNamePropertyHandle)
			// .Filter(DATABOARD_ENTRY_CATEGORY)
			// .OnTagChanged_Lambda([Binding](const FGameplayTag InTag)
			// {
			// 	Binding->DataBoardEntry = InTag;
			// 	FSlateApplication::Get().DismissAllMenus();
			// }),
		// EntryNamePropertyHandle->CreatePropertyValueWidget()
	]
	;
}

void FNeAbilityDataBoardKeyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}
