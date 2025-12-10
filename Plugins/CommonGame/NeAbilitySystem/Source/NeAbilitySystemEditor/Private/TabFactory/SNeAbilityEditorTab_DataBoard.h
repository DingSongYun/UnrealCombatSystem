// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PropertyEditorDelegates.h"
#include "PropertyEditorModule.h"
#include "Widgets/SCompoundWidget.h"
#include "NeAbility.h"
#include "Timeline/NeAbilitySegmentEditorObject.h"

/**
 * SNeAbilityEditorTab_AssetDetails
 */
class SNeAbilityEditorTab_DataBoard : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_DataBoard) {};
	SLATE_ARGUMENT( FOnFinishedChangingProperties::FDelegate, OnFinishedChangingProperties )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit)
	{
		UNeAbility* EditingAbility = InAssetEditorToolkit->GetEditingAbility();
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		{
			DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
			DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
			DetailsViewArgs.bHideSelectionTip = true;
			DetailsViewArgs.NotifyHook = this;
		}

		FStructureDetailsViewArgs StructureDetailsViewArgs;
		{
			StructureDetailsViewArgs.bShowObjects = true;
			StructureDetailsViewArgs.bShowAssets = true;
			StructureDetailsViewArgs.bShowClasses = true;
			StructureDetailsViewArgs.bShowInterfaces = true;
		}

		FNeAbilityDataBoard& DataBoard = EditingAbility->GetMutableDataBoard();
		TSharedRef<FStructOnScope> DataStruct = MakeShareable(new FStructOnScope(DataBoard.StaticStruct(), (uint8*)&DataBoard));
		TSharedRef<IStructureDetailsView> DetailsView = PropertyEditorModule.CreateStructureDetailView(DetailsViewArgs, StructureDetailsViewArgs, DataStruct);

		ChildSlot
		[
			DetailsView->GetWidget().ToSharedRef()
		];
	}

	virtual void NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged )
	{
	}
};
