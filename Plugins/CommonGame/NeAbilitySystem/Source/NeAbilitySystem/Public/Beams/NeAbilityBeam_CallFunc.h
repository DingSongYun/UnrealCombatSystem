// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeam.h"
// #include "Engine/MemberReference.h"
#include "PropertyPathHelpers.h"
#include "NeAbilityBeam_CallFunc.generated.h"

USTRUCT(Blueprintable)
struct NEABILITYSYSTEM_API FNeAbilityFuncRef
{
	GENERATED_BODY()

public:
	void SetFunction(const UFunction* InFunction);

	UFunction* Resolve(UObject* InContainer) const;
public:
	UPROPERTY(EditAnywhere)
	FCachedPropertyPath FunctionReference;
};

/**
 * UNeAbilityBeam_Event
 */
UCLASS(Category="Misc", DisplayName="Call Function")
class NEABILITYSYSTEM_API UNeAbilityBeam_CallFunc : public UNeAbilityBeam
{
	GENERATED_BODY()
	UNeAbilityBeam_CallFunc(const FObjectInitializer& Initializer);

	virtual void OnActive(FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext) override;

protected:
	void ExecuteFunction();

protected:
	UPROPERTY(EditAnywhere, Category="Call Function")
	FNeAbilityFuncRef FunctionData;

	UPROPERTY(EditAnywhere, Category="Call Function", meta=(EditCondition = "DurationType != EAbilityDurationPolicy::Instant"))
	uint8 bRepeat : 1;

private:
	UPROPERTY()
	UFunction* CachedFunction = nullptr;
};
