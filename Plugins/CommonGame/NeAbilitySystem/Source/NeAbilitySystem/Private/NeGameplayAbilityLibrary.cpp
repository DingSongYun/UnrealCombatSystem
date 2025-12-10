// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeGameplayAbilityLibrary.h"
#include "GameplayAbilitySpec.h"
#include "NeAbilityLocatingData.h"
#include "Abilities/GameplayAbility.h"
#include "Blueprint/BlueprintExceptionInfo.h"
#include "Components/MeshComponent.h"

#define LOCTEXT_NAMESPACE "NeGameplayAbilityLibrary"

bool UNeGameplayAbilityLibrary::IsTaskFactoryMethod(const UFunction* Function, const UClass* InTargetType)
{
	if (!Function->HasAnyFunctionFlags(FUNC_Static))
	{
		return false;
	}

	if (!Function->GetOwnerClass()->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists))
	{
		const FObjectProperty* ReturnProperty = CastField<FObjectProperty>(Function->GetReturnProperty());
		// see if the function is a static factory method
		bool const bIsFactoryMethod = (ReturnProperty != nullptr) && (ReturnProperty->PropertyClass != nullptr) &&
			ReturnProperty->PropertyClass->IsChildOf(InTargetType);

		return bIsFactoryMethod;
	}
	return false;
}

USceneComponent* UNeGameplayAbilityLibrary::GetComponentByNameOrTag(const FName& NameOrTag, AActor* InActor)
{
	if (!InActor) return nullptr;


	if (FProperty* Prop = InActor->GetClass()->FindPropertyByName(NameOrTag))
	{
		UObject* ObjectValue = CastField<FObjectProperty>(Prop)->GetObjectPropertyValue_InContainer(InActor);
		USceneComponent* Comp = Cast<USceneComponent>(ObjectValue);
		if (Comp)
		{
			return Comp;
		}
	}


	// 关于根据Tag查询，可以在这里自定义一些逻辑
	TArray<USceneComponent*> AllSceneComps;
	InActor->GetComponents<USceneComponent>(AllSceneComps);
	for (int32 i = 0; i < AllSceneComps.Num(); ++i)
	{
		USceneComponent* Comp = AllSceneComps[i];

		if (Comp && (Comp->GetFName().IsEqual(NameOrTag) || Comp->ComponentHasTag(NameOrTag)))
			return Comp;
	}

	return nullptr;
}

USceneComponent* UNeGameplayAbilityLibrary::GetComponentOfSocket(const FName& SocketName, AActor* InActor)
{
	check(InActor);
	check(!SocketName.IsNone())
	TArray<USceneComponent*> Components;
	InActor->GetComponents<USceneComponent>(Components);
	for (int32 i = 0; i < Components.Num(); ++i)
	{
		if (Components[i]->DoesSocketExist(SocketName))
		{
			return Components[i];
		}
	}

	return nullptr;
}

FGameplayAbilitySpec UNeGameplayAbilityLibrary::Make_GameplayAbilitySpec(TSubclassOf<UGameplayAbility> InAbilityClass, int32 InLevel)
{
	check(InAbilityClass);
	return FGameplayAbilitySpec(InAbilityClass, InLevel);
}

FTransform UNeGameplayAbilityLibrary::AbilityLocatingData_GetWorldTransform(const FNeAbilityLocatingData& InData)
{
	return InData.GetWorldTransform();
}

void UNeGameplayAbilityLibrary::AbilityLocatingData_BuildFromBeam(const FNeAbilityLocatingData& InData, const UNeAbilityBeam* InBeam)
{
	InData.GetLocatingContextBuilder().BuildFromBeam(InBeam);
}

void UNeGameplayAbilityLibrary::AbilityLocatingData_UpdateTarget(const FNeAbilityLocatingData& InData, const FNeAbilityTargetingInfo& InTarget)
{
	InData.GetLocatingContextBuilder().UpdateTarget(InTarget);
}

const FNeAbilityLocatingData& UNeGameplayAbilityLibrary::AbilityLocatingData_UpdateTargetActor(const FNeAbilityLocatingData& InData, AActor* InTargetActor)
{
	InData.GetLocatingContextBuilder().UpdateTarget(InTargetActor);
	return InData;
}

FNeAbilityTargetData_Generic* UNeGameplayAbilityLibrary::MakeGenericStructTargetData( const UScriptStruct* InScriptStruct, const uint8* InStructMemory )
{
	FNeAbilityTargetData_Generic* TargetData = new FNeAbilityTargetData_Generic();
	TargetData->Data.InitializeAs(InScriptStruct, InStructMemory);
	return TargetData;
}

void UNeGameplayAbilityLibrary::GetGenericStructFromTargetData(const FNeAbilityTargetData_Generic& InTargetData, void*& OutStructMemory)
{
	if (InTargetData.Data.IsValid())
	{
		InTargetData.Data.GetScriptStruct()->CopyScriptStruct(OutStructMemory, InTargetData.Data.GetMemory());
	}
}

FGameplayAttribute UNeGameplayAbilityLibrary::MakeGameplayAttribute(TSubclassOf<UAttributeSet> AttributeClass, const FName& AttributeName)
{
	FProperty* AttrProp = FindFieldChecked<FProperty>(AttributeClass, AttributeName);
	return AttrProp;
}

DEFINE_FUNCTION(UNeGameplayAbilityLibrary::execAbilityTargetDataFromStruct)
{
	// Read wildcard Value input.
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	const void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			LOCTEXT("NeGameplayAbility_MakeTargetDataWarning", "Failed to resolve the Value for Ability TargetData From Struct")
		);

		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);

		P_NATIVE_BEGIN;
		static_cast<FGameplayAbilityTargetDataHandle*>(RESULT_PARAM)->Data.Reset();
		P_NATIVE_END;
	}
	else
	{
		P_NATIVE_BEGIN;
		FGameplayAbilityTargetDataHandle* TargetDataHandle = static_cast<FGameplayAbilityTargetDataHandle*>(RESULT_PARAM);
		TargetDataHandle->Data.Add(TSharedPtr<FGameplayAbilityTargetData>(MakeGenericStructTargetData(ValueProp->Struct, (const uint8*)ValuePtr)));
		P_NATIVE_END;
	}
}

DEFINE_FUNCTION(UNeGameplayAbilityLibrary::execGetStructFromTargetData)
{
	P_GET_STRUCT_REF(FGameplayAbilityTargetDataHandle, TargetDataHandle);

	// Read wildcard Value input.
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			LOCTEXT("NeGameplayAbility_GetTargetDataWarning", "Failed to resolve the Value for GetStructFromTargetData")
		);

		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else
	{
		P_NATIVE_BEGIN;
		if (TargetDataHandle.IsValid(0))
		{
			const TSharedPtr<FGameplayAbilityTargetData> TargetData = TargetDataHandle.Data[0];
			if (TargetData->GetScriptStruct() == FNeAbilityTargetData_Generic::StaticStruct())
			{
				TSharedPtr<FNeAbilityTargetData_Generic> GenericTargetData = StaticCastSharedPtr<FNeAbilityTargetData_Generic>(TargetData);
				if (GenericTargetData->IsDataValid() && GenericTargetData->GetDataStructType() == ValueProp->Struct)
				{
					UNeGameplayAbilityLibrary::GetGenericStructFromTargetData(*GenericTargetData.Get(), ValuePtr);
				}
			}
		}
		P_NATIVE_END;
	}
}

#undef LOCTEXT_NAMESPACE