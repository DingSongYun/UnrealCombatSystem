// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//~=============================================================
// Asset type registration
#define REG_ASSET_TYPE_SECTION_BEGIN(AssetTools, AssetActionArray) \
	{ \
	auto& _AssetTypeActions_ = AssetActionArray; \
	auto& _AssetTools_ = AssetTools;

#define REG_ASSET_TYPE_ACTIONS(ActionClass, Category) \
	{ TSharedRef<IAssetTypeActions> NewAction = MakeShareable(new ActionClass(Category)); \
	AssetTools.RegisterAssetTypeActions(NewAction); \
	_AssetTypeActions_.Add(NewAction); }

#define REG_ASSET_TYPE_SECTION_END() }

//~=============================================================
// Custom layouts
#define REG_CUSTOM_PROP_LAYOUT_GENERIC(StructType, CustomLayout) \
	PropertyModule.RegisterCustomPropertyTypeLayout(StructType::StaticStruct()->GetFName(), \
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&CustomLayout::MakeInstance));

//~=============================================================
// Editor tabs
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
		: FWorkflowTabFactory(TabIdentify, InHostingApp) {}

#define DECLARE_TAB_SUMMONER_END };



