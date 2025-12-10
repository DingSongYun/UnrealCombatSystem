 // Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityEditorApplicationMode.h"

/////////////////////////////////////////////////////
// FWidgetDesignerApplicationMode

class FAbilityApplicationMode_Debug : public FNeAbilityEditorApplicationMode
{
public:
	FAbilityApplicationMode_Debug(TSharedPtr<FNeAbilityBlueprintEditor> InAbilityEditor);

	//~BEGIN: FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	//~End: of FApplicationMode interface

protected:
	void CreateModeTabs(TSharedRef<FNeAbilityBlueprintEditor> AbilityEditor, FWorkflowAllowedTabSet& OutTabFactories) const;
};