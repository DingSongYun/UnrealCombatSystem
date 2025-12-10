// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityFuncRefCustomization.h"

#include "BlueprintEditorModule.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "IPropertyAccessEditor.h"
#include "K2Node_FunctionEntry.h"
#include "Abilities/GameplayAbility.h"
#include "Beams/NeAbilityBeam_CallFunc.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Features/IModularFeatures.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SFunctionPicker.h"

#define LOCTEXT_NAMESPACE "NeAbilityFuncRefCustomization"

FName FNeAbilityFuncRefCustomization::GetTypeName()
{
	return FNeAbilityFuncRef::StaticStruct()->GetFName();
}

void FNeAbilityFuncRefCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> FunctionReference = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FNeAbilityFuncRef, FunctionReference));
	HeaderRow
	.NameContent()
	[
		InStructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		MakeFunctionRefWidget2(InStructPropertyHandle, FunctionReference.ToSharedRef())
	];
}

void FNeAbilityFuncRefCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

TSharedRef<SWidget> FNeAbilityFuncRefCustomization::MakeFunctionRefWidget(TSharedRef<IPropertyHandle> InFuncRefPropertyHandle)
{
	UBlueprint* Blueprint = nullptr;
	TArray<UObject*> OuterObjects;
	InFuncRefPropertyHandle->GetOuterObjects(OuterObjects);
	if (OuterObjects.Num() > 0)
	{
		UObject* OuterObject = OuterObjects[0];
		UGameplayAbility* Ability = OuterObject->GetTypedOuter<UGameplayAbility>();
		// UClass* AbilityClass = Ability->GetClass()
		UBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(Ability->GetClass());
		Blueprint = Cast<UBlueprint>(BlueprintGeneratedClass->ClassGeneratedBy);
	}
	if (Blueprint == nullptr)
	{
		return SNullWidget::NullWidget;
	}

	if(IModularFeatures::Get().IsModularFeatureAvailable("PropertyAccessEditor") == false)
	{
		return SNullWidget::NullWidget;
	}

	const bool bFunctionReference = true;

	// Try to find the prototype function
	const FString& PrototypeFunctionName = InFuncRefPropertyHandle->GetMetaData("PrototypeFunction");
	UFunction* PrototypeFunction = PrototypeFunctionName.IsEmpty() ? nullptr : FindObject<UFunction>(nullptr, *PrototypeFunctionName);

	FString DefaultBindingName = InFuncRefPropertyHandle->HasMetaData("DefaultBindingName") ? InFuncRefPropertyHandle->GetMetaData("DefaultBindingName") : TEXT("NewFunction");

	// Binding widget re-adds the 'On' prefix because of legacy support for UMG events, so we remove it here
	DefaultBindingName.RemoveFromStart(TEXT("AbilityEvent_"));

	auto OnGenerateBindingName = [DefaultBindingName]() -> FString
	{
		return DefaultBindingName;
	};

	auto OnCanBindProperty = [](FProperty* InProperty)
	{
		return true;
	};

	auto OnGoToBinding = [InFuncRefPropertyHandle, Blueprint](FName InPropertyName)
	{
		void* StructData = nullptr;
		const FPropertyAccess::Result Result = InFuncRefPropertyHandle->GetValueData(StructData);
		if(Result == FPropertyAccess::Success)
		{
			check(StructData);
			FMemberReference* MemberReference = static_cast<FMemberReference*>(StructData);
			UFunction* Function = MemberReference->ResolveMember<UFunction>(Blueprint->SkeletonGeneratedClass);
			if(Function)
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);

				if(IBlueprintEditor* BlueprintEditor = static_cast<IBlueprintEditor*>(GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Blueprint, true)))
				{
					BlueprintEditor->JumpToHyperlink(Function, false);
					return true;
				}
			}
		}

		return false;
	};

	auto OnCanGotoBinding = [InFuncRefPropertyHandle](FName InPropertyName)
	{
		void* StructData = nullptr;
		const FPropertyAccess::Result Result = InFuncRefPropertyHandle->GetValueData(StructData);
		if(Result == FPropertyAccess::Success)
		{
			check(StructData);
			FMemberReference* MemberReference = static_cast<FMemberReference*>(StructData);
			return MemberReference->GetMemberName() != NAME_None;
		}

		return false;
	};

	auto OnCanBindFunction = [PrototypeFunction](UFunction* InFunction)
	{
		if(PrototypeFunction != nullptr)
		{
			return PrototypeFunction->IsSignatureCompatibleWith(InFunction)
				&& FBlueprintEditorUtils::HasFunctionBlueprintThreadSafeMetaData(PrototypeFunction) == FBlueprintEditorUtils::HasFunctionBlueprintThreadSafeMetaData(InFunction);
		}

		return false;
	};

	auto OnAddBinding = [InFuncRefPropertyHandle, Blueprint](FName InPropertyName, const TArray<FBindingChainElement>& InBindingChain)
	{
		void* StructData = nullptr;
		const FPropertyAccess::Result Result = InFuncRefPropertyHandle->GetValueData(StructData);
		if(Result == FPropertyAccess::Success)
		{
			InFuncRefPropertyHandle->NotifyPreChange();

			check(StructData);
			FMemberReference* MemberReference = static_cast<FMemberReference*>(StructData);
			UFunction* Function = InBindingChain[0].Field.Get<UFunction>();
			UClass* OwnerClass = Function ? Function->GetOwnerClass() : nullptr;
			bool bSelfContext = false;
			if(OwnerClass != nullptr)
			{
				bSelfContext = (Blueprint->GeneratedClass != nullptr && Blueprint->GeneratedClass->IsChildOf(OwnerClass)) ||
								(Blueprint->SkeletonGeneratedClass != nullptr && Blueprint->SkeletonGeneratedClass->IsChildOf(OwnerClass));
			}
			MemberReference->SetFromField<UFunction>(Function, bSelfContext);

			InFuncRefPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		}
	};

	auto OnRemoveBinding = [InFuncRefPropertyHandle](FName InPropertyName)
	{
		void* StructData = nullptr;
		const FPropertyAccess::Result Result = InFuncRefPropertyHandle->GetValueData(StructData);
		if(Result == FPropertyAccess::Success)
		{
			InFuncRefPropertyHandle->NotifyPreChange();

			check(StructData);
			FMemberReference* MemberReference = static_cast<FMemberReference*>(StructData);
			*MemberReference = FMemberReference();

			InFuncRefPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		}
	};

	auto CanRemoveBinding = [InFuncRefPropertyHandle](FName InPropertyName)
	{
		void* StructData = nullptr;
		const FPropertyAccess::Result Result = InFuncRefPropertyHandle->GetValueData(StructData);
		if(Result == FPropertyAccess::Success)
		{
			check(StructData);
			FMemberReference* MemberReference = static_cast<FMemberReference*>(StructData);
			return MemberReference->GetMemberName() != NAME_None;
		}

		return false;
	};

	auto OnNewFunctionBindingCreated = [PrototypeFunction](UEdGraph* InFunctionGraph, UFunction* InFunction)
	{
		// Ensure newly created function is thread safe
		if(PrototypeFunction && FBlueprintEditorUtils::HasFunctionBlueprintThreadSafeMetaData(PrototypeFunction))
		{
			TArray<UK2Node_FunctionEntry*> EntryNodes;
			InFunctionGraph->GetNodesOfClass<UK2Node_FunctionEntry>(EntryNodes);
			if(EntryNodes.Num() > 0)
			{
				EntryNodes[0]->MetaData.bThreadSafe = true;
			}
		}
	};

	auto CurrentBindingText = [bFunctionReference, Blueprint, InFuncRefPropertyHandle]()
	{
		void* StructData = nullptr;
		const FPropertyAccess::Result Result = InFuncRefPropertyHandle->GetValueData(StructData);
		if(Result == FPropertyAccess::Success)
		{
			check(StructData);
			FMemberReference* MemberReference = static_cast<FMemberReference*>(StructData);
			if(bFunctionReference)
			{
				UFunction* Function = MemberReference->ResolveMember<UFunction>(Blueprint->SkeletonGeneratedClass);
				if(Function)
				{
					return FText::FromName(Function->GetFName());
				}
				else
				{
					return FText::FromName(MemberReference->GetMemberName());
				}
			}
		}
		else if(Result == FPropertyAccess::MultipleValues)
		{
			return LOCTEXT("MultipleValues", "Multiple Values");
		}

		return FText::GetEmpty();
	};

	auto CurrentBindingToolTipText = [InFuncRefPropertyHandle]()
	{
		void* StructData = nullptr;
		const FPropertyAccess::Result Result = InFuncRefPropertyHandle->GetValueData(StructData);
		if(Result == FPropertyAccess::Success)
		{
			check(StructData);
			FMemberReference* MemberReference = static_cast<FMemberReference*>(StructData);
			return FText::FromName(MemberReference->GetMemberName());
		}
		else if(Result == FPropertyAccess::MultipleValues)
		{
			return LOCTEXT("MultipleValues", "Multiple Values");
		}

		return FText::GetEmpty();
	};

	FPropertyBindingWidgetArgs Args;
	Args.BindableSignature = PrototypeFunction;
	Args.OnGenerateBindingName = FOnGenerateBindingName::CreateLambda(OnGenerateBindingName);
	Args.OnCanBindProperty = FOnCanBindProperty::CreateLambda(OnCanBindProperty);
	Args.OnGotoBinding = FOnGotoBinding::CreateLambda(OnGoToBinding);
	Args.OnCanGotoBinding = FOnCanGotoBinding::CreateLambda(OnCanGotoBinding);
	Args.OnCanBindFunction = FOnCanBindFunction::CreateLambda(OnCanBindFunction);
	Args.OnCanBindToClass = FOnCanBindToClass::CreateLambda([](UClass* InClass){ return true; });
	Args.OnAddBinding = FOnAddBinding::CreateLambda(OnAddBinding);
	Args.OnRemoveBinding = FOnRemoveBinding::CreateLambda(OnRemoveBinding);
	Args.OnCanRemoveBinding = FOnCanRemoveBinding::CreateLambda(CanRemoveBinding);
	Args.OnNewFunctionBindingCreated = FOnNewFunctionBindingCreated::CreateLambda(OnNewFunctionBindingCreated);
	Args.CurrentBindingText = MakeAttributeLambda(CurrentBindingText);
	Args.CurrentBindingToolTipText = MakeAttributeLambda(CurrentBindingToolTipText);
	Args.CurrentBindingImage = FAppStyle::GetBrush("GraphEditor.Function_16x");
	Args.CurrentBindingColor = FAppStyle::GetSlateColor("Colors.Foreground").GetSpecifiedColor();
	Args.bGeneratePureBindings = false;
	Args.bAllowFunctionBindings = true;
	Args.bAllowFunctionLibraryBindings = true;
	Args.bAllowPropertyBindings = false;
	Args.bAllowNewBindings = true;
	Args.bAllowArrayElementBindings = false;
	Args.bAllowUObjectFunctions = true;
	Args.bAllowStructFunctions = false;
	Args.bAllowStructMemberBindings = false;

	IPropertyAccessEditor& PropertyAccessEditor = IModularFeatures::Get().GetModularFeature<IPropertyAccessEditor>("PropertyAccessEditor");
	return SNew(SBox)
		.MaxDesiredWidth(200.0f)
		[
			PropertyAccessEditor.MakePropertyBindingWidget(Blueprint, Args)
		];
}

