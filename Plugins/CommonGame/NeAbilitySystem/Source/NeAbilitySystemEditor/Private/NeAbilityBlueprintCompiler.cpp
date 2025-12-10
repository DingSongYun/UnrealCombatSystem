// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityBlueprintCompiler.h"

#include "BlueprintEditorSettings.h"
#include "EdGraphSchema_K2.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Beams/NeAbilityBeam_GameplayTask.h"
#include "Beams/NeAbilityBeam_GameplayEffect.h"
#include "Beams/NeAbilityBeam_GameplayCue.h"

namespace BeamLinkageUtils
{
	static bool IsFactoryMethod(const UFunction* Function, const UClass* InTargetType)
	{
		if (!Function->HasAnyFunctionFlags(FUNC_Static))
		{
			return false;
		}

		if (!Function->GetOwnerClass()->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists) &&
			(!Function->HasMetaData(FBlueprintMetadata::MD_DeprecatedFunction) || GetDefault<UBlueprintEditorSettings>()->bExposeDeprecatedFunctions))
		{
			FObjectProperty* ReturnProperty = CastField<FObjectProperty>(Function->GetReturnProperty());
			// see if the function is a static factory method
			bool const bIsFactoryMethod = (ReturnProperty != nullptr) && (ReturnProperty->PropertyClass != nullptr) &&
				ReturnProperty->PropertyClass->IsChildOf(InTargetType);

			return bIsFactoryMethod;
		}
		else
		{
			return false;
		}
	}
}

void FBeamLinkageCompilerContext::PostConstructBeamLinkage(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeamLinkage* BeamLinkage)
{
	if (UNeAbilityBeam_GameplayTask* GameplayTaskBeam = Cast<UNeAbilityBeam_GameplayTask>(BeamLinkage))
	{
		PostLinkGameplayTask(SegmentPtr, GameplayTaskBeam);
	}
	else if (UNeAbilityBeam_GameplayEffect* GameplayEffectBeam = Cast<UNeAbilityBeam_GameplayEffect>(BeamLinkage))
	{
		PostLinkGameplayEffect(SegmentPtr, GameplayEffectBeam);
	}
	else if (UNeAbilityBeam_GameplayCue* GameplayCueBeam = Cast<UNeAbilityBeam_GameplayCue>(BeamLinkage))
	{
		PostLinkGameplayCue(SegmentPtr, GameplayCueBeam);
	}
}

void FBeamLinkageCompilerContext::PostLinkGameplayTask(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeam_GameplayTask* GameplayTaskBeam)
{
	check(!GameplayTaskBeam->LinkedClass.IsNull());
	UClass* LinkedClass = GameplayTaskBeam->LinkedClass.ResolveClass();
	if (LinkedClass == nullptr)
	{
		LinkedClass = GameplayTaskBeam->LinkedClass.TryLoadClass<UClass>();
	}
	check(LinkedClass);

	if (LinkedClass->HasAnyClassFlags(CLASS_Abstract))
	{
		checkf(false, TEXT("We should never try to link a abstract ability task."));
		return ;
	}

	UFunction* FactoryFunction = nullptr;
	if (GameplayTaskBeam->bCompactParams)
	{
		for (TFieldIterator<UFunction> FuncIt(LinkedClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
		{
			UFunction* Function = *FuncIt;
			if (!BeamLinkageUtils::IsFactoryMethod(Function, LinkedClass))
			{
				continue;
			}
			FactoryFunction = Function;
		}
		check(FactoryFunction);
	}

	if (GameplayTaskBeam->bCompactParams && FactoryFunction)
	{
		for (TFieldIterator<FProperty> PropIt(FactoryFunction); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++ PropIt)
		{
			FProperty* Param = *PropIt;
			const bool bIsFunctionInput = !Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm);
			if (!bIsFunctionInput)
			{
				// skip function output, it's internal node data
				continue;
			}
			GameplayTaskBeam->TaskParameters.AddProperty(Param->GetFName(), Param);

			// Copy Default Value
			const FName DefaultPropertyKey(*(FString(TEXT("CPP_Default_")) + Param->GetName()));
			const FString& PropertyDefaultValue = FactoryFunction->GetMetaData(DefaultPropertyKey);
			if (!PropertyDefaultValue.IsEmpty())
			{
				const FPropertyBagPropertyDesc* Desc = GameplayTaskBeam->TaskParameters.FindPropertyDescByName(Param->GetFName());
				// void* PropAddress = GameplayTaskBeam->TaskParameters.GetMutableValueAddress(Desc);
				// const TCHAR* Result = Desc->CachedProperty->ImportText_InContainer(*PropertyDefaultValue, PropAddress, nullptr, PPF_None);
				// if (Result == nullptr)
				// {
				// 	ensure(false);
				// }
			}
		}
	}
	else
	{
		GameplayTaskBeam->TaskTemplate = NewObject<UAbilityTask>(GameplayTaskBeam, LinkedClass, NAME_None, RF_Public | RF_ArchetypeObject);
	}
}

void FBeamLinkageCompilerContext::PostLinkGameplayEffect(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeam_GameplayEffect* GameplayTaskBeam)
{
}

void FBeamLinkageCompilerContext::PostLinkGameplayCue(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeam_GameplayCue* GameplayTaskBeam)
{
}
