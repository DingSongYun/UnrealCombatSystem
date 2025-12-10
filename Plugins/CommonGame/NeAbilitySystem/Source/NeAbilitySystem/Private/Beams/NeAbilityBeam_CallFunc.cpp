// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_CallFunc.h"
#include "NeAbility.h"

void FNeAbilityFuncRef::SetFunction(const UFunction* InFunction)
{
	if (InFunction == nullptr)
	{
		FunctionReference = FCachedPropertyPath();
		return;
	}
	FunctionReference = FCachedPropertyPath(InFunction->GetName());
}

UFunction* FNeAbilityFuncRef::Resolve(UObject* InContainer) const
{
	FunctionReference.Resolve(InContainer);
	return FunctionReference.GetCachedFunction();
}

UNeAbilityBeam_CallFunc::UNeAbilityBeam_CallFunc(const FObjectInitializer& Initializer) : Super(Initializer)
{
	bRepeat = false;
	DurationType = EAbilityDurationPolicy::Instant;
}

void UNeAbilityBeam_CallFunc::OnActive(FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnActive(EvalContext);
	CachedFunction = FunctionData.Resolve(OwnerAbility);
	ExecuteFunction();
}

void UNeAbilityBeam_CallFunc::OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnUpdate(DeltaTime, EvalContext);

	if (bRepeat)
	{
		ExecuteFunction();
	}
}

void UNeAbilityBeam_CallFunc::ExecuteFunction()
{
	if (CachedFunction == nullptr)
	{
		return ;
	}

	OwnerAbility->ProcessEvent(CachedFunction, nullptr);
}
