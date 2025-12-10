// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityEditorViewport.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityPreviewScene.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Application/SlateApplication.h"
#include "Editor/EditorEngine.h"
#include "Slate/SceneViewport.h"
#include "Engine/Selection.h"
#include "Editor/UnrealEdEngine.h"
#include "Widgets/SOverlay.h"
#include "Viewports.h"
#include "ShowFlagMenuCommands.h"
#include "Viewport/NeAbilityEditorViewportClient.h"
#include "Viewport/NeAbilityEditorViewportPlaybackCommands.h"
#include "Widgets/SNeActorLockedViewportToolbar.h"

#define LOCTEXT_NAMESPACE "NeAbilityEditorViewport"


SNeAbilityEditorViewport::~SNeAbilityEditorViewport()
{
	UEditorEngine* Editor = Cast<UEditorEngine>(GEngine);
	Editor->OnPreviewFeatureLevelChanged().Remove(PreviewFeatureLevelChangedHandle);
}

void SNeAbilityEditorViewport::Construct(const FArguments& InArgs, const FEdViewportArgs& InRequiredArgs)
{
	HostEditor = StaticCastSharedRef<FNeAbilityBlueprintEditor>(InRequiredArgs.AssetEditorToolkit);
	
	if (!CommandList)
	{
		CommandList = MakeShareable(new FUICommandList);
	}
	FNeAbilityEditorViewportPlaybackCommands::Register();
	SNeSimpleEdViewport::Construct
	(
		SNeSimpleEdViewport::FArguments()
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
		.AddMetaData<FTagMetaData>(TEXT("Able.AbilityEditor.Viewport")),
		InRequiredArgs
	);

	Client->VisibilityDelegate.BindSP(this, &SNeAbilityEditorViewport::IsVisible);

	// Restore last used feature level
	const TSharedPtr<FNeAbilityPreviewScene> PreviewScene = HostEditor.Pin()->GetAbilityPreviewScene();
	check(PreviewScene.IsValid());
	UWorld* World = PreviewScene->GetWorld();
	if (World != nullptr)
	{
		World->ChangeFeatureLevel(GWorld->GetFeatureLevel());
	}

	UEditorEngine* Editor = Cast<UEditorEngine>(GEngine);
	PreviewFeatureLevelChangedHandle = Editor->OnPreviewFeatureLevelChanged().AddLambda
	(
		[this](ERHIFeatureLevel::Type NewFeatureLevel)
		{
			if (HostEditor.Pin().IsValid())
			{
				// Local cache ptr, should not use outter variable
				TSharedPtr<FNeAbilityPreviewScene> PreviewScene = HostEditor.Pin()->GetAbilityPreviewScene();
				if (UWorld* World = PreviewScene->GetWorld())
				{
					World->ChangeFeatureLevel(GWorld->GetFeatureLevel());
				}
			}
		}
	);
	PreviewScene->OnCreateViewport(this, SceneViewport);
	
	// BindCommands();
	
}

void SNeAbilityEditorViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SNeSimpleEdViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

TSharedRef<FEditorViewportClient> SNeAbilityEditorViewport::MakeEditorViewportClient()
{
	const TSharedPtr<FNeAbilityEditorViewportClient> NewViewportClient = MakeShareable(
		new FNeAbilityEditorViewportClient(HostEditor.Pin(), HostEditor.Pin()->GetAbilityPreviewScene().Get(), SharedThis(this)));

	NewViewportClient->ViewportType = LVT_Perspective;
	NewViewportClient->bSetListenerPosition = false;
	NewViewportClient->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation);
	NewViewportClient->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation);
	NewViewportClient->SetRealtime(true);
	NewViewportClient->SetShowStats(true);
	ViewportClient = NewViewportClient;

	return NewViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SNeAbilityEditorViewport::MakeViewportToolbar()
{
	return SNew(SVerticalBox)
	.Visibility( EVisibility::SelfHitTestInvisible )
	+SVerticalBox::Slot()
	.AutoHeight()
	.VAlign(VAlign_Top)
	[
		SNew(SNeAbilityEditorViewportToolBar, HostEditor.Pin(), SharedThis(this))
			.Cursor(EMouseCursor::Default)
	]
	+SVerticalBox::Slot()
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	[
		SNew(SNeActorLockedViewportToolbar)
		.Viewport( SharedThis( this ) )
		.Visibility(this, &SNeAbilityEditorViewport::GetLockedIconVisibility)
	];
}

void SNeAbilityEditorViewport::BindCommands()
{
	SNeSimpleEdViewport::BindCommands();

	FUICommandList& CommandListRef = *CommandList;
	const FNeAbilityEditorViewportPlaybackCommands& ViewportPlaybackCommands = FNeAbilityEditorViewportPlaybackCommands::Get();

	FShowFlagMenuCommands::Get().BindCommands(*CommandList, Client);
	FBufferVisualizationMenuCommands::Get().BindCommands(*CommandList, Client);

	//Create a menu item for each playback speed in EAnimationPlaybackSpeeds
	for (int32 i = 0; i < int(ENeAbilityPlaybackSpeeds::NumPlaybackSpeeds); ++i)
	{
		CommandListRef.MapAction(
			ViewportPlaybackCommands.PlaybackSpeedCommands[i],
			FExecuteAction::CreateSP(this, &SNeAbilityEditorViewport::OnSetPlaybackSpeed, i),
			FCanExecuteAction(),
			FIsActionChecked::CreateSP(this, &SNeAbilityEditorViewport::IsPlaybackSpeedSelected, i));
	}
}

void SNeAbilityEditorViewport::OnFocusViewportToSelection()
{
	// TSharedPtr<FNeAbilityPreviewScene> PreviewScene = HostEditor.Pin()->GetAbilityPreviewScene();
	// if (PreviewScene != nullptr)
	// {
	// 	PreviewScene->OnFocusViewportToSelection();
	// }
}

void SNeAbilityEditorViewport::OnSetPlaybackSpeed(int32 PlaybackSpeedMode) const
{
	const TSharedRef<FNeAbilityEditorViewportClient> AbilityViewportClient = StaticCastSharedRef<FNeAbilityEditorViewportClient>(ViewportClient.ToSharedRef());
	AbilityViewportClient->SetPlaybackSpeedMode((ENeAbilityPlaybackSpeeds::Type)PlaybackSpeedMode);
}

bool SNeAbilityEditorViewport::IsPlaybackSpeedSelected(int32 PlaybackSpeedMode) const
{
	const TSharedRef<FNeAbilityEditorViewportClient> AbilityViewportClient = StaticCastSharedRef<FNeAbilityEditorViewportClient>(ViewportClient.ToSharedRef());
	return PlaybackSpeedMode == AbilityViewportClient->GetPlaybackSpeedMode();
}
#undef LOCTEXT_NAMESPACE