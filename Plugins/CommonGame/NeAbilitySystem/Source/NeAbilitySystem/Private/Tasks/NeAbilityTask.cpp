// Copyright NetEase Games, Inc. All Rights Reserved.


#include "Tasks/NeAbilityTask.h"
#include "Beams/NeAbilityBeam.h"
#include "GameFramework/Character.h"

void UNeAbilityTask::Activate()
{
	Super::Activate();
}

void UNeAbilityTask::OnDestroy(bool bInOwnerFinished)
{
	OnEndTask();
	Super::OnDestroy(bInOwnerFinished);
}

void UNeAbilityTask::SamplePosition(const float Position, const float PreviousPosition)
{
}

ACharacter* UNeAbilityTask::GetCharacter() const
{
	return Cast<ACharacter>(GetAvatarActor());
}

float UNeAbilityTask::GetRunningTime() const
{
	if (LinkedBeam)
	{
		return LinkedBeam->GetRunningTime();
	}
	return 0.f;
}

float UNeAbilityTask::GetRequiredDuration() const
{
	if (LinkedBeam)
	{
		EAbilityDurationPolicy DurationPolicy = LinkedBeam->GetDurationPolicy();
		if (DurationPolicy == EAbilityDurationPolicy::Infinite)
		{
			return -1.f;
		}
		else if (DurationPolicy == EAbilityDurationPolicy::Instant)
		{
			return 0.f;
		}
		else if (DurationPolicy == EAbilityDurationPolicy::HasDuration)
		{
			return LinkedBeam->GetDuration();
		}
	}

	return -1.f;
}

void UNeAbilityTask::OnEndTask()
{
}

#if WITH_EDITOR
void UNeAbilityTask::NotifyAnimRelevanceChanged() const
{
	if (UNeAbilityBeam* Beam = GetTypedOuter<UNeAbilityBeam>())
	{
		Beam->SetDuration(EvalAnimRelevanceDuration());
	}
}

FText UNeAbilityTask::GetDisplayText() const
{
	return GetClass()->GetDisplayNameText();
}
#endif
