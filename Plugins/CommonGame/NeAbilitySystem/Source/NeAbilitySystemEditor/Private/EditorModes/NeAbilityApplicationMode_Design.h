 // Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityEditorApplicationMode.h"

/////////////////////////////////////////////////////
// FWidgetDesignerApplicationMode

class FAbilityApplicationMode_Design : public FNeAbilityEditorApplicationMode
{
public:
	FAbilityApplicationMode_Design(TSharedPtr<FNeAbilityBlueprintEditor> InAbilityEditor);

	//~BEGIN: FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;
	//~End: of FApplicationMode interface

protected:
	void CreateModeTabs(TSharedRef<FNeAbilityBlueprintEditor> AbilityEditor, FWorkflowAllowedTabSet& OutTabFactories) const;
};