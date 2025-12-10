// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityTimeController.h"

#include "NeAbility.h"

void FNeAbilityTimeController::InitializeFor(UNeAbility* InAbility)
{
	Ability = InAbility;
}

void FNeAbilityTimeController::StartPlaying(int32 FromSection)
{
	PlayedTime = 0;
	SubPlayedTime = 0;
	PlayingSection = FromSection;
}

void FNeAbilityTimeController::StopPlaying()
{
	PlayedTime = -1;
	SubPlayedTime = -1;
	PlayingSection = -1;
}

void FNeAbilityTimeController::Update(float DeltaTime)
{
	PlayedTime += DeltaTime;

	float RemainingTime = DeltaTime;
	do {
		FNeAbilitySection& Section = Ability->GetSection(PlayingSection);

		const float PreviousSectionTime = SubPlayedTime;

		const float SectionStepTime = FMath::Min(RemainingTime, FMath::Max(Section.SectionDuration - SubPlayedTime, 0));

		SubPlayedTime += SectionStepTime;

		UpdateSectionTimeCursor(SubPlayedTime, PreviousSectionTime, SectionStepTime, PlayingSection);

		if (SubPlayedTime >= Section.SectionDuration)
		{
			ReachSectionEnd(PlayingSection);

			if (Section.NextSection == INDEX_NONE)
			{
				ReachAbilityEnd();
				break;
			}
			else
			{
				MoveToSection(Section.NextSection);
			}
		}

		RemainingTime -= SectionStepTime;
	} while (RemainingTime > 0);
}

void FNeAbilityTimeController::UpdateSectionTimeCursor(float Position, float PreviousPosition, float DeltaTime, int32 SectionIndex)
{
	check(Ability);
	Ability->UpdateSectionTimeCursor(Position, PreviousPosition, DeltaTime, SectionIndex);
}

void FNeAbilityTimeController::ReachSectionEnd(int32 SectionIndex)
{
	check(Ability);
	Ability->OnReachSectionEnd(SectionIndex);
}

void FNeAbilityTimeController::ReachAbilityEnd()
{
	PlayingSection = INDEX_NONE;
}

void FNeAbilityTimeController::MoveToSection(int32 NextSection)
{
	PlayingSection = NextSection;
	SubPlayedTime = 0;
}
