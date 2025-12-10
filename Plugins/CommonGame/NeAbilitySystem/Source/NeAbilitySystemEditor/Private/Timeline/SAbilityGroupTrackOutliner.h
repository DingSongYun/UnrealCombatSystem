// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"

struct FNeAbilityTrackGroup;
class FNeAbilityTimelineMode;

/**
 * SAbilityGroupTrackOutliner
 */
class SAbilityGroupTrackOutliner final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAbilityGroupTrackOutliner)
		{}

		SLATE_ARGUMENT(TSharedPtr<SWidget>, MainWidget)
		SLATE_END_ARGS()
public:

	/** Type used for list widget of tracks */
	void Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityTimelineMode>& InTimelineMode, const FNeAbilityTrackGroup& InGroupData);

	// SWidget interface
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End of SWidget interface

protected:

	TWeakPtr<FNeAbilityTimelineMode> WeakTimelineMode;
	const FNeAbilityTrackGroup* GroupData = nullptr;
};
