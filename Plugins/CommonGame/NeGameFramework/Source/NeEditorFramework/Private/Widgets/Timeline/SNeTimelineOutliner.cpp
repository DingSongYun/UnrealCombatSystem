// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/Timeline/SNeTimelineOutliner.h"
#include "Widgets/Timeline/NeTimelineMode.h"
#include "Widgets/Timeline/NeTimelineTrack.h"
#include "Widgets/Timeline/SNeTimelineOutlinerItem.h"
#include "Widgets/Timeline/SNeTimelineTrackArea.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Timeline/SNeTimelineTrack.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/TextFilterExpressionEvaluator.h"

#define LOCTEXT_NAMESPACE "SNeAnimOutliner"

SNeTimelineOutliner::~SNeTimelineOutliner()
{
	if(TimelineMode.IsValid())
	{
		TimelineMode.Pin()->OnTracksChanged().Remove(TracksChangedDelegateHandle);
	}
}

void SNeTimelineOutliner::Construct(const FArguments& InArgs, const TSharedRef<FNeTimelineMode>& InTimelineMode, const TSharedRef<SNeTimelineTrackArea>& InTrackArea)
{	
	TimelineMode = InTimelineMode;
	TrackArea = InTrackArea;
	FilterText = InArgs._FilterText;
	bPhysicalTracksNeedUpdate = false;

	TracksChangedDelegateHandle = InTimelineMode->OnTracksChanged().AddSP(this, &SNeTimelineOutliner::HandleTracksChanged);

	TextFilter = MakeShareable(new FTextFilterExpressionEvaluator(ETextFilterExpressionEvaluatorMode::BasicString));

	HeaderRow = SNew(SHeaderRow)
		.Visibility(EVisibility::Collapsed);

	HeaderRow->AddColumn(
		SHeaderRow::Column(TEXT("Outliner"))
		.FillWidth(1.0f)
	);

	STreeView::Construct
	(
		STreeView::FArguments()
		.TreeItemsSource(&InTimelineMode->GetRootTracks())
		.SelectionMode(ESelectionMode::Multi)
		.OnGenerateRow(this, &SNeTimelineOutliner::HandleGenerateRow)
		.OnGetChildren(this, &SNeTimelineOutliner::HandleGetChildren)
		.HeaderRow(HeaderRow)
		.ExternalScrollbar(InArgs._ExternalScrollbar)
		.OnExpansionChanged(this, &SNeTimelineOutliner::HandleExpansionChanged)
		.AllowOverscroll(EAllowOverscroll::No)
		.OnContextMenuOpening(this, &SNeTimelineOutliner::HandleContextMenuOpening)
	);

	// expand all
	for(const TSharedRef<FNeTimelineTrack>& RootTrack : InTimelineMode->GetRootTracks())
	{
		RootTrack->Traverse_ParentFirst([this](FNeTimelineTrack& InTrack){ SetItemExpansion(InTrack.AsShared(), InTrack.IsExpanded()); return true; });
	}
}

void SNeTimelineOutliner::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	STreeView::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// These are updated in both tick and paint since both calls can cause changes to the cached rows and the data needs
	// to be kept synchronized so that external measuring calls get correct and reliable results.
	if (bPhysicalTracksNeedUpdate)
	{
		PhysicalTracks.Reset();
		CachedTrackGeometry.GenerateValueArray(PhysicalTracks);

		PhysicalTracks.Sort([](const FCachedGeometry& A, const FCachedGeometry& B)
		{
			return A.Top < B.Top;
		});

		bPhysicalTracksNeedUpdate = false;
	}
}

int32 SNeTimelineOutliner::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = STreeView::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// These are updated in both tick and paint since both calls can cause changes to the cached rows and the data needs
	// to be kept synchronized so that external measuring calls get correct and reliable results.
	if (bPhysicalTracksNeedUpdate)
	{
		PhysicalTracks.Reset();
		CachedTrackGeometry.GenerateValueArray(PhysicalTracks);

		PhysicalTracks.Sort([](const FCachedGeometry& A, const FCachedGeometry& B) 
		{
			return A.Top < B.Top;
		});

		bPhysicalTracksNeedUpdate = false;
	}

	return LayerId + 1;
}

