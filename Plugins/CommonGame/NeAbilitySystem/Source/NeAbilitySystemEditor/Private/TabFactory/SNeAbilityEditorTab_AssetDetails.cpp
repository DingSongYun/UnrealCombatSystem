// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityEditorTab_AssetDetails.h"

#include "NeAbility.h"
#include "SKismetInspector.h"
#include "NeAbilityBlueprintEditor.h"

void SNeAbilityEditorTab_AssetDetails::Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityBlueprintEditor>& InAssetEditorToolkit)
{
	ChildSlot
	[
		SAssignNew(KismetInspector, SKismetInspector)
			. Kismet2(InAssetEditorToolkit)
			. ViewIdentifier(FName("BlueprintDefaults"))
			. IsEnabled(true)
			. ShowPublicViewControl(true)
			. ShowTitleArea(false)
			. HideNameArea(true)
			. OnFinishedChangingProperties(InArgs._OnFinishedChangingProperties)
	];
	KismetInspector->ShowDetailsForSingleObject(InAssetEditorToolkit->GetEditingAbility());
}
