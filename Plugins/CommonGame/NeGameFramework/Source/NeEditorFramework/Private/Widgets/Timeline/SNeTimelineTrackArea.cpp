// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/Timeline/SNeTimelineTrackArea.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Types/PaintArgs.h"
#include "Layout/ArrangedChildren.h"
#include "Rendering/DrawElements.h"
#include "Layout/LayoutUtils.h"
#include "Layout/WidgetPath.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Timeline/SNeTimelineTrack.h"
#include "Widgets/Timeline/SNeTimelineOutliner.h"
#include "Widgets/Timeline/NeTimelineTrack.h"
#include "Widgets/Timeline/NeTimelineMode.h"
#include "Widgets/Timeline/NeTimeSliderController.h"

#define LOCTEXT_NAMESPACE "SNeAnimTrackArea"

FTimelineTrackAreaSlot::FTimelineTrackAreaSlot(const TSharedPtr<SNeTimelineTrack>& InSlotContent)
	: TAlignmentWidgetSlotMixin<FTimelineTrackAreaSlot>(HAlign_Fill, VAlign_Top)
{
	TrackWidget = InSlotContent;

	AttachWidget(
		SNew(SWeakWidget)
		.Clipping(EWidgetClipping::ClipToBounds)
		.PossiblyNullContent(InSlotContent)
	);
}

float FTimelineTrackAreaSlot::GetVerticalOffset() const
{
	TSharedPtr<SNeTimelineTrack> PinnedTrackWidget = TrackWidget.Pin();
	return PinnedTrackWidget.IsValid() ? PinnedTrackWidget->GetPhysicalPosition() : 0.f;
}

void SNeTimelineTrackArea::Construct(const FArguments& InArgs, const TSharedRef<FNeTimelineMode>& InTimelineMode, const TSharedRef<FNeTimeSliderController>& InTimeSliderController)
{
	WeakTimelineMode = InTimelineMode;
	WeakTimeSliderController = InTimeSliderController;
}

void SNeTimelineTrackArea::SetOutliner(const TSharedPtr<SNeTimelineOutliner>& InOutliner)
{
	WeakOutliner = InOutliner;
}

void SNeTimelineTrackArea::Empty()
{
	TrackSlots.Empty();
	Children.Empty();
}

void SNeTimelineTrackArea::AddTrackSlot(const TSharedRef<FNeTimelineTrack>& InTrack, const TSharedPtr<SNeTimelineTrack>& InSlot)
{
	TrackSlots.Add(InTrack, InSlot);
	Children.AddSlot(FTimelineTrackAreaSlot::FSlotArguments(MakeUnique<FTimelineTrackAreaSlot>(InSlot)));
}

TSharedPtr<SNeTimelineTrack> SNeTimelineTrackArea::FindTrackSlot(const TSharedRef<FNeTimelineTrack>& InTrack)
{
	// Remove stale entries
	TrackSlots.Remove(TWeakPtr<FNeTimelineTrack>());

	return TrackSlots.FindRef(InTrack).Pin();
}

void SNeTimelineTrackArea::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
	{
		const FTimelineTrackAreaSlot& CurChild = Children[ChildIndex];

		const EVisibility ChildVisibility = CurChild.GetWidget()->GetVisibility();
		if (!ArrangedChildren.Accepts(ChildVisibility))
		{
			continue;
		}

		const FMargin Padding(0, CurChild.GetVerticalOffset(), 0, 0);

		AlignmentArrangeResult XResult = AlignChild<Orient_Horizontal>(AllottedGeometry.GetLocalSize().X, CurChild, Padding, 1.0f, false);
		AlignmentArrangeResult YResult = AlignChild<Orient_Vertical>(AllottedGeometry.GetLocalSize().Y, CurChild, Padding, 1.0f, false);

		ArrangedChildren.AddWidget(ChildVisibility,
			AllottedGeometry.MakeChild(
				CurChild.GetWidget(),
				FVector2D(XResult.Offset, YResult.Offset),
				FVector2D(XResult.Size, YResult.Size)
			)
		);
	}
}

FVector2D SNeTimelineTrackArea::ComputeDesiredSize( float ) const
{
	FVector2D MaxSize(0.0f, 0.0f);
	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
	{
		const FTimelineTrackAreaSlot& CurChild = Children[ChildIndex];

		const EVisibility ChildVisibilty = CurChild.GetWidget()->GetVisibility();
		if (ChildVisibilty != EVisibility::Collapsed)
		{
			FVector2D ChildDesiredSize = CurChild.GetWidget()->GetDesiredSize();
			MaxSize.X = FMath::Max(MaxSize.X, ChildDesiredSize.X);
			MaxSize.Y = FMath::Max(MaxSize.Y, ChildDesiredSize.Y);
		}
	}

	return MaxSize;
}

FChildren* SNeTimelineTrackArea::GetChildren()
{
	return &Children;
}

int32 SNeTimelineTrackArea::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	// paint the child widgets
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);

	const FPaintArgs NewArgs = Args.WithNewParent(this);

	for (int32 ChildIndex = 0; ChildIndex < ArrangedChildren.Num(); ++ChildIndex)
	{
		FArrangedWidget& CurWidget = ArrangedChildren[ChildIndex];
		FSlateRect ChildClipRect = MyCullingRect.IntersectionWith(CurWidget.Geometry.GetLayoutBoundingRect());
		const int32 ThisWidgetLayerId = CurWidget.Widget->Paint(NewArgs, CurWidget.Geometry, ChildClipRect, OutDrawElements, LayerId + 2, InWidgetStyle, bParentEnabled);

		LayerId = FMath::Max(LayerId, ThisWidgetLayerId);
	}

	return LayerId;
}

