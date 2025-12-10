// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityEditorUtilities.h"
#include "ClassViewerFilter.h"
#include "Widgets/SWidget.h"
#include "Factories.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"
#include "NeAbility.h"
#include "NeAbilityActionViewer.h"
#include "SGraphActionMenu.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SSegmentTypePickerItem.h"

#define LOCTEXT_NAMESPACE "AbilityBlueprintEditor"

class FTextObjectFactory_Segment : public FCustomizableTextObjectFactory
{
public:
	FTextObjectFactory_Segment ()
		: FCustomizableTextObjectFactory(GWarn)
	{
	}

	virtual bool CanCreateClass(UClass* InObjectClass, bool& bOmitSubObjs) const override
	{
		// if (InObjectClass->IsChildOf(UNeAbilitySegment::StaticClass()))
		// {
		// 	return true;
		// }
		return false;
	}


	virtual void ProcessConstructedObject(UObject* NewObject) override
	{
		check(NewObject);

		// Tasks.Add(Cast<UNeAbilitySegment>(NewObject));
	}

public:
	TArray<UNeAbilitySegment*> Tasks;
};

/**
 * FSegmentTypeClassFilter
 *
 * For segment picker
 */
class FSegmentTypeClassFilter : public IClassViewerFilter
{
	TArray<UClass*> SupportTypes;
public:
	FSegmentTypeClassFilter()
	{
		SupportTypes = FNeAbilityActionViewer::GetAbilityActionSupportTypes();
	}

	bool IsChildOfSegmentType(const UClass* InClass) const
	{
		for (UClass* Type : SupportTypes)
		{
			if (InClass->IsChildOf(Type))
			{
				return true;
			}
		}

		return false;
	}

	bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		const bool bChildOfObjectClass = IsChildOfSegmentType(InClass);
		const bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);
		return bChildOfObjectClass && bMatchesFlags;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		const bool bChildOfObjectClass = IsChildOfSegmentType(InUnloadedClassData->GetClassWithin());
		const bool bMatchesFlags = !InUnloadedClassData->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);

		return bChildOfObjectClass && bMatchesFlags;
	}
};

