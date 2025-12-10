// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityEditorTab_Palette.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "IAssetTools.h"
#include "NeAbilityActionViewer.h"
#include "Subsystems/EditorAssetSubsystem.h"

#define LOCTEXT_NAMESPACE "SNeAbilityEditorTab_Palette"

FReply FEdGraphSchemaAction_NewTask::OnDoubleClick(UBlueprint* InBlueprint)
{
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.OpenEditorForAssets({ActionItem.GetActionClass()});
	return FReply::Handled();
}

void SNeAbilityPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData)
{
	FSlateFontInfo NameFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);

	check(InCreateData->Action.IsValid());

	TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
	ActionPtr = InCreateData->Action;

	const FSlateBrush* IconBrush = FAppStyle::GetBrush(TEXT("NoBrush"));
	FSlateColor IconColor = FSlateColor::UseForeground();
	FText IconToolTip = GraphAction->GetTooltipDescription();
	bool bIsReadOnly = false;

	TSharedRef<SWidget> IconWidget = CreateIconWidget(IconToolTip, IconBrush, IconColor);
	TSharedRef<SWidget> NameSlotWidget = CreateTextSlotWidget(InCreateData, bIsReadOnly);

	// 创建子控件
	this->ChildSlot
	[
		SNew(SHorizontalBox)
		// Icon Slot
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			IconWidget
		]
		// Name Slot
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(3, 0)
		[
			NameSlotWidget
		]
	];
}

FReply SNeAbilityPaletteItem::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	TSharedPtr<FEdGraphSchemaAction_NewTask> NewTaskActionPtr = StaticCastSharedPtr<FEdGraphSchemaAction_NewTask>(ActionPtr.Pin());
	if (NewTaskActionPtr.IsValid())
	{
		const UPackage* Package = NewTaskActionPtr->ActionItem.GetActionClass()->GetPackage();
		UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
		if (UObject* AssetObject = EditorAssetSubsystem->LoadAsset(Package->GetPathName()))
		{
			GEditor->EditObject( AssetObject );
		}
	}

	return SGraphPaletteItem::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FText SNeAbilityPaletteItem::GetItemTooltip() const
{
	return ActionPtr.Pin()->GetTooltipDescription();
}


void SNeAbilityEditorTab_Palette::Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit)
{
	AbilityEditor = InAssetEditorToolkit;
	// FEditorWidgetsModule& EditorWidgetsModule = FModuleManager::LoadModuleChecked<FEditorWidgetsModule>("EditorWidgets");

	this->ChildSlot
	[
		SNew(SBorder)
		.Padding(2.0f)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(GraphActionMenu, SGraphActionMenu)
					.OnActionDragged(this, &SNeAbilityEditorTab_Palette::OnActionDragged)
					.OnCreateWidgetForAction(this, &SNeAbilityEditorTab_Palette::OnCreateWidgetForAction)
					.OnCollectAllActions(this, &SNeAbilityEditorTab_Palette::CollectAllActions)
					.AutoExpandActionMenu(true)
				]
			]
		]
	];

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnAssetAdded().AddSP(this, &SNeAbilityEditorTab_Palette::AddAssetFromAssetRegistry);
	AssetRegistryModule.Get().OnAssetRemoved().AddSP(this, &SNeAbilityEditorTab_Palette::RemoveAssetFromRegistry);
	AssetRegistryModule.Get().OnAssetRenamed().AddSP(this, &SNeAbilityEditorTab_Palette::RenameAssetFromRegistry);
}

TSharedRef<SWidget> SNeAbilityEditorTab_Palette::OnCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData)
{
	return SNew(SNeAbilityPaletteItem, InCreateData);
}

void SNeAbilityEditorTab_Palette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	FGraphActionMenuBuilder ActionMenuBuilder;
	AllPaletteAction = FNeAbilityActionViewer::GetAllAbilityActions();
	for (const auto& Action : AllPaletteAction)
	{
		FString Category = Action.GetCategory();
		if (Category.IsEmpty()) Category = "Default";
		TSharedPtr<FEdGraphSchemaAction_NewTask> NewAction = TSharedPtr<FEdGraphSchemaAction_NewTask>(new FEdGraphSchemaAction_NewTask(Action,
			FText::FromString(Category), Action.GetDisplayNameText(), Action.GetToolTips(), 0));
		ActionMenuBuilder.AddAction(NewAction);
	}
	OutAllActions.Append(ActionMenuBuilder);
}

FReply SNeAbilityEditorTab_Palette::OnActionDragged(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions, const FPointerEvent& MouseEvent)
{
	if (InActions.Num() > 0 && InActions[0].IsValid())
	{
		TSharedPtr<FEdGraphSchemaAction> InAction = InActions[0];

		for (const auto& Action : AllPaletteAction)
		{
			if (Action.GetDisplayNameText().ToString() == InAction->GetMenuDescription().ToString())
			{
				return FReply::Handled().BeginDragDrop(FGraphSchemaActionDragDropAction_AbilityPaletteItem::New(InAction, Action));
			}
		}
	}

	return FReply::Unhandled();
}

void SNeAbilityEditorTab_Palette::AddAssetFromAssetRegistry(const FAssetData& InAddedAssetData)
{
	RefreshAssetInRegistry(InAddedAssetData);
}

void SNeAbilityEditorTab_Palette::RemoveAssetFromRegistry(const FAssetData& InAddedAssetData)
{
	RefreshAssetInRegistry(InAddedAssetData);
}

void SNeAbilityEditorTab_Palette::RenameAssetFromRegistry(const FAssetData& InAddedAssetData, const FString& InNewName)
{
	RefreshAssetInRegistry(InAddedAssetData);
}

void SNeAbilityEditorTab_Palette::RefreshAssetInRegistry(const FAssetData& InAddedAssetData)
{

}



#undef LOCTEXT_NAMESPACE
