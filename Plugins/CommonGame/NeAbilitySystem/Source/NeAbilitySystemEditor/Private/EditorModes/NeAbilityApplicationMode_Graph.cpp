 // Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityApplicationMode_Graph.h"
#include "BlueprintEditorTabs.h"
#include "NeAbilityBlueprintEditor.h"
#include "SBlueprintEditorToolbar.h"

#define LOCTEXT_NAMESPACE "AbilityDesignerMode"

FAbilityApplicationMode_Graph::FAbilityApplicationMode_Graph(TSharedPtr<FNeAbilityBlueprintEditor> InAbilityEditor)
	: FNeAbilityEditorApplicationMode(InAbilityEditor, FNeAbilityBlueprintEditorModes::GraphMode)
{
	TSharedPtr<FNeAbilityBlueprintEditor> AbilityEditorPtr = MyBlueprintEditor.Pin();

	WorkspaceMenuCategory = FWorkspaceItem::NewGroup(LOCTEXT("WorkspaceMenu_AbilityGraph", "Ability Graph"));


	CreateModeTabs(AbilityEditorPtr.ToSharedRef(), TabFactories);

	TabLayout = FTabManager::NewLayout( "AbilityBlueprintEditor_Graph_v1_3" )
	->AddArea
	(
	FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
	->Split
		(
			FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.15f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.50f)
					->AddTab( FBlueprintEditorTabs::MyBlueprintID, ETabState::OpenedTab )
				)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation( Orient_Vertical )
				->SetSizeCoefficient(0.60f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient( 0.80f )
					->AddTab( "Document", ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient( 0.20f )
					// ->AddTab( FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab )
					->AddTab( FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab )
					->AddTab( FBlueprintEditorTabs::BookmarksID, ETabState::ClosedTab )
				)
			)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation( Orient_Vertical )
				->SetSizeCoefficient(0.25f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.60f)
					->AddTab( FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab )
					->AddTab( FBlueprintEditorTabs::PaletteID, ETabState::ClosedTab )
				)
			)
		)
	);

	if (UToolMenu* Toolbar = InAbilityEditor->RegisterModeToolbarIfUnregistered(GetModeName()))
	{
		InAbilityEditor->GetToolbarBuilder()->AddCompileToolbar(Toolbar);
		InAbilityEditor->GetToolbarBuilder()->AddScriptingToolbar(Toolbar);
		InAbilityEditor->GetToolbarBuilder()->AddBlueprintGlobalOptionsToolbar(Toolbar, false);
		InAbilityEditor->GetToolbarBuilder()->AddDebuggingToolbar(Toolbar);
	}

	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	// BlueprintEditorModule.OnRegisterTabsForEditor().Broadcast(BlueprintEditorTabFactories, ModeName, InAbilityEditor);

	LayoutExtender = MakeShared<FLayoutExtender>();
	BlueprintEditorModule.OnRegisterLayoutExtensions().Broadcast(*LayoutExtender);
}

void FAbilityApplicationMode_Graph::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FNeAbilityBlueprintEditor> BlueprintEditorPtr = GetBlueprintEditor();

	BlueprintEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());
	BlueprintEditorPtr->PushTabFactories(TabFactories);

	FNeAbilityEditorApplicationMode::RegisterTabFactories(InTabManager);
}

void FAbilityApplicationMode_Graph::PreDeactivateMode()
{
	FNeAbilityEditorApplicationMode::PreDeactivateMode();
}

void FAbilityApplicationMode_Graph::PostActivateMode()
{
	FNeAbilityEditorApplicationMode::PostActivateMode();
}

#undef LOCTEXT_NAMESPACE