static bool bSegmentPicker_ShowFullClassName = false;
void FNeAbilityEditorUtilities::MakeNewSegmentPicker(const TSharedRef<class FNeAbilityBlueprintEditor>& AbilityEidtorPtr, class FMenuBuilder& MenuBuilder, const FOnClassPicked& OnTypePicked, bool bUseClassViewer)
{
	// MenuBuilder always has a search widget added to it by default, hence if larger then 1 then something else has been added to it
	if (MenuBuilder.GetMultiBox()->GetBlocks().Num() > 1)
	{
		MenuBuilder.AddMenuSeparator();
	}

	if (bUseClassViewer)
	{
		FClassViewerInitializationOptions InitOptions;
		InitOptions.Mode = EClassViewerMode::ClassPicker;
		InitOptions.bShowObjectRootClass = false;
		InitOptions.bShowUnloadedBlueprints = true;
		InitOptions.bShowNoneOption = false;
		InitOptions.bEnableClassDynamicLoading = true;
		InitOptions.bExpandRootNodes = true;
		InitOptions.NameTypeToDisplay = bSegmentPicker_ShowFullClassName ? EClassViewerNameTypeToDisplay::Dynamic : EClassViewerNameTypeToDisplay::DisplayName;
		InitOptions.ClassFilters.Add(MakeShared<FSegmentTypeClassFilter>());
		InitOptions.bShowBackgroundBorder = false;
		InitOptions.bAllowViewOptions = true;

		FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
		MenuBuilder.AddWidget(
			SNew(SBox)
			.MinDesiredWidth(300.0f)
			.MaxDesiredHeight(400.0f)
			[
				ClassViewerModule.CreateClassViewer(InitOptions, OnTypePicked)
			],
			FText(), true, false
		);
	}
	else
	{
		MenuBuilder.AddWidget(
			SNew(SBox)
			.MinDesiredWidth(300.0f)
			.MaxDesiredHeight(400.0f)
			[
				SNew(SGraphActionMenu)
				.OnCreateWidgetForAction_Lambda([](FCreateWidgetForActionData* const InCreateData)
				{
					return SNew(SSegmentTypePickItem, InCreateData);
				})
				.OnCollectAllActions_Lambda([AbilityEidtorPtr](FGraphActionListBuilderBase& OutAllActions)
				{
					FGraphActionMenuBuilder ActionMenuBuilder;
					for (const FNeAbilityActionViewItem& Action : FNeAbilityActionViewer::GetAllAbilityActions())
					{
						const FText DisplayName = !bSegmentPicker_ShowFullClassName ? Action.GetDisplayNameText()
							: FText::FromString(Action.GetDisplayNameText().ToString() + TEXT("(") + Action.GetClassName() + TEXT(")"));
						TSharedPtr<FEdGraphSchemaAction_AbilityActionItem> NewAction = MakeShared<FEdGraphSchemaAction_AbilityActionItem>(
							FEdGraphSchemaAction_AbilityActionItem(Action, FText::GetEmpty(), DisplayName, Action.GetToolTips(), 0));
						ActionMenuBuilder.AddAction(NewAction);
					}
					OutAllActions.Append(ActionMenuBuilder);

				})

				.OnActionSelected_Lambda([OnTypePicked](const TArray< TSharedPtr<FEdGraphSchemaAction> >& Action, ESelectInfo::Type type)
				{
					if (Action.Num() > 0)
					{
						TSharedPtr<FEdGraphSchemaAction_AbilityActionItem> TaskNodePtr = StaticCastSharedPtr<FEdGraphSchemaAction_AbilityActionItem>(Action[0]);
						if (TaskNodePtr.IsValid())
						{
							OnTypePicked.ExecuteIfBound(TaskNodePtr->GetActionClass());
						}
					}
				})

				.OnActionDoubleClicked_Lambda([OnTypePicked](const TArray< TSharedPtr<FEdGraphSchemaAction> >& Action)
				{
					if (Action.Num() > 0)
					{
						TSharedPtr<FEdGraphSchemaAction_AbilityActionItem> TaskNodePtr = StaticCastSharedPtr<FEdGraphSchemaAction_AbilityActionItem>(Action[0]);
						if (TaskNodePtr.IsValid())
						{
							OnTypePicked.ExecuteIfBound(TaskNodePtr->GetActionClass());
						}
					}
				})

			.AutoExpandActionMenu(true)
			], FText(), true, false
		);
	}


	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("TimeFormat_ClassViewText", "Display Full ClassText"),
		LOCTEXT("TimeFormat_ClassViewText", "Display all Class Text"),
		FSlateIcon(),
		FUIAction(
		FExecuteAction::CreateLambda([]()
			{
				bSegmentPicker_ShowFullClassName = !bSegmentPicker_ShowFullClassName;
			}),
		FCanExecuteAction(),
		FIsActionChecked::CreateLambda([AbilityEidtorPtr]() { return bSegmentPicker_ShowFullClassName; })
		),
		NAME_None,
		EUserInterfaceActionType::RadioButton
	);
}

TSharedRef<SWidget> FNeAbilityEditorUtilities::MakeTrackButton(FText HoverText, FOnGetContent MenuContent, const TAttribute<bool>& HoverState)
{
	FSlateFontInfo SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 8);

	TSharedRef<STextBlock> ComboButtonText = SNew(STextBlock)
		.Text(HoverText)
		.Font(SmallLayoutFont)
		.ColorAndOpacity(FSlateColor::UseForeground());

	TSharedRef<SComboButton> ComboButton =

		SNew(SComboButton)
		.HasDownArrow(false)
		.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
		.ForegroundColor(FSlateColor::UseForeground())
		.OnGetMenuContent(MenuContent)
		.ContentPadding(FMargin(5, 2))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.ButtonContent()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(0, 0, 2, 0))
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(FAppStyle::GetBrush("ComboButton.Arrow"))
		]

	+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			ComboButtonText
		]
		];

	auto GetRolloverVisibility = [WeakComboButton = TWeakPtr<SComboButton>(ComboButton), HoverState]()
	{
		TSharedPtr<SComboButton> ComboButton = WeakComboButton.Pin();
		if (HoverState.Get() || ComboButton->IsOpen())
		{
			return EVisibility::SelfHitTestInvisible;
		}
		else
		{
			return EVisibility::Collapsed;
		}
	};

	TAttribute<EVisibility> Visibility = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(GetRolloverVisibility));
	ComboButtonText->SetVisibility(Visibility);

	return ComboButton;
}

void FNeAbilityEditorUtilities::ImportTasksFromText(UNeAbility* Asset, const FString& ImportText, TArray<UNeAbilitySegment*>& OutTasks)
{
	FTextObjectFactory_Segment Factory;
	Factory.ProcessBuffer(Asset, RF_Transactional, ImportText);

	OutTasks = Factory.Tasks;
}

#undef LOCTEXT_NAMESPACE
