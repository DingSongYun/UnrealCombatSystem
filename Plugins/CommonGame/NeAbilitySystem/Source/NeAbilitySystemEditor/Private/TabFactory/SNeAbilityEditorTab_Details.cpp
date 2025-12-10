// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityEditorTab_Details.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityEditorDelegates.h"
#include "NeAbilityPreviewScene.h"
#include "CustomLayout/NeAbilitySegmentCustomization.h"
#include "Details/NeDetailViewExtensionHandler.h"
#include "Misc/NeAbilityGizmoActor.h"
#include "Timeline/NeAbilitySegmentEditorObject.h"

void SNeAbilityEditorTab_Details::Construct(const FArguments& InArgs, const TSharedPtr<class FNeAbilityBlueprintEditor>& InAssetEditorToolkit)
{
	HostEditor = InAssetEditorToolkit;

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NotifyHook = this;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

	if (InArgs._TopContent.IsValid())
	{
		Content->AddSlot()
		.AutoHeight()
		[
			InArgs._TopContent.ToSharedRef()
		];
	}

	Content->AddSlot()
	.FillHeight(1.0f)
	[
		SAssignNew(Inspector, SKismetInspector)
			. HideNameArea(true)
			. ViewIdentifier(FName("Ability Inspector"))
			. Kismet2(StaticCastSharedPtr<FBlueprintEditor>(ConstCastSharedPtr<FNeAbilityBlueprintEditor>(InAssetEditorToolkit)))
			. OnFinishedChangingProperties( FOnFinishedChangingProperties::FDelegate::CreateSP(this, &SNeAbilityEditorTab_Details::OnFinishedChangingProperties) )
		// DetailsView.ToSharedRef()
	];

	// Inspector->GetPropertyView()->RegisterInstancedCustomPropertyLayout(FNeAbilitySegmentCustomization::GetTypeName(), FOnGetDetailCustomizationInstance::CreateStatic(&FNeAbilitySegmentCustomization::MakeInstance));

	// Create a handler for property binding
	const TSharedRef<FNeDetailViewExtensionHandler> DetailExtensionHandler = MakeShareable( new FNeDetailViewExtensionHandler( InAssetEditorToolkit ) );
	Inspector->GetPropertyView()->SetExtensionHandler(DetailExtensionHandler);

	if (InArgs._BottomContent.IsValid())
	{
		Content->AddSlot()
		.AutoHeight()
		[
			InArgs._BottomContent.ToSharedRef()
		];
	}

	ChildSlot
	[
		Content
	];
}

void SNeAbilityEditorTab_Details::SetDetailObject(UObject* InObject)
{
	DetailsView->SetObject(InObject);
}

void SNeAbilityEditorTab_Details::ShowSingleStruct(TSharedPtr<FStructOnScope> InStructToDisplay)
{
	Inspector->ShowSingleStruct(InStructToDisplay);
}

void SNeAbilityEditorTab_Details::ShowDetailsForSingleObject(UObject* Object, const FShowDetailsOptions& Options)
{
	Inspector->ShowDetailsForSingleObject(Object, Options);
}

void SNeAbilityEditorTab_Details::ShowDetailsForObjects(const TArray<UObject*>& PropertyObjects, const FShowDetailsOptions& Options)
{
	DetailObjects = PropertyObjects;
	Inspector->ShowDetailsForObjects(PropertyObjects, Options);
}

void SNeAbilityEditorTab_Details::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	// No need
	// FNotifyHook::NotifyPostChange(PropertyChangedEvent, PropertyThatChanged);
}

void SNeAbilityEditorTab_Details::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	for (TObjectPtr<UObject> Object : DetailObjects)
	{
		if (UNeAbilitySegmentEditorObject* SegmentObject = Cast<UNeAbilitySegmentEditorObject>(Object))
		{
			if (SegmentObject->SegmentPtr->ShouldCreateGizmo())
			{
				ANeAbilityGizmoActor* GizmoActor = HostEditor.Pin()->GetAbilityPreviewScene()->FindGizmoActor(SegmentObject->SegmentPtr);
				if (GizmoActor)
				{
					GizmoActor->PostTaskChangeProperty(PropertyChangedEvent);
				}
			}
		}
	}
	FNeAbilityEditorDelegates::OnDetailPropertyChanged.Broadcast(DetailObjects[0], PropertyChangedEvent);
}