void SNeTimelineTrackArea::UpdateHoverStates( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{

}

FReply SNeTimelineTrackArea::OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	TSharedPtr<FNeTimeSliderController> TimeSliderController = WeakTimeSliderController.Pin();
	if(TimeSliderController.IsValid())
	{
		WeakOutliner.Pin()->ClearSelection();
		WeakTimelineMode.Pin()->ClearTrackSelection();

		TimeSliderController->OnMouseButtonDown(*this, MyGeometry, MouseEvent);

		//TODO  ref: SSequencerTimeSlider::OnMouseButtonDown for TrackAre drag
		// return FReply::Handled().CaptureMouse(AsShared()).PreventThrottling();
	}

	return FReply::Unhandled();
}

FReply SNeTimelineTrackArea::OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();

		const bool bCloseAfterSelection = true;
		FMenuBuilder MenuBuilder(bCloseAfterSelection, nullptr);

		MenuBuilder.BeginSection("TimelineOptions", LOCTEXT("TimelineOptions", "Timeline Options"));
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("Action_AddTrack_Lable", "Add Track"),
				LOCTEXT("Action_AddTrack_Tooltip", "Add a new track"),
				FNewMenuDelegate::CreateRaw(this, &SNeTimelineTrackArea::AddNewTrack)
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_Sort_Lable", "Sort Track"),
				LOCTEXT("Action_Sort_Tooltip", "Sort Track By startTime."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeTimelineTrackArea::SortTrack)
				),
				NAME_None,
				EUserInterfaceActionType::Button
			);
		}
		MenuBuilder.EndSection();

		FSlateApplication::Get().PushMenu(SharedThis(this), WidgetPath, MenuBuilder.MakeWidget(), FSlateApplication::Get().GetCursorPos(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
		return FReply::Handled();
	}

	WeakTimelineMode.Pin()->ClearTrackSelection();
	TSharedPtr<FNeTimeSliderController> TimeSliderController = WeakTimeSliderController.Pin();
	if(TimeSliderController.IsValid())
	{
		return WeakTimeSliderController.Pin()->OnMouseButtonUp(*this, MyGeometry, MouseEvent);
	}

	return FReply::Unhandled();
}

FReply SNeTimelineTrackArea::OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	UpdateHoverStates(MyGeometry, MouseEvent);

	TSharedPtr<FNeTimeSliderController> TimeSliderController = WeakTimeSliderController.Pin();
	if(TimeSliderController.IsValid())
	{
		FReply Reply = WeakTimeSliderController.Pin()->OnMouseMove(*this, MyGeometry, MouseEvent);

		// Handle right click scrolling on the track area
		if (Reply.IsEventHandled())
		{
			if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton) && HasMouseCapture())
			{
				WeakOutliner.Pin()->ScrollByDelta(-MouseEvent.GetCursorDelta().Y);
			}
		}

		return Reply;
	}

	return FReply::Unhandled();
}

FReply SNeTimelineTrackArea::OnMouseWheel( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	TSharedPtr<FNeTimeSliderController> TimeSliderController = WeakTimeSliderController.Pin();
	if(TimeSliderController.IsValid())
	{
		FReply Reply = WeakTimeSliderController.Pin()->OnMouseWheel(*this, MyGeometry, MouseEvent);
		if (Reply.IsEventHandled())
		{
			return Reply;
		}

		const float ScrollAmount = -MouseEvent.GetWheelDelta() * GetGlobalScrollAmount();
		WeakOutliner.Pin()->ScrollByDelta(ScrollAmount);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SNeTimelineTrackArea::OnMouseLeave(const FPointerEvent& MouseEvent)
{
}

FCursorReply SNeTimelineTrackArea::OnCursorQuery( const FGeometry& MyGeometry, const FPointerEvent& CursorEvent ) const
{
	if (CursorEvent.IsMouseButtonDown(EKeys::RightMouseButton) && HasMouseCapture())
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHandClosed);
	}
	else
	{
		TSharedPtr<FNeTimeSliderController> TimeSliderController = WeakTimeSliderController.Pin();
		if(TimeSliderController.IsValid())
		{
			return TimeSliderController->OnCursorQuery(SharedThis(this), MyGeometry, CursorEvent);
		}
	}

	return FCursorReply::Unhandled();
}

void SNeTimelineTrackArea::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	CachedGeometry = AllottedGeometry;

	for (int32 Index = 0; Index < Children.Num();)
	{
		if (!StaticCastSharedRef<SWeakWidget>(Children[Index].GetWidget())->ChildWidgetIsValid())
		{
			Children.RemoveAt(Index);
		}
		else
		{
			++Index;
		}
	}
}


FReply SNeTimelineTrackArea::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	bool bWasDropHandled = WeakTimelineMode.Pin()->OnDropAction(DragDropEvent);
	return bWasDropHandled ? FReply::Handled() : FReply::Unhandled();
}

FReply SNeTimelineTrackArea::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return FReply::Unhandled();
}

void SNeTimelineTrackArea::AddNewTrack(class FMenuBuilder& MenuBuilder)
{

}

void SNeTimelineTrackArea::SortTrack()
{
	TSharedPtr<FNeTimelineMode> EditorTimelineController = StaticCastSharedPtr<FNeTimelineMode>(WeakTimelineMode.Pin());
	if (EditorTimelineController)
	{
		EditorTimelineController->SortTrackByStartTime();
	}

	FSlateApplication::Get().DismissAllMenus();
}

#undef LOCTEXT_NAMESPACE
