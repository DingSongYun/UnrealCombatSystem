// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SAbilityGroupTrackOutliner.h"
#include "NeAbilityTrackDragDropOp.h"
#include "NeAbilityTimelineMode.h"
#include "SNeAbilityTimelineTrackOutliner.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "SAbilityGroupTrackOutliner"

void SAbilityGroupTrackOutliner::Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityTimelineMode>& InTimelineMode, const FNeAbilityTrackGroup& InGroupData)
{
	WeakTimelineMode = InTimelineMode;
	GroupData = &InGroupData;

	TSharedPtr<SWidget> MainWidget = InArgs._MainWidget;

	this->ChildSlot
	[
		MainWidget.ToSharedRef()
	];
}

FReply SAbilityGroupTrackOutliner::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	bool bWasDropHandled = false;

	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid())
	{
	}
	else if (Operation->IsOfType<FNeAbilityTrackDragDropOp>())
	{
		const auto& FrameDragDropOp = StaticCastSharedPtr<FNeAbilityTrackDragDropOp>(Operation);

		WeakTimelineMode.Pin()->SwapTrackToGroup(*FrameDragDropOp->SourceTrackData, *GroupData);

		bWasDropHandled = true;
	}
	

	return bWasDropHandled ? FReply::Handled() : FReply::Unhandled();
}


#undef LOCTEXT_NAMESPACE
