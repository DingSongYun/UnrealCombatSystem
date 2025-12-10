// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityBlueprintEditor.h"

#include "BlueprintEditorTabs.h"
#include "GameplayAbilityBlueprint.h"
#include "NeAbility.h"
#include "NeAbilityBlueprintEditorToolbar.h"
#include "NeAbilityConsoleVariables.h"
#include "NeAbilityEditorCommands.h"
#include "NeAbilityEditorDelegates.h"
#include "NeAbilityEditorPlayer.h"
#include "NeAbilityPreviewScene.h"
#include "NeAbilityPreviewSettings.h"
#include "NeAbilitySystemEditorModule.h"
#include "EditorModes/NeAbilityApplicationMode_Debug.h"
#include "EditorModes/NeAbilityApplicationMode_Design.h"
#include "EditorModes/NeAbilityApplicationMode_Graph.h"
#include "GameFramework/WorldSettings.h"
#include "TabFactory/SNeAbilityEditorTab_AssetDetails.h"
#include "TabFactory/SNeAbilityEditorTab_Details.h"
#include "TabFactory/SNeAbilityEditorTab_Viewport.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "Viewport/NeAbilityEditorViewportClient.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "AbilityBlueprintEditor"

const FName FNeAbilityBlueprintEditorModes::TimelineMode("AbilityTimeline");
const FName FNeAbilityBlueprintEditorModes::GraphMode("AbilityGraph");
const FName FNeAbilityBlueprintEditorModes::DebugMode("AbilityDebug");

FText FNeAbilityBlueprintEditorModes::GetLocalizedMode(const FName InMode)
{
	if (InMode == FNeAbilityBlueprintEditorModes::TimelineMode)
	{
		return NSLOCTEXT("AbilityBlueprintModes", "TimelineMode", "Timeline");
	}
	else if (InMode == FNeAbilityBlueprintEditorModes::GraphMode)
	{
		return NSLOCTEXT("AbilityBlueprintModes", "GraphMode", "Graph");
	}
	else if (InMode == FNeAbilityBlueprintEditorModes::DebugMode)
	{
		return NSLOCTEXT("AbilityBlueprintModes", "DebugMode", "Debug");
	}
	return FText::GetEmpty();
}

bool FNeAbilityBlueprintEditorModes::IsDebugModeEnabled()
{
	return AbilityConsoleVars::bEnableDebugMode;
}

const FName FNeAbilityBlueprintEditorTabs::Viewport("Ability Viewport");
const FName FNeAbilityBlueprintEditorTabs::Timeline("Ability Timeline");
const FName FNeAbilityBlueprintEditorTabs::AssetDetails("Ability AssetDetails");
const FName FNeAbilityBlueprintEditorTabs::Details("Ability Details");
const FName FNeAbilityBlueprintEditorTabs::Palette("Ability Palette");
const FName FNeAbilityBlueprintEditorTabs::DataBoard("Ability DataBoard");
const FName FNeAbilityBlueprintEditorTabs::AssetBrowser("Ability AssetBrowser");
const FName FNeAbilityBlueprintEditorTabs::PreviewSettings("Ability PreviewSettings");


FText FNeAbilityBlueprintEditorTabs::GetLocalizedTab(const FName InMode)
{
	if (InMode == Viewport)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "Viewport", "Viewport");
	}
	else if (InMode == Timeline)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "Timeline", "Timeline");
	}
	else if (InMode == AssetDetails)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "AssetDetails", "AssetDetails");
	}
	else if (InMode == Details)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "Details", "Details");
	}
	else if (InMode == Palette)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "Palette", "Palette");
	}
	else if (InMode == DataBoard)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "DataBoard", "DataBoard");
	}
	else if (InMode == AssetBrowser)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "AssetBrowser", "AssetBrowser");
	}
	else if (InMode == PreviewSettings)
	{
		return NSLOCTEXT("AbilityBlueprintTabs", "PreviewSettings", "PreviewSettings");
	}

	return FText::GetEmpty();
}

FNeAbilityBlueprintEditor::FNeAbilityBlueprintEditor()
{
}

FNeAbilityBlueprintEditor::~FNeAbilityBlueprintEditor()
{
}

