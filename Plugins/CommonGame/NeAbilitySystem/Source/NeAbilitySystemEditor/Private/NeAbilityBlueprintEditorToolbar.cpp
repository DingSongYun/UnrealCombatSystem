// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityBlueprintEditorToolbar.h"

#include "IDocumentation.h"
#include "NeAbility.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityEditorCommands.h"
#include "NeAbilityEditorPlayer.h"
#include "ToolMenu.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Layout/SSpacer.h"
#include "WorkflowOrientedApp/SModeWidget.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "NeAbility"

FNeAbilityBlueprintEditorToolbar::FNeAbilityBlueprintEditorToolbar(TSharedPtr<FNeAbilityBlueprintEditor>& InAbilityEditor)
{
	AbilityEditor = InAbilityEditor;
}

void FNeAbilityBlueprintEditorToolbar::AddEditorModesToolbar(TSharedPtr<FExtender> Extender)
{
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		AbilityEditor.Pin()->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
		{
			TSharedPtr<FNeAbilityBlueprintEditor> BlueprintEditorPtr = AbilityEditor.Pin();
			UBlueprint* BlueprintObj = AbilityEditor.Pin()->GetBlueprintObj();

			if( !BlueprintObj ||
				(!FBlueprintEditorUtils::IsLevelScriptBlueprint(BlueprintObj)
				&& !FBlueprintEditorUtils::IsInterfaceBlueprint(BlueprintObj)
				&& !BlueprintObj->bIsNewlyCreated)
				)
			{
				TAttribute<FName> GetActiveMode(BlueprintEditorPtr.ToSharedRef(), &FBlueprintEditor::GetCurrentMode);
				FOnModeChangeRequested SetActiveMode = FOnModeChangeRequested::CreateRaw(BlueprintEditorPtr.Get(), &FBlueprintEditor::SetCurrentMode);

				// Left side padding
				BlueprintEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));

				BlueprintEditorPtr->AddToolbarWidget(
					SNew(SModeWidget, FNeAbilityBlueprintEditorModes::GetLocalizedMode(FNeAbilityBlueprintEditorModes::TimelineMode), FNeAbilityBlueprintEditorModes::TimelineMode)
					.OnGetActiveMode(GetActiveMode)
					.OnSetActiveMode(SetActiveMode)
					.ToolTip(IDocumentation::Get()->CreateToolTip(
						LOCTEXT("TimelineModeButtonTooltip", "Switch to Blueprint Timeline Mode"),
						nullptr,
						TEXT("Shared/Editors/BlueprintEditor"),
						TEXT("DesignerMode")))
					.IconImage(FAppStyle::GetBrush("UMGEditor.SwitchToDesigner"))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("DesignerMode")))
				);

				BlueprintEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(10.0f, 1.0f)));

				BlueprintEditorPtr->AddToolbarWidget(
					SNew(SModeWidget, FNeAbilityBlueprintEditorModes::GetLocalizedMode(FNeAbilityBlueprintEditorModes::GraphMode), FNeAbilityBlueprintEditorModes::GraphMode)
					.OnGetActiveMode(GetActiveMode)
					.OnSetActiveMode(SetActiveMode)
					.CanBeSelected(BlueprintEditorPtr.Get(), &FBlueprintEditor::IsEditingSingleBlueprint)
					.ToolTip(IDocumentation::Get()->CreateToolTip(
						LOCTEXT("GraphModeButtonTooltip", "Switch to Graph Editing Mode"),
						nullptr,
						TEXT("Shared/Editors/BlueprintEditor"),
						TEXT("GraphMode")))
					.IconImage(FAppStyle::GetBrush("FullBlueprintEditor.SwitchToScriptingMode"))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("GraphMode")))
				);

				// Right side padding
				BlueprintEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(10.0f, 1.0f)));

				if (FNeAbilityBlueprintEditorModes::IsDebugModeEnabled())
				{
					BlueprintEditorPtr->AddToolbarWidget(
						SNew(SModeWidget, FNeAbilityBlueprintEditorModes::GetLocalizedMode(FNeAbilityBlueprintEditorModes::DebugMode), FNeAbilityBlueprintEditorModes::DebugMode)
						.OnGetActiveMode(GetActiveMode)
						.OnSetActiveMode(SetActiveMode)
						.CanBeSelected(BlueprintEditorPtr.Get(), &FBlueprintEditor::IsEditingSingleBlueprint)
						.ToolTip(IDocumentation::Get()->CreateToolTip(
							LOCTEXT("DebugModeButtonTooltip", "Switch to Debugging Mode"),
							nullptr,
							TEXT("Shared/Editors/BlueprintEditor"),
							TEXT("DebugMode")))
						.IconImage(FAppStyle::GetBrush("BlueprintDebugger.TabIcon"))
						.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("DebugMode")))
					);

					// Right side padding
					BlueprintEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(10.0f, 1.0f)));
				}
			}

		}));
}

