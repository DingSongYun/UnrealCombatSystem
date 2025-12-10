// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "SGraphPalette.h"
#include "GraphEditorDragDropAction.h"
#include "NeAbilityActionViewer.h"
#include "SNeAbilityEditorTab_Palette.generated.h"

class FGraphSchemaActionDragDropAction_AbilityPaletteItem : public FGraphSchemaActionDragDropAction
{
public:
	DRAG_DROP_OPERATOR_TYPE(FGraphSchemaActionDragDropAction_AbilityPaletteItem, FGraphSchemaActionDragDropAction)
		static TSharedRef<FGraphSchemaActionDragDropAction_AbilityPaletteItem> New(TSharedPtr<FEdGraphSchemaAction> InActionNode, const FNeAbilityActionViewItem& InAction)
	{
		TSharedRef<FGraphSchemaActionDragDropAction_AbilityPaletteItem> Operation = MakeShareable(new FGraphSchemaActionDragDropAction_AbilityPaletteItem);
		Operation->SourceAction = InActionNode;
		Operation->ActionItem = InAction;
		Operation->Construct();
		return Operation;
	}

	TSharedPtr<FEdGraphSchemaAction> GetAction() { return SourceAction; }

	UClass* GetAbilityActionClass() const { return ActionItem.GetActionClass(); }
public:
	FNeAbilityActionViewItem ActionItem;
};

USTRUCT()
struct FEdGraphSchemaAction_NewTask : public FEdGraphSchemaAction
{
	GENERATED_BODY()

public:
	FEdGraphSchemaAction_NewTask() : FEdGraphSchemaAction() {}

	FEdGraphSchemaAction_NewTask(const FNeAbilityActionViewItem& InItem, FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, FText InKeywords = FText(), int32 InSectionID = 0)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping, InKeywords, InSectionID), ActionItem(InItem)
	{}

	virtual FReply OnDoubleClick(UBlueprint* InBlueprint) override;

public:
	FNeAbilityActionViewItem ActionItem;
};

class SNeAbilityPaletteItem : public SGraphPaletteItem
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityPaletteItem) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData);
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

private:
	virtual FText GetItemTooltip() const override;
};

class SNeAbilityEditorTab_Palette : public SGraphPalette
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_Palette)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit);

	virtual TSharedRef<SWidget> OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData) override;

	virtual void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override;

	FReply OnActionDragged(const TArray< TSharedPtr<FEdGraphSchemaAction> >& InActions, const FPointerEvent& MouseEvent);

private:
	void AddAssetFromAssetRegistry(const FAssetData& InAddedAssetData);
	void RemoveAssetFromRegistry(const FAssetData& InAddedAssetData);
	void RenameAssetFromRegistry(const FAssetData& InAddedAssetData, const FString& InNewName);
	void RefreshAssetInRegistry(const FAssetData& InAddedAssetData);

private:
	TWeakPtr<FNeAbilityBlueprintEditor> AbilityEditor;

	TArray<FNeAbilityActionViewItem> AllPaletteAction;
};
