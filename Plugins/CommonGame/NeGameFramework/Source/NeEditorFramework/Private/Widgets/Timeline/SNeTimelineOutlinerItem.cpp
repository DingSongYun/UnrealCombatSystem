// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/Timeline/SNeTimelineOutlinerItem.h"
#include "Widgets/Timeline/NeTimelineTrack.h"
#include "Widgets/Timeline/SNeTimelineOutliner.h"
#include "Widgets/SOverlay.h"

SNeTimelineOutlinerItem::~SNeTimelineOutlinerItem()
{
	TSharedPtr<SNeTimelineOutliner> Outliner = StaticCastSharedPtr<SNeTimelineOutliner>(OwnerTablePtr.Pin());
	TSharedPtr<FNeTimelineTrack> PinnedTrack = Track.Pin();
	if (Outliner.IsValid() && PinnedTrack.IsValid())
	{
		Outliner->OnChildRowRemoved(PinnedTrack.ToSharedRef());
	}
}

void SNeTimelineOutlinerItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedRef<FNeTimelineTrack>& InTrack)
{
	Track = InTrack;
	OnGenerateWidgetForColumn = InArgs._OnGenerateWidgetForColumn;
	HighlightText = InArgs._HighlightText;

	bHovered = false;
	SetHover(TAttribute<bool>::CreateSP(this, &SNeTimelineOutlinerItem::ShouldAppearHovered));

	SMultiColumnTableRow::Construct(
		SMultiColumnTableRow::FArguments()
			.ShowSelection(true),
		InOwnerTableView);
}

TSharedRef<SWidget> SNeTimelineOutlinerItem::GenerateWidgetForColumn(const FName& ColumnId)
{
	TSharedPtr<FNeTimelineTrack> PinnedTrack = Track.Pin();
	if (PinnedTrack.IsValid())
	{
		TSharedPtr<SWidget> ColumnWidget = SNullWidget::NullWidget;
		if(OnGenerateWidgetForColumn.IsBound())
		{
			ColumnWidget = OnGenerateWidgetForColumn.Execute(PinnedTrack.ToSharedRef(), ColumnId, SharedThis(this));
		}

		return SNew(SOverlay)
		+SOverlay::Slot()
		[
			ColumnWidget.ToSharedRef()
		];
	}

	return SNullWidget::NullWidget;
}

void SNeTimelineOutlinerItem::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	StaticCastSharedPtr<SNeTimelineOutliner>(OwnerTablePtr.Pin())->ReportChildRowGeometry(Track.Pin().ToSharedRef(), AllottedGeometry);
}

FVector2D SNeTimelineOutlinerItem::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	TSharedPtr<FNeTimelineTrack> PinnedTrack = Track.Pin();
	if(PinnedTrack.IsValid())
	{
		return FVector2D(100.0f, PinnedTrack->GetHeight() + PinnedTrack->GetPadding().Combined());
	}

	return FVector2D(100.0f, 16.0f);
}

void SNeTimelineOutlinerItem::AddTrackAreaReference(const TSharedPtr<SNeTimelineTrack>& InTrackWidget)
{
	TrackWidget = InTrackWidget;
}

void SNeTimelineOutlinerItem::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bHovered = true;
	SMultiColumnTableRow<TSharedRef<FNeTimelineTrack>>::OnMouseEnter(MyGeometry, MouseEvent);

	TSharedPtr<FNeTimelineTrack> PinnedTrack = Track.Pin();
	if(PinnedTrack.IsValid())
	{
		PinnedTrack->SetHovered(true);
	}
}

void SNeTimelineOutlinerItem::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	bHovered = false;

	SMultiColumnTableRow<TSharedRef<FNeTimelineTrack>>::OnMouseLeave(MouseEvent);

	TSharedPtr<FNeTimelineTrack> PinnedTrack = Track.Pin();
	if(PinnedTrack.IsValid())
	{
		PinnedTrack->SetHovered(false);
	}
}

bool SNeTimelineOutlinerItem::ShouldAppearHovered() const
{
	if(TSharedPtr<FNeTimelineTrack> PinnedTrack = Track.Pin())
	{
		return bHovered || PinnedTrack->IsHovered();
	}

	return bHovered;
}