void FNeAbilityBlueprintEditorToolbar::AddRepairToolbar(UToolMenu* InMenu)
{
	FToolMenuSection& Section = InMenu->FindOrAddSection("AssetTools");
	Section.InsertPosition = FToolMenuInsert("Asset", EToolMenuInsertType::After);

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"Repair",
		FUIAction(
			FExecuteAction::CreateRaw(this, &FNeAbilityBlueprintEditorToolbar::TryRepairAsset),
			FCanExecuteAction()
		)
		, LOCTEXT("RepairAsset", "Repair")
		, LOCTEXT("RepairAssetToolTip", "Fix asset error if exist.")
		, FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "WidgetReflector.Icon")
	));

}

void FNeAbilityBlueprintEditorToolbar::AddPlayControlToolbar(TSharedPtr<FExtender> Extender)
{
	Extender->AddToolBarExtension
	(
		"Asset",
		EExtensionHook::After,
		AbilityEditor.Pin()->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Preview");
			
			TSharedPtr<SButton> ForwardPlayButton = SNew(SButton)
				.OnClicked(this, &FNeAbilityBlueprintEditorToolbar::OnClick_PlayOrPause)
				.ToolTipText(LOCTEXT("ToPlay", "To Play"))
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(2.0f);
			ForwardPlayButton->SetContent(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SImage)
					.Image_Lambda([=, this] ()
					{
						TSharedPtr<class FNeAbilityEditorPlayerBase> PreviewPlayer = AbilityEditor.Pin()->GetAbilityPreviewPlayer();
						if (PreviewPlayer && PreviewPlayer->IsPlaying() && !PreviewPlayer->IsPaused())
						{
							return ForwardPlayButton.IsValid() && ForwardPlayButton->IsPressed() ?
								&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Animation.Pause").Pressed :
								&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Animation.Pause").Normal;
						}

						return ForwardPlayButton.IsValid() && ForwardPlayButton->IsPressed() ?
							&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Animation.Forward").Pressed :
							&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Animation.Forward").Normal;
					})
				]
				+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					.Padding(1.0f, 2.0f, 0.0f, 2.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PlayAbility", "Play "))
					]);

			TSharedPtr<SButton> ForwardStepPlayButton =
				SNew(SButton)
				.OnClicked(this, &FNeAbilityBlueprintEditorToolbar::OnClick_Step)
				.ToolTipText(LOCTEXT("ToNext", "To Next"))
				.ButtonStyle(FAppStyle::Get(), "SimpleButton");
			ForwardStepPlayButton->SetContent(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SImage)
					.Image_Lambda([=, this]()
					{
						return ForwardStepPlayButton.IsValid() && ForwardStepPlayButton->IsPressed() ?
							&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Animation.Forward_Step").Pressed :
							&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Animation.Forward_Step").Normal;
					})
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(1.0f, 1.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ToPlayAbility", "Step "))
				]);

			ToolbarBuilder.AddWidget(ForwardPlayButton.ToSharedRef());
			ToolbarBuilder.AddWidget(ForwardStepPlayButton.ToSharedRef());
			
			ToolbarBuilder.AddToolBarButton(FNeAbilityEditorCommands::Get().Reset, NAME_None, LOCTEXT("Toolbar_Reset", "Reset"),
				LOCTEXT("Toolbar_Reset_ToolTips", "Reset preview world."), FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"));

			ToolbarBuilder.EndSection();
		})
	);
}

void FNeAbilityBlueprintEditorToolbar::TryRepairAsset() const
{
	if (UNeAbility* Ability = AbilityEditor.Pin()->GetEditingAbility())
	{
		Ability->TryRepairAsset();
	}
}

FReply FNeAbilityBlueprintEditorToolbar::OnClick_PlayOrPause() const
{
	AbilityEditor.Pin()->PlayAbility();
	return FReply::Handled();
}

FReply FNeAbilityBlueprintEditorToolbar::OnClick_Step() const
{
	AbilityEditor.Pin()->StepAbility();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
