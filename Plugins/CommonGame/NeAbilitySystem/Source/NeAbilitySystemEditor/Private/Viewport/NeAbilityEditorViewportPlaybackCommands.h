// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * Class containing commands for viewport playback actions
 */
class FNeAbilityEditorViewportPlaybackCommands : public TCommands<FNeAbilityEditorViewportPlaybackCommands>
{
public:
	FNeAbilityEditorViewportPlaybackCommands();

	/** Command list for playback speed, indexed by EPlaybackSpeeds*/
	TArray<TSharedPtr< FUICommandInfo >> PlaybackSpeedCommands;

public:
	/** Registers our commands with the binding system */
	virtual void RegisterCommands() override;
};