// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

namespace NeAbilityEditorConstants
{
	const float TrackHeight = 30.0f;
	const float SubTrackHeight = 25.0f;
	const float CompositeTrackHeight = 20.f;

	const FLinearColor TrackNodeColorEnable = FColor::Orange;
	const FLinearColor TrackInstantNodeColorEnable = FColor(250, 180, 70);
	const FLinearColor TrackNodeColorDisable = FLinearColor::Gray;
	const FLinearColor SubTrackNodeColorEnable = FLinearColor(0.2, 1, 0.2f);
	const FLinearColor SubTrackNodeColorDisable = FLinearColor::Gray;
	const FLinearColor CompositeTaskNodeColorEnable = FLinearColor(0.03f, 0.76f, 0.7f, 0.8f);
	const FLinearColor CompositeTaskNodeColorDisable = FLinearColor::Gray;

	const FLinearColor DebuggerNodeUnActiveColor = FColor(243, 156, 18, 128);
	const FLinearColor DebuggerNodeActivatingColor = FColor::Green;
	const FLinearColor DebuggerNodeActivedColor = FColor(0, 200, 0, 128);

	inline float GetTrackHeight(bool bIsSubTrack)
	{
		return bIsSubTrack ? SubTrackHeight : TrackHeight;
	}
}

enum class EAbilityNodeExecutionState : uint8
{
	UnActivated = 0,
	Activating,
	Activated,
};