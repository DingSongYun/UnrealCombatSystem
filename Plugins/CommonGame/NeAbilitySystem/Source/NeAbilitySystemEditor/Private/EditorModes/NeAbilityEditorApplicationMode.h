 // Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "BlueprintEditorModes.h"

class FNeAbilityBlueprintEditor;

class FNeAbilityEditorApplicationMode : public FBlueprintEditorApplicationMode
{
public:
	FNeAbilityEditorApplicationMode(TSharedPtr<FNeAbilityBlueprintEditor> InAbilityEditor, FName InModeName);

	//~BEGIN: FApplicationMode interface
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;
	//~End: FApplicationMode interface

public:
	FORCEINLINE TSharedPtr<FNeAbilityBlueprintEditor> GetBlueprintEditor() const { return MyBlueprintEditor.Pin(); }
	class UGameplayAbilityBlueprint* GetBlueprint() const;

protected:
	TWeakPtr<FNeAbilityBlueprintEditor> MyBlueprintEditor;

	FWorkflowAllowedTabSet TabFactories;
};
