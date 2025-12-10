// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_Trigger.h"

#include "NeAbilitySegmentEvalQueue.h"
#include "UObject/ObjectSaveContext.h"

UNeAbilityBeam_Trigger::UNeAbilityBeam_Trigger(const FObjectInitializer& Initializer) : Super(Initializer)
{
	bRepeat = false;
	bCompound = true;
}

void UNeAbilityBeam_Trigger::ExecuteTrigger(FNeAbilitySegmentEvalContext& EvalContext)
{
	EvalContext.NotifySegmentChildrenTriggered();

	if (!bRepeat)
	{
		RequestEnd();
	}
	else
	{
		ResetState();
	}
}

void UNeAbilityBeam_Trigger::ResetState()
{
}

#if WITH_EDITOR
void UNeAbilityBeam_Trigger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property->GetName() == TEXT("SimulatorStartTime"))
	{
		SimulatorStartTime = FMath::Min(SimulatorStartTime, FMath::Max(GetDuration() - 0.05f, 0));
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif