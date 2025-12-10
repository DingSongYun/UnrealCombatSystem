// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"

class FNeAbilityTimelineTrack;
struct FNeAbilityTrack;
class SExpanderArrow;
class FNeAbilityTimelineMode;
class SBorder;

class SNeAbilityTimelineTrackOutliner : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityTimelineTrackOutliner)
		{}
		SLATE_ATTRIBUTE(FText, DisplayText)
		SLATE_ATTRIBUTE(FText, ToolTipText)
		SLATE_ARGUMENT(TSharedPtr<SWidget>, ExpanderArrow)
		SLATE_ARGUMENT(TSharedPtr<SWidget>, InlineEditableTextBlock)
	SLATE_END_ARGS()
public:

	/** Type used for list widget of tracks */
	void Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityTimelineTrack>& InTimelineTrack);

	// SWidget interface
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End of SWidget interface

private:
	/** Track下拉菜单 */
	TSharedRef<SWidget> BuildMenus() const;

	/** 更改显示名称 */
	void OnCommitName(const FText& InText, ETextCommit::Type CommitInfo);

protected:
	/** 所关联的时间轴轨道 */
	TWeakPtr<FNeAbilityTimelineTrack> OwnerTrack;

	TSharedPtr<SBorder>	TrackPanelArea;

	TSharedPtr<SWidget>	 InlineEditableTextBlock;
	TSharedPtr<SWidget> TrackButton;
};