TSharedRef<ITableRow> SNeTimelineOutliner::HandleGenerateRow(TSharedRef<FNeTimelineTrack> InTrack, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<SNeTimelineOutlinerItem> Row =
		SNew(SNeTimelineOutlinerItem, OwnerTable, InTrack)
		.OnGenerateWidgetForColumn(this, &SNeTimelineOutliner::GenerateWidgetForColumn)
		.HighlightText(FilterText);

	// Ensure the track area is kept up to date with the virtualized scroll of the tree view
	TSharedPtr<SNeTimelineTrack> TrackWidget = TrackArea->FindTrackSlot(InTrack);

	if (!TrackWidget.IsValid())
	{
		// Add a track slot for the row
		TrackWidget = SNew(SNeTimelineTrack, InTrack, SharedThis(this))
		[
			 InTrack->GenerateContainerWidgetForTimeline()
		];

		TrackArea->AddTrackSlot(InTrack, TrackWidget);
	}

	if (ensure(TrackWidget.IsValid()))
	{
		Row->AddTrackAreaReference(TrackWidget);
	}

	return Row;
}

TSharedRef<SWidget> SNeTimelineOutliner::GenerateWidgetForColumn(const TSharedRef<FNeTimelineTrack>& InTrack, const FName& ColumnId, const TSharedRef<SNeTimelineOutlinerItem>& Row) const
{
	return InTrack->GenerateContainerWidgetForOutliner(Row);
}

void SNeTimelineOutliner::HandleGetChildren(TSharedRef<FNeTimelineTrack> Item, TArray<TSharedRef<FNeTimelineTrack>>& OutChildren)
{
	if(!FilterText.Get().IsEmpty())
	{
		for(const TSharedRef<FNeTimelineTrack>& Child : Item->GetChildren())
		{
			if(!Child->SupportsFiltering() || TextFilter->TestTextFilter(FBasicStringFilterExpressionContext(Child->GetLabel().ToString())))
			{
				OutChildren.Add(Child);
			}
		}
	}
	else
	{
		OutChildren.Append(Item->GetChildren());
	}
}

void SNeTimelineOutliner::HandleExpansionChanged(TSharedRef<FNeTimelineTrack> InTrack, bool bIsExpanded)
{
	InTrack->SetExpanded(bIsExpanded);
	
	// Expand any children that are also expanded
	for (const TSharedRef<FNeTimelineTrack>& Child : InTrack->GetChildren())
	{
		if (Child->IsExpanded())
		{
			SetItemExpansion(Child, true);
		}
	}
}

TSharedPtr<SWidget> SNeTimelineOutliner::HandleContextMenuOpening()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;

	//TODO
	//FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, AnimModel.Pin()->GetCommandList());

	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, nullptr);

	TimelineMode.Pin()->BuildContextMenu(MenuBuilder);

	// > 1 because the search widget is always added
	return MenuBuilder.GetMultiBox()->GetBlocks().Num() > 1 ? MenuBuilder.MakeWidget() : TSharedPtr<SWidget>();
}

void SNeTimelineOutliner::HandleTracksChanged()
{
	RequestTreeRefresh();

	//auto expande
	for (const TSharedRef<FNeTimelineTrack>& RootTrack : TimelineMode.Pin()->GetRootTracks())
	{
		if (RootTrack->IsExpanded())
		{
			RootTrack->Traverse_ParentFirst([this](FNeTimelineTrack& InTrack) { SetItemExpansion(InTrack.AsShared(), InTrack.IsExpanded()); return true; });
		}
	}
}

