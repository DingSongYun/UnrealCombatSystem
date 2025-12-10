// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityPreviewScene.h"
#include "Components/VerticalBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Viewport/SNeSimpleEdViewport.h"

//=============================================================================
/**
 * SNeAbilityyEditorTab_Viewport
 */
class SNeAbilityEditorTab_Viewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_Viewport) {};
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs,
		const TSharedRef<FNeAbilityBlueprintEditor>& InAssetEditorToolkit,
		const TSharedRef<FNeAbilityPreviewScene>& InPreviewScene,
		int32 InViewportIndex)
	{
		FEdViewportArgs ViewportArgs(InPreviewScene, InAssetEditorToolkit, InViewportIndex);
		ViewportWidget = SNew(SNeAbilityEditorViewport, ViewportArgs);
		TSharedPtr<FEditorViewportClient> ViewportClient = ViewportWidget->GetViewportClient();
		ViewportClient->SetRealtime(true);

		this->ChildSlot
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				ViewportWidget.ToSharedRef()
			]
		];
	}

	TSharedPtr<FEditorViewportClient> GetViewportClinet() const
	{
		return ViewportWidget->GetViewportClient();
	}

private:
	TSharedPtr<SNeAbilityEditorViewport> ViewportWidget;
};