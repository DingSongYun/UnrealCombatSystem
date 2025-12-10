// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Viewport/SNeSimpleEdViewport.h"
#include "AdvancedPreviewScene.h"
#include "LevelViewportActions.h"
#include "PreviewScene.h"
#include "SViewportToolBar.h"
#include "Viewports.h"
#include "Framework/Application/SlateApplication.h"

/*********************************************************/
// SNeSimpleEdViewport
// Viewport Widget
/*********************************************************/
void SNeSimpleEdViewport::Construct(const FArguments& InArgs, const FEdViewportArgs& InRequiredArgs)
{
	PreviewScenePtr = InRequiredArgs.PreviewScene;
	AssetEditorToolkitPtr = InRequiredArgs.AssetEditorToolkit;
	ViewportIndex = InRequiredArgs.ViewportIndex;

	SEditorViewport::Construct(
		SEditorViewport::FArguments()
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
		.AddMetaData<FTagMetaData>(TEXT("ProjectC.Viewport"))
	);

	if (Client)
	{
		Client->VisibilityDelegate.BindSP(this, &SNeSimpleEdViewport::IsVisible);
	}

	// restore last used feature level
	auto ScenePtr = PreviewScenePtr.Pin();
	if (ScenePtr.IsValid())
	{
		UWorld* World = ScenePtr->GetWorld();
		if (World != nullptr)
		{
			World->ChangeFeatureLevel(GWorld->FeatureLevel);
		}
	}

	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	PreviewFeatureLevelChangedHandle = Editor->OnPreviewFeatureLevelChanged().AddLambda([this](ERHIFeatureLevel::Type NewFeatureLevel)
		{
			auto ScenePtr = PreviewScenePtr.Pin();
			if (ScenePtr.IsValid())
			{
				UWorld* World = ScenePtr->GetWorld();
				if (World != nullptr)
				{
					World->ChangeFeatureLevel(NewFeatureLevel);
				}
			}
		});
}

TSharedRef<FEditorViewportClient> SNeSimpleEdViewport::MakeEditorViewportClient()
{
	// Create viewport client
	ViewportClient = MakeShareable(new FNeSimpleEdViewportClient(nullptr, PreviewScenePtr.Pin().Get(), SharedThis(this)));
	ViewportClient->ViewportType = LVT_Perspective;
	ViewportClient->bSetListenerPosition = false;
	ViewportClient->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation);
	ViewportClient->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation);

	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SNeSimpleEdViewport::MakeViewportToolbar()
{
	return SAssignNew(ViewportToolbar, SViewportToolBar);
}

void SNeSimpleEdViewport::PostUndo(bool bSuccess)
{
	ViewportClient->Invalidate();
}

void SNeSimpleEdViewport::PostRedo(bool bSuccess)
{
	ViewportClient->Invalidate();
}

void SNeSimpleEdViewport::OnFocusViewportToSelection()
{
}

void SNeSimpleEdViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	FUICommandList& CommandListRef = *CommandList;
	const FLevelViewportCommands& LevelViewportActions = FLevelViewportCommands::Get();
	CommandListRef.MapAction( 
		LevelViewportActions.ToggleGameView,
		FExecuteAction::CreateSP( this, &SNeSimpleEdViewport::ToggleGameView),
		FCanExecuteAction::CreateSP(this, &SNeSimpleEdViewport::CanToggleGameView),
		FIsActionChecked::CreateSP(this, &SNeSimpleEdViewport::IsInGameView));
	CommandListRef.MapAction(
		LevelViewportActions.EjectActorPilot,
		FExecuteAction::CreateSP( this, &SNeSimpleEdViewport::OnActorUnlock ),
		FCanExecuteAction::CreateSP( this, &SNeSimpleEdViewport::CanExecuteActorUnlock ));
}

UWorld* SNeSimpleEdViewport::GetWorld() const
{
	if (PreviewScenePtr.Pin())
	{
		return PreviewScenePtr.Pin()->GetWorld();
	}

	return nullptr;
}

TSharedPtr<SOverlay> SNeSimpleEdViewport::GetViewportOverlay()
{
	return ViewportOverlay;
}

void SNeSimpleEdViewport::ToggleGameView()
{
	if( ViewportClient->IsPerspective() )
	{
		bool bGameViewEnable = !ViewportClient->IsInGameView();

		ViewportClient->SetGameView(bGameViewEnable);
	}
}

bool SNeSimpleEdViewport::CanToggleGameView() const
{
	return ViewportClient->IsPerspective();
}

bool SNeSimpleEdViewport::IsInGameView() const
{
	return ViewportClient->IsInGameView();
}

TSharedPtr<FNeSimpleEdViewportClient> SNeSimpleEdViewport::GetSimpleViewportClient() const
{
	return StaticCastSharedPtr<FNeSimpleEdViewportClient>(ViewportClient);
}

void SNeSimpleEdViewport::OnActorUnlock()
{
	if (ViewportClient->GetLockedActor() != nullptr)
	{
		ViewportClient->SetLockedActor(nullptr);
		ViewportClient->ViewFOV = ViewportClient->FOVAngle;
	}
}

bool SNeSimpleEdViewport::CanExecuteActorUnlock() const
{
	return ViewportClient->GetLockedActor() != nullptr;
}

EVisibility SNeSimpleEdViewport::GetLockedIconVisibility() const
{
	return ViewportClient->GetLockedActor() != nullptr ? EVisibility::Visible : EVisibility::Collapsed;
}

