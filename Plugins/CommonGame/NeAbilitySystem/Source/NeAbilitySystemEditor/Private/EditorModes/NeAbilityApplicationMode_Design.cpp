 // Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityApplicationMode_Design.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityBlueprintEditorToolbar.h"
#include "ToolMenus.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "AbilityDesignerMode"

FAbilityApplicationMode_Design::FAbilityApplicationMode_Design(TSharedPtr<FNeAbilityBlueprintEditor> InAbilityEditor)
	: FNeAbilityEditorApplicationMode(InAbilityEditor, FNeAbilityBlueprintEditorModes::TimelineMode)
{
	TSharedPtr<FNeAbilityBlueprintEditor> AbilityEditorPtr = MyBlueprintEditor.Pin();

	WorkspaceMenuCategory = FWorkspaceItem::NewGroup(LOCTEXT("WorkspaceMenu_AbilityDesign", "Ability Design"));

	TabLayout = FTabManager::NewLayout("AbilityBlueprintEditor_Design_Layout_v1_4")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				// 左
				FTabManager::NewSplitter()
				->SetSizeCoefficient(1.f)
				->SetOrientation(Orient_Horizontal)

				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->SetHideTabWell(false)
					->AddTab(FNeAbilityBlueprintEditorTabs::AssetDetails, ETabState::OpenedTab)
				)

				// 中
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.6f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetHideTabWell(true)
						->AddTab(FNeAbilityBlueprintEditorTabs::Viewport, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetHideTabWell(true)
						->AddTab(FNeAbilityBlueprintEditorTabs::Timeline, ETabState::OpenedTab)
					)
				)
				// 右
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.2f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetHideTabWell(false)
						->AddTab(FNeAbilityBlueprintEditorTabs::Details, ETabState::OpenedTab)
						->AddTab(FNeAbilityBlueprintEditorTabs::Palette, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->SetHideTabWell(false)
						->AddTab(FNeAbilityBlueprintEditorTabs::PreviewSettings, ETabState::OpenedTab)
						->AddTab(FNeAbilityBlueprintEditorTabs::AssetBrowser, ETabState::OpenedTab)
					)
				)
			)
		);

	CreateModeTabs(AbilityEditorPtr.ToSharedRef(), TabFactories);

	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	// BlueprintEditorModule.OnRegisterTabsForEditor().Broadcast(BlueprintEditorTabFactories, ModeName, InAbilityEditor);
	LayoutExtender = MakeShared<FLayoutExtender>();
	BlueprintEditorModule.OnRegisterLayoutExtensions().Broadcast(*LayoutExtender);
	AbilityEditorPtr->RegisterModeToolbarIfUnregistered(GetModeName());

	InAbilityEditor->GetEditorToolbarBuilder()->AddPlayControlToolbar(ToolbarExtender);
	FName OutParentToolbarName;
	FName ToolBarname = AbilityEditorPtr->GetToolMenuToolbarNameForMode(GetModeName(), OutParentToolbarName);
	if (UToolMenu* Toolbar = UToolMenus::Get()->FindMenu(ToolBarname))
	{
		AbilityEditorPtr->GetEditorToolbarBuilder()->AddRepairToolbar(Toolbar);
	}
	// AbilityEditorPtr->GetEditorToolbarBuilder()->AddRepairToolbar(ToolbarExtender);
}

void FAbilityApplicationMode_Design::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FNeAbilityBlueprintEditor> BlueprintEditorPtr = GetBlueprintEditor();

	BlueprintEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());
	BlueprintEditorPtr->PushTabFactories(TabFactories);

	FNeAbilityEditorApplicationMode::RegisterTabFactories(InTabManager);
}

void FAbilityApplicationMode_Design::PreDeactivateMode()
{
	// FNeAbilityEditorApplicationMode::PreDeactivateMode();
}

void FAbilityApplicationMode_Design::PostActivateMode()
{
	// FNeAbilityEditorApplicationMode::PostActivateMode();
}

#undef LOCTEXT_NAMESPACE
