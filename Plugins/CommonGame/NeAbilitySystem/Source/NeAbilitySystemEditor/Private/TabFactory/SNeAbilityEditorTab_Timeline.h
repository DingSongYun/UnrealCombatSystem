// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FUICommandList;
struct FNeAbilitySection;
class UNeAbility;
/**
 * SNeAbilityEditorTab_Timeline
 */
class SNeAbilityEditorTab_Timeline : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_Timeline) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit);

	void GenerateSecitonWidgets();

	UNeAbility* GetAbilityAsset() const { return AbilityAsset.Get(); }

	int32 GetSectionNum() const;

	FNeAbilitySection* GetSection(int32 InSectionIndex) const;

	void AddNewSection();
	void DeleteSection(int32 InSectionIndex);
	void LinkSection(int32 SectionIndex, int32 NextSectionIndex) const;
	void LinkSection(int32 SectionIndex, FName NextSectionName) const;
	void MoveDownSection(int32 InSectionIndex);
	void MoveUpSection(int32 InSectionIndex);

	void MarkAbilityModify();

	TSharedPtr<class SSplitter> GetSectionContainer() const { return SectionContainer; }

private:
	void AddSectionSlot(const TSharedPtr<class SNeAbilityTimelineSection>& SectionWidget) const;

private:
	TWeakObjectPtr<UNeAbility> AbilityAsset;
	TWeakPtr<class FNeAbilityBlueprintEditor> AbilityEditor;
	TSharedPtr<class SSplitter> SectionContainer;
	TArray<TSharedPtr<class SNeAbilityTimelineSection>> AbilitySectionWidgets;
};
