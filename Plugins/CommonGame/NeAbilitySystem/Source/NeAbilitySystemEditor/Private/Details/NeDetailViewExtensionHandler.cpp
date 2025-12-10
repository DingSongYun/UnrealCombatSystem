// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeDetailViewExtensionHandler.h"

#include "DetailLayoutBuilder.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityPropertyBindingEditor.h"
#include "Beams/NeAbilityBeam.h"

FNeDetailViewExtensionHandler::FNeDetailViewExtensionHandler(TSharedPtr<FNeAbilityBlueprintEditor> InBlueprintEditor)
	: BlueprintEditor(InBlueprintEditor)
{}

bool FNeDetailViewExtensionHandler::IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& PropertyHandle) const
{
	if (PropertyHandle.GetNumOuterObjects() != 1)
	{
		return false;
	}

	TArray<UObject*> Objects;
	PropertyHandle.GetOuterObjects(Objects);

	// We don't allow bindings on the CDO.
	if (Objects[0] != nullptr && Objects[0]->HasAnyFlags(RF_ClassDefaultObject))
	{
		return false;
	}

	TSharedPtr<FNeAbilityBlueprintEditor> AbilityBlueprintEditor = BlueprintEditor.Pin();
	if (AbilityBlueprintEditor == nullptr)
	{
		return false;
	}

	const UNeAbilityBeam* Beam = Cast<UNeAbilityBeam>(Objects[0]);
	if (Beam == nullptr)
	{
		return false;
	}

	// 是否允许
	if (CanBindingPropery(InObjectClass, PropertyHandle))
	{
		return true;
	}

	return false;
}

bool FNeDetailViewExtensionHandler::CanBindingPropery(const UClass* InObjectClass, const IPropertyHandle& PropertyHandle) const
{
	if (PropertyHandle.HasMetaData(Name_PropBindingMeta))
	{
		if (PropertyHandle.GetBoolMetaData(Name_PropBindingMeta))
		{
			return true;
		}
	}
	return false;
}

void FNeDetailViewExtensionHandler::ExtendWidgetRow(FDetailWidgetRow& InWidgetRow, const IDetailLayoutBuilder& InDetailBuilder, const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle)
{
	TArray<UObject*> Objects;
	InPropertyHandle->GetOuterObjects(Objects);

	// Create Binding Widget
	if (Objects.Num() > 0)
	{
		if (UNeAbilityBeam* Beam = Cast<UNeAbilityBeam>(Objects[0]))
		{
			FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(BlueprintEditor.Pin()->GetEditingAbility(), Beam, InPropertyHandle->GetProperty()->GetFName());
			// 原始Value是否可编辑
			InWidgetRow.IsValueEnabled(TAttribute<bool>::Create(
				[=]() {
					return Binding == nullptr || Binding->Type != ENeAbilityPropertyBindingType::Curve;
			}));
			
			InWidgetRow.ExtensionContent()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					FNeAbilityPropertyBindingEditor::MakePropertyBindingWidget(BlueprintEditor.Pin(), Beam, InPropertyHandle.ToSharedRef())
				];

			if (Binding && Binding->Type == ENeAbilityPropertyBindingType::Curve)
			{
				if (Binding->CurveData.IsValid())
				{
					// FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
					// DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

					TSharedRef<FStructOnScope> ChildStructure = MakeShareable(new FStructOnScope(Binding->CurveData.StaticStruct(), (uint8*)&Binding->CurveData));
					IDetailLayoutBuilder* DetailLayoutBuilder = const_cast<IDetailLayoutBuilder*>(&InDetailBuilder);
					DetailLayoutBuilder->AddStructurePropertyData(ChildStructure, InPropertyHandle->GetProperty()->GetFName());
				}
			}
		}
	}
}
