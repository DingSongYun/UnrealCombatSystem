// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilitySegmentCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "NeAbilitySegment.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Beams/NeAbilityBeam_GameplayTask.h"
#include "Timeline/NeAbilitySegmentEditorObject.h"

FName FNeAbilitySegmentCustomization::GetTypeName()
{
	return UNeAbilitySegmentEditorObject::StaticClass()->GetFName();
}

void FNeAbilitySegmentCustomization::CustomizeDetails( IDetailLayoutBuilder& DetailBuilder )
{
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
	TArray<UClass*> NotifyClasses;
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
	check(SelectedObjects.Num() > 0);
	UNeAbilitySegmentEditorObject* EditorObject = Cast<UNeAbilitySegmentEditorObject>(SelectedObjects[0].Get());

	IDetailCategoryBuilder& SegmentCategory = DetailBuilder.EditCategory(TEXT("Segment"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);

	TSharedPtr<FStructOnScope> SegmentStructData = MakeShareable( new FStructOnScope(FNeAbilitySegment::StaticStruct() ,reinterpret_cast<uint8*>(&EditorObject->SegmentPtr.Get())) );

	// Customize Segment
	TSharedPtr<IPropertyHandle> BeamPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FNeAbilitySegment, Beam), FNeAbilitySegment::StaticStruct());
	for (TFieldIterator<FProperty> Property(FNeAbilitySegment::StaticStruct(), EFieldIteratorFlags::ExcludeSuper, EFieldIteratorFlags::ExcludeDeprecated); Property; ++Property)
	{
		IDetailPropertyRow* Row = SegmentCategory.AddExternalStructureProperty(SegmentStructData, Property->GetFName(), Property->HasAnyPropertyFlags(CPF_AdvancedDisplay) ? EPropertyLocation::Advanced : EPropertyLocation::Default);
		/**
		 * 对于Beam属性，我们在下面单独定制显示，这里不显示
		 * 至于为什么这里依然添加该属性，是想要拿到其对应的PropertyHandle，方便后面逻辑使用
		 * 不拿也是可以的，后面的逻辑麻烦点
		 */
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(FNeAbilitySegment, Beam))
		{
			BeamPropertyHandle = Row->GetPropertyHandle();
			Row->Visibility(EVisibility::Hidden);
		}
	}

	// Customize Beam
	UNeAbilityBeam* Beam = EditorObject->SegmentPtr->Beam;
	if (Beam && BeamPropertyHandle.IsValid())
	{
		TSharedPtr<IPropertyHandle> BeamObjectHandler = BeamPropertyHandle->GetChildHandle(0);
		IDetailCategoryBuilder& BeamCategory = DetailBuilder.EditCategory(TEXT("Beam"), FText::GetEmpty(), ECategoryPriority::Variable);
		uint32 NumChildren = 0;
		BeamObjectHandler->GetNumChildren(NumChildren);
		TMap<FName, IDetailGroup*> CachedGroups;
		for(uint32 ChildIdx = 0 ; ChildIdx < NumChildren ; ++ChildIdx)
		{
			TSharedPtr<IPropertyHandle> BeamProp = BeamObjectHandler->GetChildHandle(ChildIdx);
			FProperty* Prop = BeamProp->GetProperty();
			if(Prop && !Prop->HasAnyPropertyFlags(CPF_DisableEditOnInstance))
			{
				FName DefaultCategoryName = BeamProp->GetDefaultCategoryName();
				TArray<FString> CategoryArray;
				FString Category, SubCategory;
				DefaultCategoryName.ToString().ParseIntoArray(CategoryArray, TEXT("|"), true);
				if (CategoryArray.Num() <= 0) continue;
				Category = CategoryArray[0].TrimChar(' ');
				SubCategory = CategoryArray.Num() >= 2 ? CategoryArray[1].TrimChar(' ') : "";
				IDetailCategoryBuilder& PropertyCategory = Category.IsEmpty() ? BeamCategory :
					DetailBuilder.EditCategory(*Category, FText::FromString(Category), ECategoryPriority::Variable);
				if(!CustomizeProperty(PropertyCategory, Beam, BeamProp))
				{
					if (SubCategory.IsEmpty())
					{
						PropertyCategory.AddProperty(BeamProp, EPropertyLocation::Default);
					}
					else
					{
						IDetailGroup* Group = nullptr;
						if (CachedGroups.Contains(DefaultCategoryName))
						{
							Group = CachedGroups[DefaultCategoryName];
						}
						else
						{
							Group = &PropertyCategory.AddGroup(*SubCategory, FText::FromString(SubCategory), false, true);
							CachedGroups.Add(DefaultCategoryName, Group);
						}
						Group->AddPropertyRow(BeamProp.ToSharedRef());
					}
				}
			}
		}
	}
}

bool FNeAbilitySegmentCustomization::CustomizeProperty(IDetailCategoryBuilder& CategoryBuilder, UNeAbilityBeam* Beam, TSharedPtr<IPropertyHandle> PropertyHandle)
{
	const FProperty* Property = PropertyHandle->GetProperty();
	const FName PropertyName = Property->GetFName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UNeAbilityBeam_GameplayTask, TaskTemplate))
	{
		UNeAbilityBeam_GameplayTask* TaskBeam = Cast<UNeAbilityBeam_GameplayTask>(Beam);
		if (TaskBeam && IsValid(TaskBeam->TaskTemplate))
		{
			return CustomizeGameplayTaskTemplate(CategoryBuilder, TaskBeam->TaskTemplate, PropertyHandle);
		}
	}
	return false;
}

bool FNeAbilitySegmentCustomization::CustomizeGameplayTaskTemplate(IDetailCategoryBuilder& CategoryBuilder, UAbilityTask* TaskTemplate, TSharedPtr<IPropertyHandle> PropertyHandle)
{
	IDetailCategoryBuilder& TaskCategory = CategoryBuilder.GetParentLayout().EditCategory(TEXT("Task Data"), FText::GetEmpty(), ECategoryPriority::Important);
	const TArray<UObject*>& Objects {TaskTemplate};
	FAddPropertyParams AddPropertyParams;
	AddPropertyParams.AllowChildren(true);
	AddPropertyParams.HideRootObjectNode(true);
	AddPropertyParams.CreateCategoryNodes(true);
	TaskCategory.AddExternalObjects(Objects, EPropertyLocation::Default, AddPropertyParams);

	// for (TFieldIterator<FProperty> Property(TaskTemplate->GetClass(), EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::ExcludeDeprecated); Property; ++Property)
	// {
	// 	if (!Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance) && Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
	// 	{
	// 		IDetailPropertyRow* Row = TaskCategory.AddExternalObjectProperty(Objects, Property->GetFName(), Property->HasAnyPropertyFlags(CPF_AdvancedDisplay) ? EPropertyLocation::Advanced : EPropertyLocation::Common);
	// 	}
	// }

	return true;
}