TSharedRef<SWidget> FNeAbilityFuncRefCustomization::MakeFunctionRefWidget2(TSharedRef<IPropertyHandle> InStructPropertyHandle, TSharedRef<IPropertyHandle> InFuncRefPropertyHandle)
{
	UClass* AbilityClass = nullptr;
	UGameplayAbility* Ability = nullptr;
	TArray<UObject*> OuterObjects;
	InFuncRefPropertyHandle->GetOuterObjects(OuterObjects);
	if (OuterObjects.Num() > 0)
	{
		UObject* OuterObject = OuterObjects[0];
		Ability = OuterObject->GetTypedOuter<UGameplayAbility>();
		AbilityClass = Ability->GetClass();
	}

	if (AbilityClass == nullptr)
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SFunctionPicker, AbilityClass, nullptr)
		.IncludeSuperFunction(true)
		.CurrentFunction(this, &FNeAbilityFuncRefCustomization::GetCurrentFunctionName, InStructPropertyHandle)
		.OnSelectedFunction(this, &FNeAbilityFuncRefCustomization::OnSelectedFunction, InFuncRefPropertyHandle);
}

void FNeAbilityFuncRefCustomization::OnSelectedFunction(const FName& InFuncName, const UFunction* InFunction, TSharedRef<IPropertyHandle> InStructPropertyHandle)
{
	void* RawData = nullptr;
	InStructPropertyHandle->GetValueData(RawData);
	FNeAbilityFuncRef* FuncRef = static_cast<FNeAbilityFuncRef*>(RawData);
	FuncRef->SetFunction(InFunction);
}

TOptional<FName> FNeAbilityFuncRefCustomization::GetCurrentFunctionName(TSharedRef<IPropertyHandle> InStructPropertyHandle) const
{
	void* RawData = nullptr;
	InStructPropertyHandle->GetValueData(RawData);
	FNeAbilityFuncRef* FuncRef = static_cast<FNeAbilityFuncRef*>(RawData);
	if (FuncRef->FunctionReference.IsValid())
	{
		return FuncRef->FunctionReference.GetLastSegment().GetName();
	}

	return TOptional<FName>();
}
