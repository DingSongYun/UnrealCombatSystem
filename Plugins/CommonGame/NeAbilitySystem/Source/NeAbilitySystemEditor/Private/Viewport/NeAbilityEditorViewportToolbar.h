// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "SViewportToolBar.h"

class FExtender;
class FUICommandList;

namespace ENeAbilityPlaybackSpeeds
{
	enum Type
	{
		OneTenth = 0,
		Quarter,
		Half,
		Normal,
		Double,
		FiveTimes,
		TenTimes,
		NumPlaybackSpeeds
	};

	extern float Values[NumPlaybackSpeeds];
};

class UToolMenu;
struct FToolMenuSection;
/**
 * SNeAbilityEditorViewportToolBar
 */
class SNeAbilityEditorViewportToolBar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorViewportToolBar) { }
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<class FNeAbilityBlueprintEditor> InHostEditor, TSharedPtr<class SNeAbilityEditorViewport> InViewport);

private:
	/* Creates the basic view option menu.*/
	TSharedRef<SWidget> GenerateViewMenu();
	TSharedPtr<FExtender> GetOptionMenuExtender();

	void GenerateOptionMenu(class FMenuBuilder& MenuBuilder);

	/* Creates the Viewport Type (Perspective, etc) menu. */
	TSharedRef<SWidget> GenerateViewportTypeMenu();

	/* Camera Labels and Icons*/
	FText GetCameraMenuLabel() const;
	const FSlateBrush* GetCameraMenuLabelIcon() const;

	/* Callback for Transform bar visibility. */
	EVisibility GetTransformToolBarVisibility() const;

	/* Called by the FOV slider in the perspective viewport to get the FOV value */
	//float OnGetFOVValue() const;

	/* Called when the FOV slider is adjusted in the perspective viewport */
	//void OnFOVValueChanged(float NewValue);

	void SetCameraMode(FName CameraMode);
	FName GetCameraMode() const;
	void SetCameraLockedBone(const FName& BoneName);

	void ResetCamaraTransform(FName CameraMode);

	//~Begin: 骨骼树
	//TSharedRef<SWidget> MakeFollowBoneWidget();
	//TSharedRef<SWidget> MakeFollowBoneWidget(TWeakPtr<class SComboButton> InWeakComboButton);
	TSharedRef<SWidget> MakeFOVWidget() const;
	//~End: 骨骼树

	FText GetCameraModeEntryLable(FName Mode, bool bSkillBone = false) const;

	FText GetPlaybackMenuLabel() const;

	TSharedRef<SWidget> GeneratePlaybackMenu();
	/**
	 * Generates the toolbar show menu content
	 *
	 * @return The widget containing the show menu content
	 */
	TSharedRef<SWidget> GenerateShowMenu() const;

private:
	/* Pointer to our Ability Editor instance. */
	TWeakPtr<FNeAbilityBlueprintEditor> HostEditor;

	TWeakPtr<SNeAbilityEditorViewport> Viewport;

	TSharedPtr<FExtender> OptionsMenuExtender;

	/** Extenders */
	TArray<TSharedPtr<FExtender>> Extenders;
	
	/** Command list */
	TSharedPtr<FUICommandList> CommandList;
};