void SNeTimelineOutliner::ReportChildRowGeometry(const TSharedRef<FNeTimelineTrack>& InTrack, const FGeometry& InGeometry)
{
	float ChildOffset = TransformPoint(
		Concatenate(
			InGeometry.GetAccumulatedLayoutTransform(),
			GetCachedGeometry().GetAccumulatedLayoutTransform().Inverse()
		),
		FVector2D(0,0)
	).Y;

	FCachedGeometry* ExistingGeometry = CachedTrackGeometry.Find(InTrack);
	if(ExistingGeometry == nullptr || (ExistingGeometry->Top != ChildOffset || ExistingGeometry->Height != InGeometry.Size.Y))
	{
		CachedTrackGeometry.Add(InTrack, FCachedGeometry(InTrack, ChildOffset, InGeometry.Size.Y));
		bPhysicalTracksNeedUpdate = true;
	}
}

void SNeTimelineOutliner::OnChildRowRemoved(const TSharedRef<FNeTimelineTrack>& InTrack)
{
	CachedTrackGeometry.Remove(InTrack);
	bPhysicalTracksNeedUpdate = true;
}

TOptional<SNeTimelineOutliner::FCachedGeometry> SNeTimelineOutliner::GetCachedGeometryForTrack(const TSharedRef<FNeTimelineTrack>& InTrack) const
{
	if (const FCachedGeometry* FoundGeometry = CachedTrackGeometry.Find(InTrack))
	{
		return *FoundGeometry;
	}

	return TOptional<FCachedGeometry>();
}

TOptional<float> SNeTimelineOutliner::ComputeTrackPosition(const TSharedRef<FNeTimelineTrack>& InTrack) const
{
	// Positioning strategy:
	// Attempt to root out any visible track in the specified track's sub-hierarchy, and compute the track's offset from that
	float NegativeOffset = 0.f;
	TOptional<float> Top;
	
	// Iterate parent first until we find a tree view row we can use for the offset height
	auto Iter = [&](FNeTimelineTrack& InTrack)
	{		
		TOptional<FCachedGeometry> ChildRowGeometry = GetCachedGeometryForTrack(InTrack.AsShared());
		if (ChildRowGeometry.IsSet())
		{
			Top = ChildRowGeometry->Top;
			// Stop iterating
			return false;
		}

		NegativeOffset -= InTrack.GetHeight() + InTrack.GetPadding().Combined();
		return true;
	};

	InTrack->TraverseVisible_ParentFirst(Iter);

	if (Top.IsSet())
	{
		return NegativeOffset + Top.GetValue();
	}

	return Top;
}

void SNeTimelineOutliner::ScrollByDelta(float DeltaInSlateUnits)
{
	ScrollBy(GetCachedGeometry(), DeltaInSlateUnits, EAllowOverscroll::No);
}

void SNeTimelineOutliner::Private_SetItemSelection( TSharedRef<FNeTimelineTrack> TheItem, bool bShouldBeSelected, bool bWasUserDirected )
{
	if(TheItem->SupportsSelection())
	{
		TimelineMode.Pin()->SetTrackSelected(TheItem, bShouldBeSelected);

		STreeView::Private_SetItemSelection(TheItem, bShouldBeSelected, bWasUserDirected);
	}
}

void SNeTimelineOutliner::Private_ClearSelection()
{
	TimelineMode.Pin()->ClearTrackSelection();

	STreeView::Private_ClearSelection();
}

void SNeTimelineOutliner::Private_SelectRangeFromCurrentTo( TSharedRef<FNeTimelineTrack> InRangeSelectionEnd )
{
	STreeView::Private_SelectRangeFromCurrentTo(InRangeSelectionEnd);

	for(TSet<TSharedRef<FNeTimelineTrack>>::TIterator Iter = SelectedItems.CreateIterator(); Iter; ++Iter)
	{
		if(!(*Iter)->SupportsSelection())
		{
			Iter.RemoveCurrent();
		}
	}

	for(const TSharedRef<FNeTimelineTrack>& SelectedItem : SelectedItems)
	{
		TimelineMode.Pin()->SetTrackSelected(SelectedItem, true);
	}
}

void SNeTimelineOutliner::RefreshFilter()
{
	TextFilter->SetFilterText(FilterText.Get());

	RequestTreeRefresh();
}

#undef LOCTEXT_NAMESPACE