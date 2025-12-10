// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityDataBoardCustomization.h"

#include "IDetailChildrenBuilder.h"
#include "IDetailCustomNodeBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "DataBoard/NeAbilityDataBoard.h"

class FDataBoardNodeBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FDataBoardNodeBuilder>
{
	IDetailChildrenBuilder* DetailLayoutBuilder;

	TSharedPtr<IPropertyHandle> AnimationMappingsPropertyHandle;
public:
	FDataBoardNodeBuilder(IDetailChildrenBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle)
	{
		// TSharedRef<SWidget> AddButton = PropertyCustomizationHelpers::MakeAddButton(
		// FSimpleDelegate::CreateSP(this, &FDataBoardNodeBuilder::AddAnimationMappingButton_OnClick),
		// TAttribute<FText>(this, &FDataBoardNodeBuilder::GetAddAnimationMappingTooltip),
		// TAttribute<bool>(this, &FDataBoardNodeBuilder::CanAddNewSlotMapping));
		//
		// TSharedRef<SWidget> ClearButton = PropertyCustomizationHelpers::MakeEmptyButton(
		// 	FSimpleDelegate::CreateSP(this, &FDataBoardNodeBuilder::ClearAnimationMappingButton_OnClick),
		// 	LOCTEXT("ClearAnimationMappingToolTip", "Removes all AnimationMappings"), true);
	}

	void OnClick_AddDataBoardEntry()
	{
		
	}
};

FName FNeAbilityDataBoardCustomization::GetTypeName()
{
	return FNeAbilityDataBoard::StaticStruct()->GetFName();
}

void FNeAbilityDataBoardCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

void FNeAbilityDataBoardCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// const TSharedRef<FDataBoardNodeBuilder> ActionMappingsBuilder = MakeShareable(new FDataBoardNodeBuilder(&StructBuilder, StructPropertyHandle));
	// StructBuilder.AddCustomBuilder(ActionMappingsBuilder);
}