void FNeAbilityBlueprintEditor::InitAbilityBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode)
{
	TSharedPtr<FNeAbilityBlueprintEditor> SharedThisPtr(SharedThis(this));

	const UBlueprint* BlueprintObj = InBlueprints.Num() > 0 ? InBlueprints[0] : nullptr;
	const UNeAbility* InAbility = BlueprintObj ? Cast<UNeAbility>(BlueprintObj->GeneratedClass->GetDefaultObject()) : nullptr;
	bTimelineBasedAbility = InAbility && InAbility->IsTimelineBasedAbility();

	PreviewSettings = GetMutableDefault<UNeAbilityPreviewSettings>(UNeAbilityPreviewSettings::StaticClass());
	check(PreviewSettings.Get());

	EditorToolbar = MakeShared<FNeAbilityBlueprintEditorToolbar>(SharedThisPtr);

	BindToolkitCommands();

	// Create preview scene
	if (IsTimelineBasedAbility())
	{
		CreatePreviewScene();
		PreviewScene->GetWorld()->bAllowAudioPlayback = false;
	}

	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);

	InitPreviewContext();

	if (TSharedPtr<SDockTab> CompilerResultTab = TabManager->FindExistingLiveTab(FBlueprintEditorTabs::CompilerResultsID); CompilerResultTab.IsValid())
	{
		CompilerResultTab->RequestCloseTab();
	}
}

void FNeAbilityBlueprintEditor::RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated)
{
	if ( InBlueprints.Num() == 1 )
	{
		TSharedPtr<FNeAbilityBlueprintEditor> SharedThisPtr(SharedThis(this));

		TArray< TSharedRef<FApplicationMode> > ModeList;

		const bool bTimelineBased = IsTimelineBasedAbility();
		if (bTimelineBased)
		{
			ModeList.Add(MakeShared<FAbilityApplicationMode_Design>(SharedThisPtr));
		}
		ModeList.Add(MakeShared<FAbilityApplicationMode_Graph>(SharedThisPtr));

		if (FNeAbilityBlueprintEditorModes::IsDebugModeEnabled())
		{
			ModeList.Add(MakeShared<FAbilityApplicationMode_Debug>(SharedThisPtr));
		}

		for ( const TSharedRef<FApplicationMode>& Mode : ModeList )
		{
			AddApplicationMode(Mode->GetModeName(), Mode);
		}

		SetCurrentMode(bTimelineBased ? FNeAbilityBlueprintEditorModes::TimelineMode : FNeAbilityBlueprintEditorModes::GraphMode);
	}
}

FGraphAppearanceInfo FNeAbilityBlueprintEditor::GetGraphAppearance(UEdGraph* InGraph) const
{
	FGraphAppearanceInfo AppearanceInfo = Super::GetGraphAppearance(InGraph);

	if ( GetBlueprintObj()->IsA(UGameplayAbilityBlueprint::StaticClass()) )
	{
		AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Ability BLUEPRINT");
	}

	return AppearanceInfo;
}

UBlueprint* FNeAbilityBlueprintEditor::GetBlueprintObj() const
{
	return FBlueprintEditor::GetBlueprintObj();
}

