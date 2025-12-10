// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "Widgets/SCompoundWidget.h"
#include "IContentBrowserSingleton.h"
#include "Editor.h"
#include "NeAbility.h"

//=============================================================================
/**
 * SNeAbilityEditorTab_AssetBrowser
 */
class SNeAbilityEditorTab_AssetBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_AssetBrowser)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		FAssetPickerConfig Config;
		FARFilter Filter;
		Filter.bRecursiveClasses = true;
		Filter.ClassPaths.Add(UNeAbility::StaticClass()->GetClassPathName());
		Config.Filter = Filter;
		Config.InitialAssetViewType = EAssetViewType::Column;
		Config.bAddFilterUI = false;
		Config.bShowPathInColumnView = false;
		Config.bSortByPathInColumnView = false;
		Config.bShowTypeInColumnView = false;
		Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SNeAbilityEditorTab_AssetBrowser::OnRequestOpenAsset);
		Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SNeAbilityEditorTab_AssetBrowser::OnGetAssetContextMenu);
		Config.bFocusSearchBoxWhenOpened = false;

		this->ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(Config)
			]
		];
	}

	void OnRequestOpenAsset(const FAssetData& AssetData)
	{
		if (UObject* ObjectToEdit = AssetData.GetAsset())
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ObjectToEdit);
		}
	}

	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
	{
		return SNullWidget::NullWidget;
	}
};