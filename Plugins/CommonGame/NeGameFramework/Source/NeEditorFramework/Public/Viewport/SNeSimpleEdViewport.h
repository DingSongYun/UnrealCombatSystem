// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "EditorUndoClient.h"
#include "Viewport/NeSimpleEdViewportClient.h"

/*********************************************************/
// FEdViewportArgs
/*********************************************************/
struct NEEDITORFRAMEWORK_API FEdViewportArgs
{
public:
	FEdViewportArgs(
		TSharedRef<class FAdvancedPreviewScene> InPreviewScene,
		TSharedRef<class FAssetEditorToolkit> InAssetEditorToolkit,
		int32 InViewportIndex
	)
	: PreviewScene(InPreviewScene)
	, AssetEditorToolkit(InAssetEditorToolkit), ViewportIndex(InViewportIndex)
	{}
	
	TSharedRef<class FAdvancedPreviewScene> PreviewScene;
	TSharedRef<class FAssetEditorToolkit> AssetEditorToolkit;
	int32 ViewportIndex;
};

/*********************************************************/
// SNeSimpleEdViewport
// Viewport Widget
/*********************************************************/
class NEEDITORFRAMEWORK_API SNeSimpleEdViewport : public SEditorViewport, public FEditorUndoClient
{
public:
	SLATE_BEGIN_ARGS( SNeSimpleEdViewport ) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FEdViewportArgs& InRequiredArgs);
		
	// Begin: SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual void OnFocusViewportToSelection() override;
	virtual void BindCommands() override;
	// End: End of SEditorViewport interface

	// Begin: FEditorUndoClient interface
	virtual void PostUndo( bool bSuccess );
	virtual void PostRedo( bool bSuccess );
	// End: FEditorUndoClient interface

	virtual UWorld* GetWorld() const override;

	TSharedPtr<SOverlay> GetViewportOverlay();
	TSharedPtr<class FNeSimpleEdViewportClient> GetSimpleViewportClient() const;

protected:
	void ToggleGameView();
	bool CanToggleGameView() const;
	bool IsInGameView() const;

	void OnActorUnlock();
	bool CanExecuteActorUnlock() const;
	EVisibility GetLockedIconVisibility() const;

protected:
	// Viewport client
	TSharedPtr<class FNeSimpleEdViewportClient> ViewportClient;

	// Viewport toolbar
	TSharedPtr<class SViewportToolBar> ViewportToolbar;

	// The preview scene that we are viewing
	TWeakPtr<class FPreviewScene> PreviewScenePtr;

	// The asset editor we are embedded in
	TWeakPtr<class FAssetEditorToolkit> AssetEditorToolkitPtr;

	/** Viewport index (0-3) */
	int32 ViewportIndex;

	// Handle to the registered OnPreviewFeatureLevelChanged delegate.
	FDelegateHandle PreviewFeatureLevelChangedHandle;
};
