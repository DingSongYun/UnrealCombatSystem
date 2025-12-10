// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "IDocumentation.h"
#include "Viewport/SNeSimpleEdViewport.h"

class FNeAbilityBlueprintEditor;
class FEditorViewportClient;

/**
 * SNeAbilityEditorViewpor
 */
class SNeAbilityEditorViewport : public SNeSimpleEdViewport
{
	friend class SNeAbilityEditorViewportToolBar;
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorViewport) {}
	SLATE_END_ARGS()

	virtual ~SNeAbilityEditorViewport();

	void Construct(const FArguments& InArgs, const FEdViewportArgs& InRequiredArgs);
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
	// SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual void BindCommands() override;
	virtual void OnFocusViewportToSelection() override;
	// End of SEditorViewport interface

protected:
	TWeakPtr<FNeAbilityBlueprintEditor> HostEditor;

	FDelegateHandle PreviewFeatureLevelChangedHandle;

	void OnSetPlaybackSpeed(int32 PlaybackSpeedMode) const;

	bool IsPlaybackSpeedSelected(int32 PlaybackSpeedMode) const;
};