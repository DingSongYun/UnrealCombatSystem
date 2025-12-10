// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PropertyEditorDelegates.h"
#include "Widgets/SCompoundWidget.h"

/**
 * SNeAbilityEditorTab_AssetDetails
 */
class SNeAbilityEditorTab_AssetDetails : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_AssetDetails) {};
			SLATE_ARGUMENT( FOnFinishedChangingProperties::FDelegate, OnFinishedChangingProperties )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit);

private:
	TSharedPtr<class SKismetInspector> KismetInspector;
};