void FNeAbilityBlueprintEditor::InitalizeExtenders()
{
	Super::InitalizeExtenders();

	INeAbiitySystemEditorModule& AbilityEditorModule = INeAbiitySystemEditorModule::Get();
	AddMenuExtender(AbilityEditorModule.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
}

void FNeAbilityBlueprintEditor::CreateDefaultTabContents(const TArray<UBlueprint*>& InBlueprints)
{
	TSharedPtr<FNeAbilityBlueprintEditor> SharedThisPtr(SharedThis(this));

	const UNeAbility* EditingAbility = GetEditingAbility();
	if (IsTimelineBasedAbility())
	{
		// Create Asset Details tab body
		TabAbilityDefaults = SNew(SNeAbilityEditorTab_AssetDetails, SharedThisPtr)
					. OnFinishedChangingProperties( FOnFinishedChangingProperties::FDelegate::CreateSP( this, &FNeAbilityBlueprintEditor::OnFinishedChangingProperties ) );

		// Create Details tab body
		TabDetails = SNew(SNeAbilityEditorTab_Details, SharedThisPtr);

		// Create Viewport tab body
		TabViewport = SNew(SNeAbilityEditorTab_Viewport, SharedThisPtr.ToSharedRef(), GetAbilityPreviewScene().ToSharedRef(), 0);
	}

	Super::CreateDefaultTabContents(InBlueprints);
}

UGameplayAbilityBlueprint* FNeAbilityBlueprintEditor::GetAbilityBlueprintObj() const
{
	return Cast<UGameplayAbilityBlueprint>(GetBlueprintObj());
}

UNeAbility* FNeAbilityBlueprintEditor::GetEditingAbility() const
{
	return Cast<UNeAbility>(GetBlueprintObj()->GeneratedClass->GetDefaultObject());
}

bool FNeAbilityBlueprintEditor::IsTimelineBasedAbility() const
{
	return bTimelineBasedAbility;
}

TSharedPtr<FNeAbilityEditorViewportClient> FNeAbilityBlueprintEditor::GetPreviewViewportClient() const
{
	if (!TabViewport.IsValid())
		return nullptr;

	return StaticCastSharedPtr<FNeAbilityEditorViewportClient>(TabViewport->GetViewportClinet());
}

UWorld* FNeAbilityBlueprintEditor::GetPreviewWorld() const
{
	check(PreviewScene.IsValid());
	return PreviewScene->GetWorld();
}

void FNeAbilityBlueprintEditor::ResetPreviewWorld() const
{
	check(PreviewScene.IsValid());
	if (!PreviewScene.IsValid()) return ;

	PreviewScene->ResetPreviewWorld();

	PreviewPlayer->OnResetPreviewWorld();
}

bool FNeAbilityBlueprintEditor::IsWorldPaused() const
{
	if (PreviewPlayer->IsPaused())
	{
		return true;
	}
	return false;
}

void FNeAbilityBlueprintEditor::ShowInDetailsView(const TSharedPtr<FStructOnScope>& InStruct)
{
	TabDetails->ShowSingleStruct(InStruct);
}

void FNeAbilityBlueprintEditor::ShowInDetailsView(const TArray<UObject*>& InObjects)
{
	TabDetails->ShowDetailsForObjects(InObjects);
}

void FNeAbilityBlueprintEditor::PlayAbility()
{
	if (PreviewPlayer->IsPlaying())
	{
		PreviewPlayer->Pause();
	}
	else if (PreviewPlayer->IsPaused())
	{
		PreviewPlayer->Resume();
	}
	else
	{
		PreviewPlayer->Play();
		FNeAbilityEditorDelegates::PlayForPreviewDelegate.Broadcast();
	}
}

void FNeAbilityBlueprintEditor::PauseAbility()
{
	if (PreviewPlayer->IsPaused())
	{
		PreviewPlayer->Resume();
	}
	else
	{
		PreviewPlayer->Pause();
	}
}

void FNeAbilityBlueprintEditor::StepAbility() const
{
	if (PreviewPlayer->IsStopped())
	{
		PreviewPlayer->Play();
		PreviewPlayer->Pause();
	}
	else if (PreviewPlayer->IsPlaying())
	{
		PreviewPlayer->Pause();
	}
	const TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = GetPreviewViewportClient();
	if (ViewportClient.IsValid())
	{
		ViewportClient->TickWorld(PreviewSettings->GetFrameRate().AsInterval());
	}
}

void FNeAbilityBlueprintEditor::PlaySection(int32 Index) const
{
	if (PreviewPlayer->IsPlaying())
	{
		PreviewPlayer->Pause();
	}
	else if (PreviewPlayer->IsPaused())
	{
		PreviewPlayer->Resume();
	}
	else
	{
		PreviewPlayer->PlaySection(Index);
		FNeAbilityEditorDelegates::PlayForPreviewDelegate.Broadcast();
	}
}

void FNeAbilityBlueprintEditor::PauseSection(int32 Index)
{
	if (PreviewPlayer->IsPaused())
	{
		PreviewPlayer->Resume();
	}
	else
	{
		PreviewPlayer->Pause();
	}
}

bool FNeAbilityBlueprintEditor::IsPlayingSection(int32 Index) const
{
	return PreviewPlayer->IsPlayingSection(Index);
}

void FNeAbilityBlueprintEditor::BindToolkitCommands()
{
	// 注册指令
	FNeAbilityEditorCommands::Register();

	const TSharedRef<FUICommandList>& TKCommands = GetToolkitCommands();

	TKCommands->MapAction(FNeAbilityEditorCommands::Get().Reset, FExecuteAction::CreateSP(this, &FNeAbilityBlueprintEditor::ResetPreviewWorld));
	TKCommands->MapAction(FNeAbilityEditorCommands::Get().Step, FExecuteAction::CreateSP(this, &FNeAbilityBlueprintEditor::StepAbility));
	// ToolkitCommands->MapAction(FNeAbilityEditorCommands::Get().ToggleCollision, FExecuteAction::CreateSP(this, &FNeAbilityBlueprintEditor::ResetPreviewWorld));
}

void FNeAbilityBlueprintEditor::CreatePreviewScene()
{
	if (!PreviewScene.IsValid())
	{
		PreviewScene = MakeShareable(
			new FNeAbilityPreviewScene(FPreviewScene::ConstructionValues()
				.ForceUseMovementComponentInNonGameWorld(true)
				.AllowAudioPlayback(true)
				.ShouldSimulatePhysics(true)
				.SetEditor(true)
				, SharedThis(this)
			)
		);
		PreviewScene->GetWorld()->GetWorldSettings()->SetIsTemporarilyHiddenInEditor(false);
	}
}

void FNeAbilityBlueprintEditor::InitPreviewContext()
{
	if (!PreviewScene.IsValid()) return ;

	PreviewScene->InitPreviewWorld();

	CreatePreviewProxy();

	PreviewScene->PostInitPreviewWorld();
}

void FNeAbilityBlueprintEditor::CreatePreviewProxy()
{
	PreviewPlayer = FNeAbilityEditorPlayerBase::Create( GetEditingAbility(), SharedThis(this) );
}

#undef LOCTEXT_NAMESPACE
