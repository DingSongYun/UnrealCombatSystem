// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SKismetInspector.h"
#include "Misc/NotifyHook.h"
#include "Widgets/SCompoundWidget.h"

class FStructOnScope;

typedef SKismetInspector::FShowDetailsOptions FShowDetailsOptions;

/**
 * SNeAbilityEditorTab_AssetDetails
 */
class SNeAbilityEditorTab_Details : public SCompoundWidget, FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityEditorTab_Details) {}

	/** Optional content to display above the details panel */
	SLATE_ARGUMENT(TSharedPtr<SWidget>, TopContent)

	/** Optional content to display below the details panel */
	SLATE_ARGUMENT(TSharedPtr<SWidget>, BottomContent)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit);

	void SetDetailObject(UObject* InObject);

	void ShowSingleStruct(TSharedPtr<FStructOnScope> InStructToDisplay);

	void ShowDetailsForSingleObject(UObject* Object, const FShowDetailsOptions& Options = FShowDetailsOptions());

	void ShowDetailsForObjects(const TArray<UObject*>& PropertyObjects, const FShowDetailsOptions& Options = FShowDetailsOptions());

	//~BEGIN: FNotifyHook Interface
	virtual void NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged ) override;
	//~END: FNotifyHook Interface

private:
	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

private:
	TSharedPtr<class IDetailsView> DetailsView;
	TSharedPtr<class SKismetInspector> Inspector;
	TWeakPtr<class FNeAbilityBlueprintEditor> HostEditor;

	TArray<TObjectPtr<UObject>> DetailObjects;
};
