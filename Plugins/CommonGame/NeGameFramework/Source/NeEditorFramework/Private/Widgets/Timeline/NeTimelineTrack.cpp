// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/Timeline/NeTimelineTrack.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Preferences/PersonaOptions.h"
#include "Animation/AnimSequenceBase.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Timeline/SNeTimelineOutlinerItem.h"
#include "Widgets/Timeline/NeTimelineMode.h"

#define LOCTEXT_NAMESPACE "FAnimTimelineTrack"

const float FNeTimelineTrack::OutlinerRightPadding = 8.0f;

ANIMTIMELINE_IMPLEMENT_TRACK(FNeTimelineTrack);

FText FNeTimelineTrack::GetLabel() const
{
	return DisplayName;
}

FText FNeTimelineTrack::GetToolTipText() const
{
	return ToolTipText;
}

bool FNeTimelineTrack::Traverse_ChildFirst(const TFunctionRef<bool(FNeTimelineTrack&)>& InPredicate, bool bIncludeThisTrack)
{
	for (TSharedRef<FNeTimelineTrack>& Child : Children)
	{
		if (!Child->Traverse_ChildFirst(InPredicate, true))
		{
			return false;
		}
	}

	return bIncludeThisTrack ? InPredicate(*this) : true;
}

bool FNeTimelineTrack::Traverse_ParentFirst(const TFunctionRef<bool(FNeTimelineTrack&)>& InPredicate, bool bIncludeThisTrack)
{
	if (bIncludeThisTrack && !InPredicate(*this))
	{
		return false;
	}

	for (TSharedRef<FNeTimelineTrack>& Child : Children)
	{
		if (!Child->Traverse_ParentFirst(InPredicate, true))
		{
			return false;
		}
	}

	return true;
}

bool FNeTimelineTrack::TraverseVisible_ChildFirst(const TFunctionRef<bool(FNeTimelineTrack&)>& InPredicate, bool bIncludeThisTrack)
{
	// If the item is not expanded, its children ain't visible
	if (IsExpanded())
	{
		for (TSharedRef<FNeTimelineTrack>& Child : Children)
		{
			if (Child->IsVisible() && !Child->TraverseVisible_ChildFirst(InPredicate, true))
			{
				return false;
			}
		}
	}

	if (bIncludeThisTrack && IsVisible())
	{
		return InPredicate(*this);
	}

	// Continue iterating regardless of visibility
	return true;
}

bool FNeTimelineTrack::TraverseVisible_ParentFirst(const TFunctionRef<bool(FNeTimelineTrack&)>& InPredicate, bool bIncludeThisTrack)
{
	if (bIncludeThisTrack && IsVisible() && !InPredicate(*this))
	{
		return false;
	}

	// If the item is not expanded, its children ain't visible
	if (IsExpanded())
	{
		for (TSharedRef<FNeTimelineTrack>& Child : Children)
		{
			if (Child->IsVisible() && !Child->TraverseVisible_ParentFirst(InPredicate, true))
			{
				return false;
			}
		}
	}

	return true;
}

TSharedRef<SWidget> FNeTimelineTrack::GenerateContainerWidgetForOutliner(const TSharedRef<SNeTimelineOutlinerItem>& InRow)
{
	TSharedPtr<SBorder> OuterBorder;
	TSharedPtr<SHorizontalBox> InnerHorizontalBox;
	TSharedRef<SWidget> Widget = GenerateStandardOutlinerWidget(InRow, true, OuterBorder, InnerHorizontalBox);

	if(bIsHeaderTrack)
	{
		OuterBorder->SetBorderBackgroundColor(FAppStyle::GetColor("AnimTimeline.Outliner.HeaderColor"));
	}

	return Widget;
}

TSharedRef<SWidget> FNeTimelineTrack::GenerateStandardOutlinerWidget(const TSharedRef<SNeTimelineOutlinerItem>& InRow, bool bWithLabelText, TSharedPtr<SBorder>& OutOuterBorder, TSharedPtr<SHorizontalBox>& OutInnerHorizontalBox)
{
	TSharedRef<SWidget> Widget =
		SAssignNew(OutOuterBorder, SBorder)
		.ToolTipText(this, &FNeTimelineTrack::GetToolTipText)
		.BorderImage(FAppStyle::GetBrush("Sequencer.Section.BackgroundTint"))
		.BorderBackgroundColor(FAppStyle::GetColor("AnimTimeline.Outliner.ItemColor"))
		[
			SAssignNew(OutInnerHorizontalBox, SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(4.0f, 1.0f)
			[
				SNew(SExpanderArrow, InRow)
			]
		];

	if(bWithLabelText)
	{
		OutInnerHorizontalBox->AddSlot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.Padding(2.0f, 1.0f)
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.TextStyle(&FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("AnimTimeline.Outliner.Label"))
				.Text(this, &FNeTimelineTrack::GetLabel)
				.HighlightText(InRow->GetHighlightText())
			];
	}

	return Widget;
}

TSharedRef<SWidget> FNeTimelineTrack::GenerateContainerWidgetForTimeline()
{
	return SNullWidget::NullWidget;
}

void FNeTimelineTrack::AddToContextMenu(FMenuBuilder& InMenuBuilder, TSet<FName>& InOutExistingMenuTypes) const
{

}

float FNeTimelineTrack::GetMaxInput() const
{
	//return GetModel()->GetAnimSequenceBase()->GetPlayLength(); 
	return GetTimelineMode()->GetPlayLength();
}

float FNeTimelineTrack::GetViewMinInput() const
{
	return GetTimelineMode()->GetViewRange().GetLowerBoundValue();
}

float FNeTimelineTrack::GetViewMaxInput() const
{
	return GetTimelineMode()->GetViewRange().GetUpperBoundValue();
}

float FNeTimelineTrack::GetScrubValue() const
{
	const int32 Resolution = FMath::RoundToInt(GetTimelineMode()->GetScrubSnapValue() * GetTimelineMode()->GetFrameRate());
	return (float)((double)GetTimelineMode()->GetScrubPosition().Value / (double)Resolution);
}

void FNeTimelineTrack::SelectObjects(const TArray<UObject*>& SelectedItems)
{
	GetTimelineMode()->SelectObjects(SelectedItems);
}

void FNeTimelineTrack::OnSetInputViewRange(float ViewMin, float ViewMax)
{
	GetTimelineMode()->SetViewRange(TRange<double>(ViewMin, ViewMax));
}

#undef LOCTEXT_NAMESPACE