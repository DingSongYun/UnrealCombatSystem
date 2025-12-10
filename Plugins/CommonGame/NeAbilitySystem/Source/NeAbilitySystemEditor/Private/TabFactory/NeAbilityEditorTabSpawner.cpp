// Copyright NetEase Games, Inc. All Rights Reserved.

#include "AdvancedPreviewSceneModule.h"
#include "NeAbilityBlueprintEditor.h"
#include "SNeAbilityEditorTab_AssetBrowser.h"
#include "SNeAbilityEditorTab_DataBoard.h"
#include "SNeAbilityEditorTab_Palette.h"
#include "SNeAbilityEditorTab_PreviewSettings.h"
#include "SNeAbilityEditorTab_Timeline.h"
#include "EditorModes/NeAbilityApplicationMode_Debug.h"
#include "EditorModes/NeAbilityApplicationMode_Design.h"
#include "EditorModes/NeAbilityApplicationMode_Graph.h"
#include "TabFactory/SNeAbilityEditorTab_AssetDetails.h"
#include "TabFactory/SNeAbilityEditorTab_Details.h"
#include "TabFactory/SNeAbilityEditorTab_Viewport.h"


#define DECLARE_TAB_SUMMONER_BEGIN(SummonerName, TabIdentify) \
struct SummonerName : public FWorkflowTabFactory \
{ \
public: \
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) \
	{ \
		return MakeShareable(new SummonerName(InHostingApp)); \
	} \
public: \
	SummonerName(TSharedPtr<class FAssetEditorToolkit> InHostingApp) \
		: FWorkflowTabFactory(TabIdentify, InHostingApp) \
	{\
		TabLabel = FNeAbilityBlueprintEditorTabs::GetLocalizedTab(TabIdentify);\
	} \
	template<typename T>  TSharedPtr<T> GetHostingEditor() const \
	{\
		return StaticCastSharedPtr<T>(HostingApp.Pin());\
	}

#define DECLARE_TAB_SUMMONER_END };

