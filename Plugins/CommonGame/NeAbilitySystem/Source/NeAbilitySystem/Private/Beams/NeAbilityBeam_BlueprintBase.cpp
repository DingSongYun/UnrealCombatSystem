// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_BlueprintBase.h"

struct TestParams
{
	FNeAbilitySegmentEvalContext EvalContext;
};

void UNeAbilityBeam_BlueprintBase::InitInstanceFor(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::InitInstanceFor(InAbility, EvalContext);
	ReceiveOnInstanceInitialize();
}

void UNeAbilityBeam_BlueprintBase::OnActive(FNeAbilitySegmentEvalContext& EvalContext)
{
	ReceiveActive(EvalContext);
}

void UNeAbilityBeam_BlueprintBase::OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext)
{
	ReceiveUpdate(DeltaTime, EvalContext);
}

void UNeAbilityBeam_BlueprintBase::OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason)
{
	ReceiveEnd(EvalContext, EndReason);
}

void UNeAbilityBeam_BlueprintBase::SamplePosition(const float Position, const float PreviousPosition)
{
	Super::SamplePosition(Position, PreviousPosition);
	ReceiveSamplePosition(Position, PreviousPosition);
}

#if WITH_EDITOR
FText UNeAbilityBeam_BlueprintBase::GetDisplayText() const
{
	FText DisplayText = Super::GetDisplayText();
	MakeDisplayText(DisplayText, DisplayText);

	return DisplayText;
}
#endif

