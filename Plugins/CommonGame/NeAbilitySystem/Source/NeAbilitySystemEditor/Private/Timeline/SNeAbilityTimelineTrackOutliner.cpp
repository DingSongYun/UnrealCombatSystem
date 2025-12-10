// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityTimelineTrackOutliner.h"
#include "NeAbilityTimelineMode.h"
#include "NeAbilityEditorUtilities.h"
#include "NeAbilityTimelineTrack.h"
#include "NeAbilityTrackDragDropOp.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "TabFactory/SNeAbilityEditorTab_Palette.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SNeAbilityTimelineTrackOutliner"

void SNeAbilityTimelineTrackOutliner::Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityTimelineTrack>& InTimelineTrack)
{
	OwnerTrack = InTimelineTrack;

	InlineEditableTextBlock = InArgs._InlineEditableTextBlock;

	const float LeftPadding = OwnerTrack.Pin()->IsChildTrack() ? 20.0f : 4.0f;

	this->ChildSlot
	[
		SNew(SBorder)
		.ToolTipText(InArgs._ToolTipText)
		.BorderImage(FAppStyle::GetBrush("Sequencer.Section.BackgroundTint"))
		.BorderBackgroundColor(FAppStyle::GetColor("AnimTimeline.Outliner.ItemColor"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(LeftPadding, 1.0f)
			[
				InArgs._ExpanderArrow.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SInlineEditableTextBlock)
				.Text(InArgs._DisplayText)
				.IsSelected(FIsSelected::CreateLambda([]() { return true; }))
				.OnTextCommitted(this, &SNeAbilityTimelineTrackOutliner::OnCommitName)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			.Padding(8.0f, 1.0f)
			[
				!OwnerTrack.Pin()->GetAbilityTimelineMode()->IsDebuggerMode()
				? FNeAbilityEditorUtilities::MakeTrackButton(LOCTEXT("AddTrackButtonText", "Track"),
					FOnGetContent::CreateSP(this, &SNeAbilityTimelineTrackOutliner::BuildMenus),
					MakeAttributeSP(this, &SWidget::IsHovered))
				: SNullWidget::NullWidget
			]
		]
	];

}

FReply SNeAbilityTimelineTrackOutliner::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !OwnerTrack.Pin()->GetAbilityTimelineMode()->IsDebuggerMode())
	{
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

FReply SNeAbilityTimelineTrackOutliner::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		TSharedRef<SOverlay> NodeDragDecoratorOverlay = SNew(SOverlay);
		TSharedRef<SBorder> NodeDragDecorator = SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(.0f, 1.f, .0f))
			[
				NodeDragDecoratorOverlay
			];

		FVector2D OffsetFromFirst(2, 2);

		NodeDragDecoratorOverlay->AddSlot()
			.Padding(FMargin(OffsetFromFirst.X, OffsetFromFirst.Y, 0.0f, 0.0f))
			[
				SharedThis(this)
			];


		return FReply::Handled().BeginDragDrop(FNeAbilityTrackDragDropOp::New(&OwnerTrack.Pin()->GetTrackData(), OwnerTrack.Pin()->IsChildTrack(), NodeDragDecorator));
	}

	return FReply::Unhandled();
}

FReply SNeAbilityTimelineTrackOutliner::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return OwnerTrack.Pin()->OnDropToTrack(MyGeometry, DragDropEvent);
}

TSharedRef<SWidget> SNeAbilityTimelineTrackOutliner::BuildMenus() const
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("Tracks", LOCTEXT("NotifiesMenuSection", "Option"));
	{
		if (!OwnerTrack.Pin()->IsChildTrack() && OwnerTrack.Pin()->HasAnyCompoundNode())
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("Action_AddTrack_Lable", "Add Child Track"),
				LOCTEXT("Action_AddTrack_Tooltip", "Add a new child track"),
				FNewMenuDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
				{
					FNeAbilityEditorUtilities::MakeNewSegmentPicker(
						OwnerTrack.Pin()->GetAbilityTimelineMode()->GetAbilityEditor().ToSharedRef(),
						MenuBuilder,
						FOnClassPicked::CreateLambda([this](UClass* InClass)
						{
							OwnerTrack.Pin()->AddNewChildTrack(InClass);
						}));
				})
			);
		}

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleTrackNodes_Track", "Remove Track"),
			LOCTEXT("ToggleTrackNodes_Track", "remove a  track"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([this]()
				{
					OwnerTrack.Pin()->RemoveTrack();
				})
			),
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}

	return MenuBuilder.MakeWidget();
}

void SNeAbilityTimelineTrackOutliner::OnCommitName(const FText& InText, ETextCommit::Type CommitInfo)
{
	const FText TrimText = FText::TrimPrecedingAndTrailing(InText);
	OwnerTrack.Pin()->ChangeTrackName(TrimText.ToString());
}


#undef LOCTEXT_NAMESPACE
