// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityEditorViewportPlaybackCommands.h"
#include "NeAbilityEditorViewportToolbar.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "AbilityViewportPlaybackCommands"

FNeAbilityEditorViewportPlaybackCommands::FNeAbilityEditorViewportPlaybackCommands() : TCommands<FNeAbilityEditorViewportPlaybackCommands>
	(
		TEXT("AbilityViewportPlayback"), // Context name for fast lookup
		NSLOCTEXT("Contexts", "AbilityViewportPlayback", "NeAbilitySystem Viewport Playback"), // Localized context name for displaying
		NAME_None, // Parent context name.  
		FAppStyle::GetAppStyleSetName() // Icon Style Set
	)
{
	PlaybackSpeedCommands.AddZeroed(ENeAbilityPlaybackSpeeds::NumPlaybackSpeeds);
}

void FNeAbilityEditorViewportPlaybackCommands::RegisterCommands()
{
	UI_COMMAND(PlaybackSpeedCommands[ENeAbilityPlaybackSpeeds::OneTenth], "x0.1", "Set the animation playback speed to a tenth of normal", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(PlaybackSpeedCommands[ENeAbilityPlaybackSpeeds::Quarter], "x0.25", "Set the animation playback speed to a quarter of normal", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(PlaybackSpeedCommands[ENeAbilityPlaybackSpeeds::Half], "x0.5", "Set the animation playback speed to a half of normal", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(PlaybackSpeedCommands[ENeAbilityPlaybackSpeeds::Normal], "x1.0", "Set the animation playback speed to normal", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(PlaybackSpeedCommands[ENeAbilityPlaybackSpeeds::Double], "x2.0", "Set the animation playback speed to double the speed of normal", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(PlaybackSpeedCommands[ENeAbilityPlaybackSpeeds::FiveTimes], "x5.0", "Set the animation playback speed to five times the normal speed", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(PlaybackSpeedCommands[ENeAbilityPlaybackSpeeds::TenTimes], "x10.0", "Set the animation playback speed to ten times the normal speed", EUserInterfaceActionType::RadioButton, FInputChord());

}

#undef LOCTEXT_NAMESPACE