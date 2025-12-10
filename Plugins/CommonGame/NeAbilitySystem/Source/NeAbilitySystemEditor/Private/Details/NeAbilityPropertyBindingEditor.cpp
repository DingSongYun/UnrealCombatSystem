// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Details/NeAbilityPropertyBindingEditor.h"
#include "IPropertyAccessEditor.h"
#include "NeAbility.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityPropertyBinding.h"
#include "NeGameplayAbilityLibrary.h"
#include "Beams/NeAbilityBeam.h"
#include "EdGraph/EdGraph.h"
#include "Features/IModularFeatures.h"
#include "SGameplayTagCombo.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "AbilityPropertyBinding"

const FString DATABOARD_ENTRY_CATEGORY = TEXT("GameplayAbility.Data");

void AddBindingMenuExtension(FMenuBuilder& InMenuBuilder, TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	InMenuBuilder.BeginSection("Binding Type", LOCTEXT("Binding Type", "Binding Type"));
	const FName PropertyName = InPropertyHandle->GetProperty()->GetFName();

	auto GetBindingTypeCheckState = [=] (ENeAbilityPropertyBindingType BindingType)
	{
		UNeAbility* EditingAbility = InAbilityEditor.Pin()->GetEditingAbility();
		FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(EditingAbility, InBeam, PropertyName);
		if (Binding)
		{
			return Binding->Type == BindingType ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		}
		return ECheckBoxState::Unchecked;
	};

	auto SetBindingType = [=] (ENeAbilityPropertyBindingType BindingType)
	{
		InAbilityEditor.Pin()->GetBlueprintObj()->Modify();
		UNeAbility* EditingAbility = InAbilityEditor.Pin()->GetEditingAbility();
		FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(EditingAbility, InBeam, PropertyName);
		if (Binding)
		{
			Binding->Type = BindingType;
		}
		else
		{
			Binding = &FNeAbilityPropertyBindingEditor::AddNewPropertyBinding(EditingAbility, InBeam, PropertyName, InPropertyHandle, BindingType);
		}

		if (Binding->Type == ENeAbilityPropertyBindingType::Curve)
		{
			FNeAbilityPropertyBindingEditor::InitializeBindingCurveData(Binding, InPropertyHandle);
		}
	};

	InMenuBuilder.AddMenuEntry(
		LOCTEXT("BindingType_DataBoard", "Data Board"),
		LOCTEXT("BindingType_DataBoard_Tooltip", "Binding a data board to the property."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Blackboard.NewEntry"),
		FUIAction(
			FExecuteAction::CreateLambda(SetBindingType, ENeAbilityPropertyBindingType::DataBoard),
			FCanExecuteAction(),
			FGetActionCheckState::CreateLambda(GetBindingTypeCheckState, ENeAbilityPropertyBindingType::DataBoard)
		),
		NAME_None,
		EUserInterfaceActionType::Check
	);

	InMenuBuilder.AddMenuEntry(
		LOCTEXT("BindingType_Curve", "Curve"),
		LOCTEXT("BindingType_Curve_Tooltip", "Binding a curve to the property."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCurveEditor.TabIcon"),
		FUIAction(
			FExecuteAction::CreateLambda(SetBindingType, ENeAbilityPropertyBindingType::Curve),
			FCanExecuteAction(),
			FGetActionCheckState::CreateLambda(GetBindingTypeCheckState, ENeAbilityPropertyBindingType::Curve)
		),
		NAME_None,
		EUserInterfaceActionType::Check
	);

	InMenuBuilder.AddMenuEntry(
		LOCTEXT("BindingType_Function", "Function"),
		LOCTEXT("BindingType_Function_Tooltip", "Binding a function to the property."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.K2Node_FunctionEntry"),
		FUIAction(
			FExecuteAction::CreateLambda(SetBindingType, ENeAbilityPropertyBindingType::Function),
			FCanExecuteAction(),
			FGetActionCheckState::CreateLambda(GetBindingTypeCheckState, ENeAbilityPropertyBindingType::Function)
		),
		NAME_None,
		EUserInterfaceActionType::Check
	);

	InMenuBuilder.AddMenuEntry(
		LOCTEXT("BindingType_Property", "Property"),
		LOCTEXT("BindingType_Property_Tooltip", "Binding a ability blueprint property to the property."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.K2Node_Variable"),
		FUIAction(
			FExecuteAction::CreateLambda(SetBindingType, ENeAbilityPropertyBindingType::Property),
			FCanExecuteAction(),
			FGetActionCheckState::CreateLambda(GetBindingTypeCheckState, ENeAbilityPropertyBindingType::Property)
		),
		NAME_None,
		EUserInterfaceActionType::Check
	);

	InMenuBuilder.EndSection();

	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(InAbilityEditor.Pin()->GetEditingAbility(), InBeam, PropertyName);
	if (Binding && Binding->Type == ENeAbilityPropertyBindingType::DataBoard)
	{
		InMenuBuilder.BeginSection("Binding Data Board", LOCTEXT("Binding Type Data Board", "Data Board"));
		InMenuBuilder.AddWidget(
			SNew(SGameplayTagCombo)
				.Tag(Binding->DataBoardEntry)
				.Filter(DATABOARD_ENTRY_CATEGORY)
				.OnTagChanged_Lambda([Binding](const FGameplayTag InTag)
				{
					Binding->DataBoardEntry = InTag;
					FSlateApplication::Get().DismissAllMenus();
				}),
			LOCTEXT("DataBoard_Entry", "Entry"));
		InMenuBuilder.EndSection();
	}
	else if (Binding && Binding->Type == ENeAbilityPropertyBindingType::Curve)
	{
		// check(Binding->CurveData.IsValid());
		InMenuBuilder.BeginSection("Curve Data", LOCTEXT("Binding Type Curve", "Curve Data"));
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		{
			DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
			DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
			DetailsViewArgs.bHideSelectionTip = true;
			DetailsViewArgs.bAllowSearch = false;
			DetailsViewArgs.bShowPropertyMatrixButton = false;
			DetailsViewArgs.bShowModifiedPropertiesOption = false;
			DetailsViewArgs.bShowKeyablePropertiesOption = false;
			DetailsViewArgs.bShowAnimatedPropertiesOption = false;
		}

		FStructureDetailsViewArgs StructureDetailsViewArgs;
		{
			StructureDetailsViewArgs.bShowObjects = true;
			StructureDetailsViewArgs.bShowAssets = true;
			StructureDetailsViewArgs.bShowClasses = true;
			StructureDetailsViewArgs.bShowInterfaces = true;
		}
		
		// TSharedRef<FStructOnScope> CurveDataStruct = MakeShareable(new FStructOnScope(Binding->CurveData.Curve.GetScriptStruct(), Binding->CurveData.Curve.GetMutableMemory()));
		TSharedRef<FStructOnScope> CurveDataStruct = MakeShareable(new FStructOnScope(Binding->CurveData.StaticStruct(), (uint8*)(&Binding->CurveData)));
		TSharedRef<IStructureDetailsView> DetailsView = PropertyEditorModule.CreateStructureDetailView(DetailsViewArgs, StructureDetailsViewArgs, CurveDataStruct);

		InMenuBuilder.AddWidget(
			DetailsView->GetWidget().ToSharedRef(),
		LOCTEXT("CurveData", ""));
		
		InMenuBuilder.EndSection();
	}
}

bool OnGotoBinding(FName InPropertyName, TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	UNeAbility* EditingAbility = InAbilityEditor.Pin()->GetEditingAbility();
	UBlueprint* BlueprintObj = InAbilityEditor.Pin()->GetBlueprintObj();
	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(EditingAbility, InBeam, InPropertyHandle->GetProperty()->GetFName());
	if (Binding)
	{
		if ( Binding->Type == ENeAbilityPropertyBindingType::Function )
		{
			TArray<UEdGraph*> AllGraphs;
			BlueprintObj->GetAllGraphs(AllGraphs);

			FGuid SearchForGuid = Binding->MemberGuid;

			for ( UEdGraph* Graph : AllGraphs )
			{
				if ( Graph->GraphGuid == SearchForGuid )
				{
					InAbilityEditor.Pin()->SetCurrentMode(FNeAbilityBlueprintEditorModes::GraphMode);
					InAbilityEditor.Pin()->OpenDocument(Graph, FDocumentTracker::OpenNewDocument);
				}
			}

			// Either way return
			return true;
		}
	}

	return false;
}

bool OnCanGotoBinding(FName InPropertyName, TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	UNeAbility* EditingAbility = InAbilityEditor.Pin()->GetEditingAbility();
	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(EditingAbility, InBeam, InPropertyHandle->GetProperty()->GetFName());
	if (Binding && Binding->Type == ENeAbilityPropertyBindingType::Function)
	{
		return true;
	}

	return false;
}

bool OnCanBindToClass(UClass* Class)
{
	return Class && (Class->IsChildOf<UNeGameplayAbilityLibrary>() || Class == UNeAbility::StaticClass() || Class->IsChildOf<UNeAbility>());
}

void OnAddBinding(FName InPropertyName, const TArray<FBindingChainElement>& InBindingChain, TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	TArray<FFieldVariant> FieldChain;
	Algo::Transform(InBindingChain, FieldChain, [](const FBindingChainElement& InElement)
	{
		return InElement.Field;
	});

	const FFieldVariant& LeafField = InBindingChain.Last().Field;

	InAbilityEditor.Pin()->GetBlueprintObj()->Modify();
	UNeAbility* EditingAbility = InAbilityEditor.Pin()->GetEditingAbility();
	const FName PropertyName = InPropertyHandle->GetProperty()->GetFName();
	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(EditingAbility, InBeam, PropertyName);
	if (Binding == nullptr)
	{
		Binding = &FNeAbilityPropertyBindingEditor::AddNewPropertyBinding(EditingAbility, InBeam, PropertyName, InPropertyHandle);
	}
	Binding->Type = LeafField.IsA<UFunction>() ? ENeAbilityPropertyBindingType::Function : ENeAbilityPropertyBindingType::Property;

	if (Binding->Type == ENeAbilityPropertyBindingType::Function)
	{
		const UFunction* Function = FieldChain.Last().Get<UFunction>();
		Binding->FunctionName = Function->GetFName();
		UBlueprint::GetGuidFromClassByFieldName<UFunction>(
					Function->GetOwnerClass(),
					Function->GetFName(),
					Binding->MemberGuid);
	}
	else if (Binding->Type == ENeAbilityPropertyBindingType::Property)
	{
		const FProperty* Property = FieldChain.Last().Get<FProperty>();
		TArray<FString> PropertyChain;
		const IPropertyAccessEditor& PropertyAccessEditor = IModularFeatures::Get().GetModularFeature<IPropertyAccessEditor>("PropertyAccessEditor");
		PropertyAccessEditor.MakeStringPath(InBindingChain, PropertyChain);
		Binding->SourcePropertyPath = PropertyChain;
		UBlueprint::GetGuidFromClassByFieldName<UFunction>(
			InAbilityEditor.Pin()->GetBlueprintObj()->SkeletonGeneratedClass,
			Property->GetFName(),
			Binding->MemberGuid);
	}
}

void OnRemoveBinding(FName InPropertyName, TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	InAbilityEditor.Pin()->GetBlueprintObj()->Modify();
	FNeAbilityPropertyBindingEditor::RemovePropertyBinding(InAbilityEditor.Pin()->GetEditingAbility(), InBeam, InPropertyHandle->GetProperty()->GetFName());
}

bool OnCanRemoveBinding(FName InPropertyName, TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(InAbilityEditor.Pin()->GetEditingAbility(), InBeam, InPropertyHandle->GetProperty()->GetFName());
	if (Binding)
	{
		return true;
	}
	return false;
}

FText GetBindingText(TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(InAbilityEditor.Pin()->GetEditingAbility(), InBeam, InPropertyHandle->GetProperty()->GetFName());
	if (Binding)
	{
		if (Binding->Type == ENeAbilityPropertyBindingType::DataBoard)
		{
			if (Binding->DataBoardEntry.IsValid())
			{
				FString BindingText = Binding->DataBoardEntry.ToString();
				FString LeftString, RightString;
				if (BindingText.Split(DATABOARD_ENTRY_CATEGORY + ".", &LeftString, &RightString))
				{
					BindingText = RightString;
				}
				return FText::FromString(BindingText);
			}
			else
			{
				return LOCTEXT("Bind_No_DataBoardEntry", "No Entry");
			}
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Curve)
		{
			return LOCTEXT("Bind_Curve", "Curve");
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Function)
		{
			return FText::FromName(Binding->FunctionName);
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Property)
		{
			if (Binding->SourcePropertyPath.IsValid())
			{
				if (!Binding->SourcePropertyPath.IsResolved())
				{
					Binding->SourcePropertyPath.Resolve(InAbilityEditor.Pin()->GetEditingAbility());
				}
				if (const FProperty* Property = Binding->SourcePropertyPath.GetFProperty())
				{
					return Property->GetDisplayNameText();
				}
			}

			return FText::FromName("No Property");
		}
	}

	return LOCTEXT("Bind", "Bind");
}

const FSlateBrush* GetBindingImage(TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	static FName PropertyIcon(TEXT("Kismet.Tabs.Variables"));
	static FName FunctionIcon(TEXT("GraphEditor.Function_16x"));
	static FName DataBoardIcon(TEXT("BTEditor.Blackboard.NewEntry"));
	static FName CurveIcon(TEXT("GenericCurveEditor.TabIcon"));

	const FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(InAbilityEditor.Pin()->GetEditingAbility(), InBeam, InPropertyHandle->GetProperty()->GetFName());
	if (Binding)
	{
		if (Binding->Type == ENeAbilityPropertyBindingType::DataBoard)
		{
			return FAppStyle::Get().GetBrush(DataBoardIcon);
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Curve)
		{
			return FAppStyle::Get().GetBrush(CurveIcon);
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Function)
		{
			return FAppStyle::Get().GetBrush(FunctionIcon);
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Property)
		{
			return FAppStyle::Get().GetBrush(PropertyIcon);
		}
	}
	return nullptr;
}

FText GetBindingTooltips(TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	FText Tooltips;
	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(InAbilityEditor.Pin()->GetEditingAbility(), InBeam, InPropertyHandle->GetProperty()->GetFName());
	if (Binding)
	{
		if (Binding->Type == ENeAbilityPropertyBindingType::DataBoard)
		{
			if (Binding->DataBoardEntry.IsValid())
			{
				Tooltips = FText::FromString(Binding->DataBoardEntry.ToString());
			}
			else
			{
				Tooltips = LOCTEXT("Bind_Invalid_DataBoardEntry", "You should choose a data board entry to bind first.");
			}
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Property)
		{
			if (Binding->SourcePropertyPath.IsValid())
			{
				Tooltips = FText::FromString(Binding->SourcePropertyPath.ToString());
			}
			else
			{
				Tooltips = LOCTEXT("Bind_Invalid_Property", "You should choose a property to bind first.");
			}
		}
	}
	return Tooltips;
}

FLinearColor GetBindingColor(TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	FLinearColor BindingColor = FLinearColor::Gray;
	FNeAbilityPropertyBinding* Binding = FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(InAbilityEditor.Pin()->GetEditingAbility(), InBeam, InPropertyHandle->GetProperty()->GetFName());
	if (Binding)
	{
		if (Binding->Type == ENeAbilityPropertyBindingType::DataBoard)
		{
			return FLinearColor::Blue;
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Curve)
		{
			return FLinearColor::Yellow;
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Function)
		{
			return FLinearColor::White;
		}
		else if (Binding->Type == ENeAbilityPropertyBindingType::Property)
		{
			return FLinearColor::Green;
		}
	}
	return BindingColor;
}

TSharedRef<SWidget> FNeAbilityPropertyBindingEditor::MakePropertyBindingWidget(TWeakPtr<FNeAbilityBlueprintEditor> InAbilityEditor, UNeAbilityBeam* InBeam, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	FProperty* PropertyToBindTo = InPropertyHandle->GetProperty();
	TFieldPath<FProperty> BindingPropertyPath(PropertyToBindTo);

	FPropertyBindingWidgetArgs Args;
	Args.Property = InPropertyHandle->GetProperty();

	Args.OnGenerateBindingName = FOnGenerateBindingName::CreateLambda([InBeam](){ return InBeam->GetName(); });
	Args.OnGotoBinding = FOnGotoBinding::CreateStatic(OnGotoBinding, InAbilityEditor, InBeam, InPropertyHandle);
	Args.OnCanGotoBinding = FOnCanGotoBinding::CreateStatic(OnCanGotoBinding, InAbilityEditor, InBeam, InPropertyHandle);
	Args.OnCanBindProperty = FOnCanBindProperty::CreateLambda([BindingPropertyPath](FProperty* InProperty)
	{
		const IPropertyAccessEditor& PropertyAccessEditor = IModularFeatures::Get().GetModularFeature<IPropertyAccessEditor>("PropertyAccessEditor");
		const FProperty* BindingProperty = BindingPropertyPath.Get();
		return BindingProperty && PropertyAccessEditor.GetPropertyCompatibility(InProperty, BindingProperty) != EPropertyAccessCompatibility::Incompatible;
	});
	Args.OnCanBindFunction = FOnCanBindFunction::CreateLambda([BindingPropertyPath](UFunction* InFunction)
	{
		const IPropertyAccessEditor& PropertyAccessEditor = IModularFeatures::Get().GetModularFeature<IPropertyAccessEditor>("PropertyAccessEditor");
		const FProperty* BindingProperty = BindingPropertyPath.Get();
		const FProperty* RetProp = InFunction->GetReturnProperty();
		if (RetProp == nullptr && InFunction->HasAnyFunctionFlags(EFunctionFlags::FUNC_HasOutParms))
		{
			for (TFieldIterator<FProperty> ParamIt(InFunction, EFieldIteratorFlags::ExcludeSuper); ParamIt; ++ParamIt)
			{
				const FProperty* ParamProperty = *ParamIt;
				if (ParamProperty->HasAnyPropertyFlags(CPF_OutParm))
				{
					RetProp = ParamProperty;
					break;
				}
			}
		}
		return InFunction->NumParms == 1
			&& BindingProperty != nullptr && PropertyAccessEditor.GetPropertyCompatibility(RetProp, BindingProperty) != EPropertyAccessCompatibility::Incompatible
			&& InFunction->HasAnyFunctionFlags(FUNC_BlueprintPure);
	});
	Args.OnCanBindToClass = FOnCanBindToClass::CreateStatic(OnCanBindToClass);
	Args.OnAddBinding = FOnAddBinding::CreateStatic(OnAddBinding, InAbilityEditor, InBeam, InPropertyHandle);
	Args.OnRemoveBinding = FOnRemoveBinding::CreateStatic(OnRemoveBinding, InAbilityEditor, InBeam, InPropertyHandle);
	Args.OnCanRemoveBinding = FOnCanRemoveBinding::CreateStatic(OnCanRemoveBinding, InAbilityEditor, InBeam, InPropertyHandle);
	Args.CurrentBindingText = MakeAttributeLambda([=] { return GetBindingText(InAbilityEditor, InBeam, InPropertyHandle); } );
	Args.CurrentBindingImage = MakeAttributeLambda([=] { return GetBindingImage(InAbilityEditor, InBeam, InPropertyHandle);  } );
	Args.CurrentBindingToolTipText = MakeAttributeLambda([=] { return GetBindingTooltips(InAbilityEditor, InBeam, InPropertyHandle); } );
	Args.CurrentBindingColor = MakeAttributeLambda([=] { return GetBindingColor(InAbilityEditor, InBeam, InPropertyHandle);  } );
	Args.MenuExtender = MakeShared<FExtender>();
	Args.MenuExtender->AddMenuExtension("BindingActions", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateStatic(AddBindingMenuExtension, InAbilityEditor, InBeam, InPropertyHandle));
	Args.bGeneratePureBindings = true;
	Args.bAllowNewBindings = true;

	// const bool bIsArrayOrArrayElement = InPropertyHandle->AsArray().IsValid();
	// const bool bIsArray = bIsArrayOrArrayElement;
	// Args.bAllowUObjectFunctions = !bIsArray;
	// Args.bAllowStructFunctions = !bIsArray;

	const IPropertyAccessEditor& PropertyAccessEditor = IModularFeatures::Get().GetModularFeature<IPropertyAccessEditor>("PropertyAccessEditor");
	TSharedRef<SWidget> BindingWidget = PropertyAccessEditor.MakePropertyBindingWidget(InAbilityEditor.Pin()->GetBlueprintObj(), Args);

	TSharedRef<SWidget> ExtraBindingWidget = SNullWidget::NullWidget;

	return SNew(SHorizontalBox)
		 + SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.AutoWidth()
		[
			BindingWidget
		]
		+ SHorizontalBox::Slot()
		[
			ExtraBindingWidget
		]
	;
	// return BindingWidget;
}

FNeAbilityPropertyBinding* FNeAbilityPropertyBindingEditor::GetExistPropertyBinding(UNeAbility* InAbility, UNeAbilityBeam* InBeam, const FName& InPropertyName)
{
	check(InAbility);
	for (FNeAbilityPropertyBinding& Binding : InAbility->PropertyBindings)
	{
		if (Binding.ObjectName == InBeam->GetName() && Binding.PropertyName == InPropertyName)
		{
			return &Binding;
		}
	}

	return nullptr;
}

FNeAbilityPropertyBinding& FNeAbilityPropertyBindingEditor::AddNewPropertyBinding(UNeAbility* InAbility, UNeAbilityBeam* InBeam, const FName& InPropertyName, TSharedRef<IPropertyHandle> InPropertyHandle, ENeAbilityPropertyBindingType BindingType)
{
	FNeAbilityPropertyBinding* ExistingBinding = GetExistPropertyBinding(InAbility, InBeam, InPropertyName);
	check(ExistingBinding == nullptr);
	if (ExistingBinding)
	{
		return *ExistingBinding;
	}

	FNeAbilityPropertyBinding& NewBinding = InAbility->PropertyBindings.AddDefaulted_GetRef();
	NewBinding.ObjectName = InBeam->GetName();
	NewBinding.PropertyName = InPropertyName;
	NewBinding.PropertyPath = InPropertyHandle->GeneratePathToProperty();
	NewBinding.Type = BindingType;

	InAbility->NotifyPropertyBindingChanged(NewBinding);

	return NewBinding;
}

void FNeAbilityPropertyBindingEditor::RemovePropertyBinding(UNeAbility* InAbility, UNeAbilityBeam* InBeam, const FName& InPropertyName)
{
	FNeAbilityPropertyBinding BindingToRemove;
	for (auto It = InAbility->PropertyBindings.CreateIterator(); It; ++ It)
	{
		if (It->ObjectName == InBeam->GetName() && It->PropertyName == InPropertyName)
		{
			BindingToRemove = *It;
			It.RemoveCurrent();
			break;
		}
	}
	InAbility->NotifyPropertyBindingChanged(BindingToRemove);
}

void FNeAbilityPropertyBindingEditor::ChangePropertyBindingType(UNeAbility* InAbility, FNeAbilityPropertyBinding* InBinding, ENeAbilityPropertyBindingType NewBindingType)
{
	if (InBinding == nullptr || InBinding->Type == NewBindingType) return;
	InBinding->Type = NewBindingType;
	InAbility->NotifyPropertyBindingChanged(*InBinding);
}

void FNeAbilityPropertyBindingEditor::InitializeBindingCurveData(FNeAbilityPropertyBinding* InBinding, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	check(InBinding && InBinding->Type == ENeAbilityPropertyBindingType::Curve);
	FProperty* Property = InPropertyHandle->GetProperty();

	if (Property->IsA<FFloatProperty>() || Property->IsA<FDoubleProperty>())
	{
		InBinding->CurveData.MakeFloatCurve();
	}
	else if (Property->IsA<FIntProperty>())
	{
		InBinding->CurveData.MakeFloatCurve();
	}
	else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		const FName StructName = StructProperty->Struct->GetFName();

		if (StructName == NAME_Vector ||
			StructName == NAME_Vector2D ||
			StructName == NAME_Vector4 ||
			StructName == NAME_Quat)
		{
			InBinding->CurveData.MakeVectorCurve();
		}
		else if (StructName == NAME_Rotation)
		{
			InBinding->CurveData.MakeVectorCurve();
		}
		else if (StructName == NAME_LinearColor)
		{
			InBinding->CurveData.MakeColorCurve();
		}
	}

}

#undef LOCTEXT_NAMESPACE
