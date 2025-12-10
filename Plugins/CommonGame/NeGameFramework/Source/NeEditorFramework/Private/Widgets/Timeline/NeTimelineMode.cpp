// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/Timeline/NeTimelineMode.h"

#include "Misc/FrameTime.h"
#include "Widgets/Timeline/NeTimelineTrack.h"

FNeTimelineMode::FNeTimelineMode(const TSharedRef<FUICommandList>& InCommandList) : CommandList(InCommandList)
{
	bIsSelecting = false;
	ScrubSnapValue = 1000;
	bDebuggerMode = false;
}

void FNeTimelineMode::SetViewRange(const TRange<double>& InRange)
{
	ViewRange = InRange;

	if(WorkingRange.HasLowerBound() && WorkingRange.HasUpperBound())
	{
		WorkingRange = TRange<double>::Hull(WorkingRange, ViewRange);
	}
	else
	{
		WorkingRange = ViewRange;
	}
}

TRange<FFrameNumber> FNeTimelineMode::GetPlaybackRange() const
{
	const int32 Resolution = GetTickResolution();
	return TRange<FFrameNumber>(FFrameNumber(FMath::RoundToInt32(PlaybackRange.GetLowerBoundValue() * static_cast<double>(Resolution))), FFrameNumber(FMath::RoundToInt32(PlaybackRange.GetUpperBoundValue() * (double)Resolution)));
}

FFrameNumber FNeTimelineMode::GetScrubPosition() const
{
	return FFrameNumber(FMath::RoundToInt32(GetScrubTime() * static_cast<double>(GetTickResolution())));
}

void FNeTimelineMode::SetScrubPosition(FFrameTime NewScrubPosition)
{
	SetScrubTime(static_cast<float>(NewScrubPosition.AsDecimal() / static_cast<double>(GetTickResolution())));
}

double FNeTimelineMode::GetFrameRate() const
{
	return 30.f;
}

int32 FNeTimelineMode::GetTickResolution() const
{
	return FMath::RoundToInt32(static_cast<double>(GetScrubSnapValue()) * GetFrameRate());
}

void FNeTimelineMode::HandleViewRangeChanged(TRange<double> InRange, EViewRangeInterpolation InInterpolation)
{
	SetViewRange(InRange);
}

void FNeTimelineMode::HandleWorkingRangeChanged(TRange<double> InRange)
{
	WorkingRange = InRange;
}

TSharedPtr<FNeTimelineTrack> FNeTimelineMode::GetHoveredTrack() const
{
	for (const TSharedRef<FNeTimelineTrack>& Track : RootTracks)
	{
		if (IsTrackHovered(Track))
		{
			return Track;
		}
	}

	return nullptr;
}

bool FNeTimelineMode::IsTrackHovered(TSharedPtr<FNeTimelineTrack> InTrack) const
{
	check(InTrack.IsValid());
	if (InTrack->IsHovered())
	{
		return true;
	}

	for (const TSharedRef<FNeTimelineTrack>& ChildTrack : InTrack->GetChildren())
	{
		if (IsTrackHovered(ChildTrack))
		{
			return true;
		}
	}

	return false;
}

void FNeTimelineMode::RefreshTracks()
{
	OnTracksChangedDelegate.Broadcast();
}

bool FNeTimelineMode::IsTrackSelected(const TSharedRef<FNeTimelineTrack>& InTrack) const
{
	return SelectedTracks.Find(InTrack) != nullptr;
}

void FNeTimelineMode::ClearTrackSelection()
{
	SelectedTracks.Empty();
}

void FNeTimelineMode::SetTrackSelected(const TSharedRef<FNeTimelineTrack>& InTrack, bool bIsSelected)
{
	if(bIsSelected)
	{
		SelectedTracks.Add(InTrack);
	}
	else
	{
		SelectedTracks.Remove(InTrack);
	}
}

void FNeTimelineMode::SelectObjects(const TArray<UObject*>& Objects)
{
	if(!bIsSelecting)
	{
		TGuardValue<bool> GuardValue(bIsSelecting, true);
		OnHandleObjectsSelectedDelegate.Broadcast(Objects);
	}
}

void FNeTimelineMode::SelectStructs(const TArray<TSharedPtr<FStructOnScope>>& Structs)
{
	if(!bIsSelecting)
	{
		TGuardValue<bool> GuardValue(bIsSelecting, true);
		OnHandleStructsSelected().Broadcast(Structs);
	}
}

void FNeTimelineMode::SetDebuggerMode(bool bNewDebuggerMode)
{
	bDebuggerMode = bNewDebuggerMode;
}

void FNeTimelineMode::BuildContextMenu(FMenuBuilder& InMenuBuilder)
{
	TSet<FName> ExistingMenuTypes;
	for(const TSharedRef<FNeTimelineTrack>& SelectedItem : SelectedTracks)
	{
		SelectedItem->AddToContextMenu(InMenuBuilder, ExistingMenuTypes);
	}
}
