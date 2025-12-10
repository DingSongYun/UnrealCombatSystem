// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityTimelineGroupTrack.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Timeline/SNeTimelineOutlinerItem.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "NeAbilityEditorUtilities.h"
#include "NeAbilitySection.h"
#include "SAbilityGroupTrackOutliner.h"
#include "NeAbilityTimelineMode.h"

#define LOCTEXT_NAMESPACE "FNeTimelineTrack_GroupTrack "

ANIMTIMELINE_IMPLEMENT_TRACK(FNeAbilityTimelineGroupTrack);

FNeAbilityTimelineGroupTrack ::FNeAbilityTimelineGroupTrack (const TSharedPtr<FNeAbilityTimelineMode>& InMode, const FNeAbilityTrackGroup& InGroupData, const FText& InDisplayName, const FText& InToolTipText)
	: FNeTimelineTrack(StaticCastSharedPtr<FNeTimelineMode>(InMode), InDisplayName, InToolTipText, true)
	, GroupData(InGroupData)
{
	SetHeight(25.0f);
}

TSharedRef<SWidget> FNeAbilityTimelineGroupTrack ::GenerateContainerWidgetForOutliner(const TSharedRef<SNeTimelineOutlinerItem>& InRow)
{
	TSharedPtr<SBorder> OuterBorder;
	TSharedPtr<SHorizontalBox> InnerHorizontalBox;
	const TSharedRef<SWidget> OutlinerWidget = GenerateStandardOutlinerWidget(InRow, false, OuterBorder, InnerHorizontalBox);

	OuterBorder->SetBorderBackgroundColor(FAppStyle::GetColor("AnimTimeline.Outliner.HeaderColor"));

	// Lable
	InnerHorizontalBox->AddSlot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		.Padding(2.0f, 1.0f)
		.FillWidth(1.0f)
		[
			SAssignNew(EditableGroupName, SInlineEditableTextBlock)
			.IsReadOnly(false)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			.Text(this, &FNeAbilityTimelineGroupTrack ::GetLabel)
			.IsSelected_Lambda([this]() { return TimelineMode.Pin()->IsTrackSelected(SharedThis(this)); })
			.OnTextCommitted(this, &FNeAbilityTimelineGroupTrack ::OnCommitGroupName)
			.HighlightText(InRow->GetHighlightText())
		];

	// Action
	if (!TimelineMode.Pin()->IsDebuggerMode())
	{
		InnerHorizontalBox->AddSlot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			.Padding(OutlinerRightPadding, 2.0f)
			[
				FNeAbilityEditorUtilities::MakeTrackButton(LOCTEXT("NeAbility_TrackGroup_Btn_Hover", "Actions"),
				FOnGetContent::CreateSP(this, &FNeAbilityTimelineGroupTrack ::BuildGroupSubMenu),
				MakeAttributeSP(this, &FNeAbilityTimelineGroupTrack ::IsHovered))
			];
	}

	return SNew(SAbilityGroupTrackOutliner, GetAbilityTimelineMode(), GroupData)
		.MainWidget(OutlinerWidget);
}

TSharedPtr<FNeAbilityTimelineMode> FNeAbilityTimelineGroupTrack::GetAbilityTimelineMode() const
{
	return StaticCastSharedPtr<FNeAbilityTimelineMode>(TimelineMode.Pin());
}

TSharedRef<SWidget> FNeAbilityTimelineGroupTrack ::BuildGroupSubMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	// Group Actions
	MenuBuilder.BeginSection("Group Actions", LOCTEXT("NotifiesMenuSection", "Group"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Action_NewGroup_Lable", "New Group"),
			LOCTEXT("Action_NewGroup_Tooltip", "Add a new track group"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FNeAbilityTimelineGroupTrack ::AddTrackGroup)),
			NAME_None,
			EUserInterfaceActionType::Button
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Action_DeleteGroup_Lable", "Delete Group"),
			LOCTEXT("Action_DeleteGroup_Tooltip", "Delete group"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FNeAbilityTimelineGroupTrack ::DeleteTrackGroup)),
			NAME_None,
			EUserInterfaceActionType::Button
		);

	}
	MenuBuilder.EndSection();

	// Track Actions
	MenuBuilder.BeginSection("Track Actions", LOCTEXT("NotifiesMenuSection", "Tracks"));
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("Action_AddTrack_Lable", "Add Track"),
			LOCTEXT("Action_AddTrack_Tooltip", "Add a new track"),
			FNewMenuDelegate::CreateLambda([this] (class FMenuBuilder& MenuBuilder)
			{
				FNeAbilityEditorUtilities::MakeNewSegmentPicker(
					GetAbilityTimelineMode()->GetAbilityEditor().ToSharedRef(),
					MenuBuilder,
					FOnClassPicked::CreateSP(this, &FNeAbilityTimelineGroupTrack::AddNewTrack));
			})
			// FNewMenuDelegate::CreateRaw( this, &FNeAbilityTimelineGroupTrack ::FillNewTrackMenu)
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FNeAbilityTimelineGroupTrack ::AddNewTrack(UClass* InActionClass) const
{
	GetAbilityTimelineMode()->AddNewTrack(GroupData, InActionClass);
}

void FNeAbilityTimelineGroupTrack::AddTrackGroup() const
{
	GetAbilityTimelineMode()->AddNewTrackGroup("New Group");
}

void FNeAbilityTimelineGroupTrack::DeleteTrackGroup() const
{
	GetAbilityTimelineMode()->DeleteTrackGroup(GroupData);
}

void FNeAbilityTimelineGroupTrack ::RequestRename()
{
	if (EditableGroupName.IsValid())
	{
		EditableGroupName->EnterEditingMode();
	}
}

FText FNeAbilityTimelineGroupTrack ::GetLabel() const
{
	return FText::FromName(GroupData.GroupName);
}

void FNeAbilityTimelineGroupTrack ::OnCommitGroupName(const FText& InText, ETextCommit::Type CommitInfo) const
{
	if (TimelineMode.Pin()->IsDebuggerMode())
		return;

	GetAbilityTimelineMode()->ChangeTrackGroupName(GroupData, *InText.ToString());
}

#undef LOCTEXT_NAMESPACE