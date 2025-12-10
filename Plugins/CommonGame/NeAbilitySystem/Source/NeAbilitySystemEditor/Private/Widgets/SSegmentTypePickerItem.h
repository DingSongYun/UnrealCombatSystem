// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "Beams/NeAbilityBeam.h"
#include "SGraphPalette.h"
#include "EdGraph/EdGraphSchema.h"
#include "Styling/SlateIconFinder.h"
#include "NeAbilityActionViewer.h"
#include "SSegmentTypePickerItem.generated.h"

USTRUCT()
struct FEdGraphSchemaAction_AbilityActionItem : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FEdGraphSchemaAction_AbilityActionItem()
		: FEdGraphSchemaAction()
	{}

	FEdGraphSchemaAction_AbilityActionItem(const FNeAbilityActionViewItem& ViewItem, FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping), ActionViewItem(ViewItem)
	{}

	UClass* GetActionClass() const { return ActionViewItem.GetActionClass(); }

public:
	FNeAbilityActionViewItem ActionViewItem;
};

class SSegmentTypePickItem : public SGraphPaletteItem
{
public:
	SLATE_BEGIN_ARGS(SSegmentTypePickItem) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData)
	{
		check(InCreateData->Action.IsValid());

		TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
		ActionPtr = InCreateData->Action;

		const FSlateBrush* ClassIcon = FSlateIconFinder::FindIconBrushForClass(UNeAbilityBeam::StaticClass());

		TSharedPtr<FEdGraphSchemaAction_AbilityActionItem> TaskNodePtr = StaticCastSharedPtr<FEdGraphSchemaAction_AbilityActionItem>(ActionPtr.Pin());
		if (TaskNodePtr.IsValid())
		{
			ClassIcon = FSlateIconFinder::FindIconBrushForClass(TaskNodePtr->GetActionClass());
		}

		this->ChildSlot
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 2.0f, 6.0f, 2.0f)
					[
						SNew(SImage)
						.Image(ClassIcon)
						.Visibility(ClassIcon != FAppStyle::GetDefaultBrush() ? EVisibility::Visible : EVisibility::Collapsed)
					]

				+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0.0f, 3.0f, 6.0f, 3.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(ActionPtr.Pin()->GetMenuDescription())
					//	.HighlightText(ActionPtr.Pin()->GetMenuDescription())
						.ColorAndOpacity(FColor::White)
						.ToolTipText(ActionPtr.Pin()->GetTooltipDescription())
					]
			];
	}

};
