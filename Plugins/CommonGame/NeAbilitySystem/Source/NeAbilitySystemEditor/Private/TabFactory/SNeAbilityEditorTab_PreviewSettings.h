// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * SNeAbilityEditorTab_PreviewSettings
 */
class SNeAbilityEditorTab_PreviewSettings : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_PreviewSettings) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit);
};
