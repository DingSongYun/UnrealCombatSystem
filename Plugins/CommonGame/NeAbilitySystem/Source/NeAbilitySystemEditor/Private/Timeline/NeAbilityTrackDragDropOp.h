// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "Input/DragAndDrop.h"

class FNeAbilityTimelineMode;
struct FNeAbilityTrack;

class FNeAbilityTrackDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FNeAbilityTrackDragDropOp, FDragDropOperation)

	FNeAbilityTrack* SourceTrackData;
	bool bSourceChildTrack;
	TSharedPtr<SWidget>	Decorator;				// The widget to display when dragging

	// FDragDropOperation interface
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
	virtual void OnDragged(const class FDragDropEvent& DragDropEvent) override;
	virtual void Construct() override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End of FDragDropOperation interface

	void SetCanDropHere(bool bCanDropHere)
	{
		MouseCursor = bCanDropHere ? EMouseCursor::TextEditBeam : EMouseCursor::SlashedCircle;
	}

	static TSharedRef<FNeAbilityTrackDragDropOp> New(FNeAbilityTrack* InTrackData, bool bSourceChildTrack, TSharedPtr<SWidget> Decorator);

protected:
	FNeAbilityTrackDragDropOp();
};
