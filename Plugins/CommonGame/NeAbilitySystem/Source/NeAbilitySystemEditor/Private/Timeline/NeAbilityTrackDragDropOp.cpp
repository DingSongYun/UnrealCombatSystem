// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityTrackDragDropOp.h"
#include "NeAbilityTimelineMode.h"
#include "Widgets/SWindow.h"

FNeAbilityTrackDragDropOp::FNeAbilityTrackDragDropOp()
{
}

TSharedPtr<SWidget> FNeAbilityTrackDragDropOp::GetDefaultDecorator() const
{
	return Decorator;
}

void FNeAbilityTrackDragDropOp::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	if (CursorDecoratorWindow.IsValid())
	{
		CursorDecoratorWindow->MoveWindowTo(DragDropEvent.GetScreenSpacePosition());
	}
}

void FNeAbilityTrackDragDropOp::Construct()
{
	MouseCursor = EMouseCursor::GrabHandClosed;

	FDragDropOperation::Construct();
}

void FNeAbilityTrackDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	if (!bDropWasHandled)
	{
	}
}

TSharedRef<FNeAbilityTrackDragDropOp> FNeAbilityTrackDragDropOp::New(FNeAbilityTrack* InTrackData, bool bSourceChildTrack, TSharedPtr<SWidget> Decorator)
{
	TSharedRef<FNeAbilityTrackDragDropOp> Operation = MakeShareable(new FNeAbilityTrackDragDropOp);
	Operation->SourceTrackData = InTrackData;
	Operation->bSourceChildTrack = bSourceChildTrack;
	Operation->Decorator = Decorator;
	Operation->Construct();

	return Operation;
}
