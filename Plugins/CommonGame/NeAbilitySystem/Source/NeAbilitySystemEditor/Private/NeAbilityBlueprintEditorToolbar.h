// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"

class FExtender;
class FToolBarBuilder;
class UToolMenu;
class FNeAbilityBlueprintEditor;


class FNeAbilityBlueprintEditorToolbar : public TSharedFromThis<FNeAbilityBlueprintEditorToolbar>
{
public:
	/** Constructor */
	FNeAbilityBlueprintEditorToolbar(TSharedPtr<FNeAbilityBlueprintEditor>& InAbilityEditor);

	/** 添加 Timeline / Blueprint Mode之前切换的按钮 */
	void AddEditorModesToolbar(TSharedPtr<FExtender> Extender);

	void AddRepairToolbar(UToolMenu* InMenu);

	void AddPlayControlToolbar(TSharedPtr<FExtender> Extender);

private:
	void TryRepairAsset() const;
	FReply OnClick_PlayOrPause() const;
	FReply OnClick_Step() const;

private:
	TWeakPtr<FNeAbilityBlueprintEditor> AbilityEditor;
};