//=============================================================================
/**
 * FEditorViewportSummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FEditorViewportSummoner, FNeAbilityBlueprintEditorTabs::Viewport)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetHostingEditor<FNeAbilityBlueprintEditor>()->TabViewport.ToSharedRef();
}

// virtual FTabSpawnerEntry& RegisterTabSpawner(TSharedRef<FTabManager> InTabManager, const FApplicationMode* CurrentApplicationMode) const
// {
// 	FWorkflowTabSpawnInfo SpawnInfo;
// 	SpawnInfo.TabManager = InTabManager;
//
// 	TWeakPtr<FTabManager> WeakTabManager(InTabManager);
// 	FTabSpawnerEntry& SpawnerEntry = InTabManager->RegisterTabSpawner(GetIdentifier(), FOnSpawnTab::CreateSP(this, &FEditorViewportSummoner::OnSpawnTab, WeakTabManager), FCanSpawnTab::CreateSP(this, &FEditorViewportSummoner::CanSpawnTab, WeakTabManager))
// 		.SetDisplayName(ConstructTabName(SpawnInfo).Get())
// 		.SetTooltipText(GetTabToolTipText(SpawnInfo));
//
// 	if (const FGBSEditorMode_Main* TmpMode = static_cast<const FGBSEditorMode_Main*>(CurrentApplicationMode))
// 	{
// 		SpawnerEntry.SetGroup(TmpMode->ViewportMenuCategory.ToSharedRef());
// 	}
//
// 	// Add the tab icon to the menu entry if one was provided
// 	const FSlateIcon& TabSpawnerIcon = GetTabSpawnerIcon(SpawnInfo);
// 	if (TabSpawnerIcon.IsSet())
// 	{
// 		SpawnerEntry.SetIcon(TabSpawnerIcon);
// 	}
//
// 	return SpawnerEntry;
//
// }
DECLARE_TAB_SUMMONER_END

//=============================================================================
/**
 * FAssetTimelineTabSummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FAssetTimelineTabSummoner, FNeAbilityBlueprintEditorTabs::Timeline)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SNeAbilityEditorTab_Timeline, GetHostingEditor<FNeAbilityBlueprintEditor>());
}
DECLARE_TAB_SUMMONER_END

//=============================================================================
/**
 * FAssetBrowserSummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FAssetBrowserSummoner, FNeAbilityBlueprintEditorTabs::AssetBrowser)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SNeAbilityEditorTab_AssetBrowser);
}
DECLARE_TAB_SUMMONER_END

//=============================================================================
/**
 * FAssetDetailsSummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FAssetDetailsSummoner, FNeAbilityBlueprintEditorTabs::AssetDetails)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetHostingEditor<FNeAbilityBlueprintEditor>()->TabAbilityDefaults.ToSharedRef();
}
DECLARE_TAB_SUMMONER_END

//=============================================================================
/**
 * FAbilityPaletteSummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FAbilityPaletteSummoner, FNeAbilityBlueprintEditorTabs::Palette)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SNeAbilityEditorTab_Palette, GetHostingEditor<FNeAbilityBlueprintEditor>());
}
DECLARE_TAB_SUMMONER_END

//=============================================================================
/**
 * FAbilityPaletteSummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FAbilityDataBoardSummoner, FNeAbilityBlueprintEditorTabs::DataBoard)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SNeAbilityEditorTab_DataBoard, GetHostingEditor<FNeAbilityBlueprintEditor>());
}
DECLARE_TAB_SUMMONER_END

//=============================================================================
/**
 * FDetailsTabSummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FDetailsTabSummoner, FNeAbilityBlueprintEditorTabs::Details)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetHostingEditor<FNeAbilityBlueprintEditor>()->TabDetails.ToSharedRef();
}
DECLARE_TAB_SUMMONER_END

//=============================================================================
/**
 * FPreviewSettingsummoner
 */
DECLARE_TAB_SUMMONER_BEGIN(FPreviewSettingSummoner, FNeAbilityBlueprintEditorTabs::PreviewSettings)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FNeAbilityBlueprintEditor> AbilityEditor = GetHostingEditor<FNeAbilityBlueprintEditor>();

	return SNew(SNeAbilityEditorTab_PreviewSettings, AbilityEditor );
}
DECLARE_TAB_SUMMONER_END

void FAbilityApplicationMode_Design::CreateModeTabs(TSharedRef<FNeAbilityBlueprintEditor> AbilityEditor, FWorkflowAllowedTabSet& OutTabFactories) const
{
	OutTabFactories.RegisterFactory(FEditorViewportSummoner::Create(AbilityEditor));
	OutTabFactories.RegisterFactory(FAssetTimelineTabSummoner::Create(AbilityEditor));
	OutTabFactories.RegisterFactory(FAssetBrowserSummoner::Create(AbilityEditor));
	OutTabFactories.RegisterFactory(FAssetDetailsSummoner::Create(AbilityEditor));
	OutTabFactories.RegisterFactory(FDetailsTabSummoner::Create(AbilityEditor));
	OutTabFactories.RegisterFactory(FAbilityPaletteSummoner::Create(AbilityEditor));
	OutTabFactories.RegisterFactory(FAbilityDataBoardSummoner::Create(AbilityEditor));
	OutTabFactories.RegisterFactory(FPreviewSettingSummoner::Create(AbilityEditor));
}

void FAbilityApplicationMode_Graph::CreateModeTabs(TSharedRef<FNeAbilityBlueprintEditor> AbilityEditor, FWorkflowAllowedTabSet& OutTabFactories) const
{
}

void FAbilityApplicationMode_Debug::CreateModeTabs(TSharedRef<FNeAbilityBlueprintEditor> AbilityEditor, FWorkflowAllowedTabSet& OutTabFactories) const
{
}