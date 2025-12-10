// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditor.h"

class UGameplayAbilityBlueprint;
class FNeAbilityBlueprintEditorToolbar;
class UNeAbility;

struct FNeAbilityBlueprintEditorModes
{
	// Mode constants
	static const FName TimelineMode;
	static const FName GraphMode;
	static const FName DebugMode;

	static FText GetLocalizedMode(const FName InMode);

	static bool IsDebugModeEnabled();

private:
	FNeAbilityBlueprintEditorModes() = delete;
};

struct FNeAbilityBlueprintEditorTabs
{
	static const FName Viewport;
	static const FName Timeline;
	static const FName AssetDetails;
	static const FName Details;
	static const FName Palette;
	static const FName DataBoard;
	static const FName AssetBrowser;
	static const FName PreviewSettings;

	static FText GetLocalizedTab(const FName InMode);
};

/**
 * UNeAbilityBlueprintEditor
 * 技能蓝图编辑器
 */
class NEABILITYSYSTEMEDITOR_API FNeAbilityBlueprintEditor : public FBlueprintEditor
{
private:
	using Super = FBlueprintEditor;

public:
	FNeAbilityBlueprintEditor();
	virtual ~FNeAbilityBlueprintEditor() override;

	void InitAbilityBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode);

	//~BEGIN: FBlueprintEditor interface
	virtual void RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated = false) override;
	virtual FGraphAppearanceInfo GetGraphAppearance(class UEdGraph* InGraph) const override;
	virtual UBlueprint* GetBlueprintObj() const override;
protected:
	virtual void InitalizeExtenders() override;
	virtual void CreateDefaultTabContents(const TArray<UBlueprint*>& InBlueprints) override;
	//~END: FBlueprintEditor interface

public:
	/** 获取技能蓝图 */
	UGameplayAbilityBlueprint* GetAbilityBlueprintObj() const;

	/** 获取技能对象(CDO) */
	UNeAbility* GetEditingAbility() const;

	/** 是否是基于时间轴的技能 */
	bool IsTimelineBasedAbility() const;

	/** 获取Toolbar */
	FORCEINLINE TSharedPtr<FNeAbilityBlueprintEditorToolbar> GetEditorToolbarBuilder() const { return EditorToolbar; }

	/** 技能预览设置 */
	FORCEINLINE class UNeAbilityPreviewSettings* GetPreviewSettings() const { return PreviewSettings.Get(); }

	/** 技能预览场景 */
	FORCEINLINE TSharedPtr<class FNeAbilityPreviewScene> GetAbilityPreviewScene() const { return PreviewScene; };

	/** 技能预览播放器 */
	FORCEINLINE TSharedPtr<class FNeAbilityEditorPlayerBase> GetAbilityPreviewPlayer() const { return PreviewPlayer; }

	/** 获取预览场景ViewportClient */
	TSharedPtr<class FNeAbilityEditorViewportClient> GetPreviewViewportClient() const;

	FORCEINLINE TSharedPtr<class SNeAbilityEditorTab_Viewport> GetViewportTabWidget() const { return TabViewport; }

	/** 获取预览场景World */
	class UWorld* GetPreviewWorld() const;

	/** 重置预览World */
	void ResetPreviewWorld() const;

	/** 是否在暂停中 */
	bool IsWorldPaused() const;

	/** 在详情面板中显示Struct */
	void ShowInDetailsView(const TSharedPtr<FStructOnScope>& InStruct);

	/** 在详情面板中显示Object */
	void ShowInDetailsView(const TArray<UObject*>& InObjects);

	/** Request play Ability */
	void PlayAbility();

	/** Request Pause Ability */
	void PauseAbility();

	/** Step Ability */
	void StepAbility() const;

	/** Play Section */
	void PlaySection(int32 Index) const;

	/** Pause Section */
	void PauseSection(int32 Index);

	/** Is Section Playing */
	bool IsPlayingSection(int32 Index) const;

protected:
	void BindToolkitCommands();

private:
	void CreatePreviewScene();
	void InitPreviewContext();
	void CreatePreviewProxy();

protected:
	TSharedPtr<FNeAbilityBlueprintEditorToolbar> EditorToolbar;

	/** Preview scene */
	TSharedPtr<class FNeAbilityPreviewScene> PreviewScene;

	/** Preview Settings */
	TWeakObjectPtr<UNeAbilityPreviewSettings> PreviewSettings;

	/** Preview Proxy */
	TSharedPtr<class FNeAbilityEditorPlayerBase> PreviewPlayer;

	friend struct FEditorViewportSummoner;
	friend struct FAssetDetailsSummoner;
	friend struct FDetailsTabSummoner;

	TSharedPtr<class SNeAbilityEditorTab_AssetDetails> TabAbilityDefaults;
	TSharedPtr<class SNeAbilityEditorTab_Details> TabDetails;
	TSharedPtr<class SNeAbilityEditorTab_Viewport> TabViewport;

	bool bTimelineBasedAbility = false;
};
