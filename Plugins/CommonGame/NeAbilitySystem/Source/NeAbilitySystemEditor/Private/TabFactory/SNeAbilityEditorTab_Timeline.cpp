// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityEditorTab_Timeline.h"

#include "NeAbility.h"
#include "NeAbilityBlueprintEditor.h"
#include "Timeline/NeAbilityTimelineMode.h"
#include "NeAbilityEditorUtilities.h"
#include "ScopedTransaction.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Timeline/SNeTimeline.h"
#include "Widgets/Timeline/SNeTimelineTransportControls.h"

#define LOCTEXT_NAMESPACE "NeAbilityTimelineSection"

class SNeAbilityTimelineSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeAbilityTimelineSection) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityBlueprintEditor>& InAssetEditorToolkit,
		const TSharedPtr<SNeAbilityEditorTab_Timeline>& AssetEditWidget,
		int32 InSectionIndex)
	{
		AssetEditorToolkit = InAssetEditorToolkit;
		WeakAssetEditTab = AssetEditWidget;
		SectionIndex = InSectionIndex;
		MySection = AssetEditWidget->GetSection(SectionIndex);
		check(MySection)

		// Create timeline widget
		TimelineMode = MakeShared<FNeAbilityTimelineMode>(InAssetEditorToolkit, InSectionIndex, InAssetEditorToolkit->GetToolkitCommands());
		TimelineMode->Initialize();
		TimelineWidget = SNew(SNeTimeline, TimelineMode.ToSharedRef(), InSectionIndex)
			.TransportControl(
				SNew(SNeTimelineTransportControls, StaticCastSharedRef<FNeTimelineMode>(TimelineMode.ToSharedRef()))
				.OnForwardPlay(this, &SNeAbilityTimelineSection::OnClick_Forward)
				.OnForwardStep(this, &SNeAbilityTimelineSection::OnClick_Forward_Step)
				.OnGetLooping(this, &SNeAbilityTimelineSection::IsLooping)
				.OnGetPlaybackMode(this, &SNeAbilityTimelineSection::GetPlaybackMode)
				.OnToggleLooping(this, &SNeAbilityTimelineSection::OnClick_ForwardLooping)
				);

		TSharedPtr<SBorder> OutOuterBorder;
		TSharedPtr<SHorizontalBox> OutInnerHorizontalBox;

		this->ChildSlot
		[
			SAssignNew(ExpandableArea, SExpandableArea)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Padding(1.0f)
			.OnAreaExpansionChanged(this, &SNeAbilityTimelineSection::OnDetailsAreaExpansionChanged)
			.HeaderContent()
			[
				SAssignNew(OutOuterBorder, SBorder)
				// .BorderBackgroundColor_Lambda([PreviewPlayer, InSectionIndex]()
				// {
				// 	if (!PreviewPlayer) return FLinearColor::White;
				// 	else return (PreviewPlayer->GetCurrentSelectSection() == InSectionIndex ||
				// 		PreviewPlayer->GetCurrentPlaySectionIndex() == InSectionIndex) ? FLinearColor::Green: FLinearColor::Gray;
				// })
				// .BorderImage(FAppStyle::GetBrush("Menu.Background"))
				.BorderImage(FAppStyle::GetBrush("NoBorder"))
				.HAlign(EHorizontalAlignment::HAlign_Fill)
				.Padding(2.0f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					[
						SAssignNew(OutInnerHorizontalBox, SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						.Padding(2.0f, 1.0f)
						.AutoWidth()
						[
							SNew(SInlineEditableTextBlock)
							.IsReadOnly(false)
							.ToolTipText(LOCTEXT("SectionName_Tooltip", "Section Name, Click&Drag Change this"))
							.Text_Lambda([this]()
							{
								return FText::FromName(MySection->SectionName);
							})
							.IsSelected_Lambda([this]() { return true; })
							.OnTextCommitted(this, &SNeAbilityTimelineSection::OnCommitSectionName)
						]

					+SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						.AutoWidth()
						.Padding(10,0, 0, 0)
						[
							SNew(SBox)
							.MinDesiredWidth(30.0f)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(SSpinBox<int32>)
								.ToolTipText(LOCTEXT("Duration_Tooltip", "Section Duration"))
								.MinValue(TOptional<int32>(0))
								.MaxValue(TOptional<int32>())
								.MaxSliderValue(TOptional<int32>())
								.MinSliderValue(TOptional<int32>(0))
								.Delta(1)
								.LinearDeltaSensitivity(25)
								//.Style(&FAppStyle::GetWidgetStyle<FSpinBoxStyle>("Sequencer.PlayTimeSpinBox"))
								.Style(&FAppStyle::GetWidgetStyle<FSpinBoxStyle>("Sequencer.HyperlinkSpinBox"))
								.Value_Lambda([this]() -> int32
								{
									return TimelineMode->GetPlayFrameCount();
								})
								.OnValueChanged(this, &SNeAbilityTimelineSection::SetPlayFrame)
								.OnValueCommitted_Lambda([this](int32 InFrame, ETextCommit::Type)
								{
									SetPlayFrame(InFrame);
								})
							]
						]
					]

				+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SAssignNew(SelectSectionBox, SBox)
						.MinDesiredWidth(30.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
					]
				]
			]
			.BodyContent()
			[
				SNew(SBox)
				.MinDesiredHeight(400)
				.Padding(FMargin(0, 2, 0, 2))
				[
					TimelineWidget.ToSharedRef()
				]
			]
		];

		OutInnerHorizontalBox->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.Padding(20.0f, 0.0f)
			[
				FNeAbilityEditorUtilities::MakeTrackButton(LOCTEXT("NeAbility_Section_Btn_Hover", "Actions"), FOnGetContent::CreateSP(this, &SNeAbilityTimelineSection::BuildSectionSubMenu),
				MakeAttributeSP(OutOuterBorder.Get(), &SWidget::IsHovered))
			];


		BindCommands();

		UpdateSection();
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const;

	void BindCommands();

	bool IsExpanded() const;

	void UpdateSection();

private:

	void OnCommitSectionName(const FText& InText, ETextCommit::Type CommitInfo);

	TSharedRef<SWidget> BuildSectionSubMenu();

	void AddSection();
	void DeleteSection();

	void MoveDownSection();
	void MoveUpSection();

	//~=============================================================================
	// Command actions

	void CopySelection();
	void CutSelection();
	void OnPaste();
	void DupilicateSelection();

	void OnDetailsAreaExpansionChanged(bool bExpanded);
	void SetPlayFrame(int32 InFrame);

	//~=============================================================================
	// Play Control
	FReply OnClick_Forward_Step();
	FReply OnClick_Forward();
	FReply OnClick_ForwardLooping();
	EPlaybackMode::Type GetPlaybackMode() const;
	bool IsLooping() const;

public:
	FNeAbilitySection* MySection = nullptr;

	TWeakPtr<FNeAbilityBlueprintEditor> AssetEditorToolkit;
	TWeakPtr<SNeAbilityEditorTab_Timeline> WeakAssetEditTab;

	TSharedPtr<SNeTimeline> TimelineWidget;
	TSharedPtr<class FNeAbilityTimelineMode> TimelineMode;

	TSharedPtr<SExpandableArea> ExpandableArea;

	int32 SectionIndex;
	float LastSplitterValue;

	TSharedPtr<SBox> SelectSectionBox;
	TArray<TSharedPtr<FString>> SharedSelectNames;
};

TSharedRef<SWidget> SNeAbilityTimelineSection::BuildSectionSubMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	// Group Actions
	MenuBuilder.BeginSection("Section Actions", LOCTEXT("MenuSectionS", "Section"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Action_NewSection_Lable", "New Section"),
			LOCTEXT("Action_NewSection_Tooltip", "Add a new section"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::AddSection)),
			NAME_None,
			EUserInterfaceActionType::Button
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Action_DeleteSection_Lable", "Delete Section"),
			LOCTEXT("Action_DeleteSection_Tooltip", "Delete Section"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::DeleteSection)),
			NAME_None,
			EUserInterfaceActionType::Button
		);


		if (SectionIndex != (WeakAssetEditTab.Pin()->GetSectionNum() - 1))
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_MoveDownSection_Lable", "Move Down"),
				LOCTEXT("Action_MoveDownSection_Tooltip", "Move Down"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::MoveDownSection)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
		}

		if (SectionIndex != 0)
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("Action_MoveUpSection_Lable", "Move Up"),
				LOCTEXT("Action_MoveUpSection_Tooltip", "Move Up"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::MoveUpSection)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
		}

	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SNeAbilityTimelineSection::OnCommitSectionName(const FText& InText, ETextCommit::Type CommitInfo)
{
	if (InText.IsEmpty())
		return;

	TimelineMode->SetSectionName(*InText.ToString());
}

void SNeAbilityTimelineSection::AddSection()
{
	WeakAssetEditTab.Pin()->AddNewSection();
}

void SNeAbilityTimelineSection::DeleteSection()
{
	WeakAssetEditTab.Pin()->DeleteSection(SectionIndex);
}

FReply SNeAbilityTimelineSection::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	TSharedPtr<FUICommandList> CommandBindings = TimelineMode->GetCommandList();

	if ( CommandBindings.IsValid() && CommandBindings->ProcessCommandBindings( InKeyEvent ) )
	{
		return FReply::Handled();
	}

	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		TimelineMode->ExitNodePickMode();
	}

	return FReply::Unhandled();
}

FCursorReply SNeAbilityTimelineSection::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	return TimelineMode->IsNodePickMode()
		? FCursorReply::Cursor( EMouseCursor::EyeDropper)
		: FCursorReply::Unhandled();
}

void SNeAbilityTimelineSection::BindCommands()
{
	TSharedPtr<FUICommandList> CommandBindings = TimelineMode->GetCommandList();

	if (CommandBindings == nullptr)
	{
		CommandBindings = MakeShareable(new FUICommandList());
	}
	CommandBindings->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::CopySelection),
		FCanExecuteAction()
	);

	CommandBindings->MapAction(
		FGenericCommands::Get().Cut,
		FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::CutSelection),
		FCanExecuteAction()
	);

	CommandBindings->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::OnPaste),
		FCanExecuteAction()
	);

	CommandBindings->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &SNeAbilityTimelineSection::DupilicateSelection),
		FCanExecuteAction()
	);

}

void SNeAbilityTimelineSection::CopySelection()
{
	// Selection track

	// Selection track node
	// if (TimelineController->GetSelectionTasks().Num())
	// {
	// 	TimelineController->CopySelectionTasks();
	// }
}

void SNeAbilityTimelineSection::CutSelection()
{
}

void SNeAbilityTimelineSection::OnPaste()
{
	TimelineMode->DoPaste();
}

void SNeAbilityTimelineSection::DupilicateSelection()
{
	// if (TimelineController->GetSelectionTasks().Num())
	// {
	// 	TimelineController->DuplicateSelectionTask();
	// }
}

bool SNeAbilityTimelineSection::IsExpanded() const
{
	if (ExpandableArea.IsValid())
	{
		return ExpandableArea->IsExpanded();
	}

	return false;
}

void SNeAbilityTimelineSection::OnDetailsAreaExpansionChanged(bool bExpanded)
{
	if (!WeakAssetEditTab.IsValid())
		return;

	const TSharedPtr<SSplitter>& OutSplitter = WeakAssetEditTab.Pin()->GetSectionContainer();
	if (!OutSplitter.IsValid())
		return;

	SSplitter::FSlot& SSplitterSlot = OutSplitter->SlotAt(SectionIndex);

	if (!bExpanded)
	{
		LastSplitterValue = SSplitterSlot.GetSizeValue();
		SSplitterSlot.SetSizeValue(0.01f);
	}
	else
	{
		SSplitterSlot.SetSizeValue(LastSplitterValue);
	}
}

void SNeAbilityTimelineSection::MoveDownSection()
{
	WeakAssetEditTab.Pin()->MoveDownSection(SectionIndex);
}

void SNeAbilityTimelineSection::MoveUpSection()
{
	WeakAssetEditTab.Pin()->MoveUpSection(SectionIndex);
}

void SNeAbilityTimelineSection::SetPlayFrame(int32 InFrame)
{
	const FScopedTransaction Transaction(LOCTEXT("SNeAbilityTimelineSection", "SetPlayFrame"));
	WeakAssetEditTab.Pin()->MarkAbilityModify();
	TimelineMode->SetSectionDuration(InFrame / TimelineMode->GetFrameRate());
}

void SNeAbilityTimelineSection::UpdateSection()
{
	const int32 NextSectionID = MySection->NextSection > 0 ? MySection->NextSection : 0;

	SharedSelectNames.Reset();
	SharedSelectNames.Add(MakeShared<FString>(TEXT("None")));

	for (int32 i = 0; i < WeakAssetEditTab.Pin()->GetSectionNum(); i++)
	{
		if (i == SectionIndex)
			continue;

		SharedSelectNames.Add(MakeShared<FString>(TimelineMode->GetAbilitySectionPtr()->SectionName.ToString()));
	}

	if (SelectSectionBox.IsValid())
	{
		TSharedPtr<STextComboBox> SelectComboBox = SNew(STextComboBox)
			.ToolTip(SNew(SToolTip).Text(LOCTEXT("JumpSectionToolTip", "next jump section name")))
			.OptionsSource(&SharedSelectNames)
			.OnSelectionChanged_Lambda([&](TSharedPtr<FString> NewValue, ESelectInfo::Type Type) -> void
			{
				if (NewValue.IsValid())
				{
					WeakAssetEditTab.Pin()->LinkSection(SectionIndex, FName((*NewValue.Get())));
				}
			})
			.InitiallySelectedItem(SharedSelectNames[NextSectionID])
			.IsEnabled_Lambda([this]()
			{
				return WeakAssetEditTab.Pin()->GetSectionNum() > 1 ? true : false;
			});

		SelectSectionBox->SetContent(SelectComboBox.ToSharedRef());
	}
}

FReply SNeAbilityTimelineSection::OnClick_Forward_Step()
{
	AssetEditorToolkit.Pin()->StepAbility();
	return FReply::Handled();
}


FReply SNeAbilityTimelineSection::OnClick_Forward()
{
	if (AssetEditorToolkit.IsValid())
	{
		if (AssetEditorToolkit.Pin()->IsPlayingSection(SectionIndex))
		{
			AssetEditorToolkit.Pin()->PauseSection(SectionIndex);
		}
		else
		{
			AssetEditorToolkit.Pin()->PlaySection(SectionIndex);
		}
	}

	return FReply::Handled();
}

FReply SNeAbilityTimelineSection::OnClick_ForwardLooping()
{
	TimelineMode->SetSectionLoop(!TimelineMode->GetSectionLoop());

	return FReply::Handled();
}

EPlaybackMode::Type SNeAbilityTimelineSection::GetPlaybackMode() const
{
	if(AssetEditorToolkit.IsValid() && AssetEditorToolkit.Pin()->IsPlayingSection(SectionIndex))
	{
		return  EPlaybackMode::PlayingForward;
	}

	return EPlaybackMode::Stopped;
}

bool SNeAbilityTimelineSection::IsLooping() const
{
	return TimelineMode->GetSectionLoop();
}

/*****************************************************************
/**
 * SNeAbilityEditorTab_Timeline
 */
void SNeAbilityEditorTab_Timeline::Construct(const FArguments& InArgs, const TSharedPtr<FNeAbilityBlueprintEditor>& InAssetEditorToolkit)
{
	AbilityEditor = InAssetEditorToolkit;
	AbilityAsset = AbilityEditor.Pin()->GetEditingAbility();
	this->ChildSlot
	[
		SNew(SBorder)
		.Visibility(EVisibility::SelfHitTestInvisible)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.Padding(FMargin(0.f, 0.f))
		.ColorAndOpacity(FLinearColor::White)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			.FillSize(1)
			[
				SAssignNew(SectionContainer, SSplitter)
				.Orientation(Orient_Vertical)
				.MinimumSlotHeight(80.0f)
				.ResizeMode(ESplitterResizeMode::FixedSize)
			]
		]
	];
	GenerateSecitonWidgets();
}

void SNeAbilityEditorTab_Timeline::GenerateSecitonWidgets()
{
	const int32 SectionNume = AbilityAsset->GetSectionNums();
	AbilitySectionWidgets.Empty(SectionNume);
	for (int32 SectionIndex = 0; SectionIndex < AbilityAsset->Sections.Num(); SectionIndex++)
	{
		TSharedPtr<SNeAbilityTimelineSection> AblSection = SNew(SNeAbilityTimelineSection, AbilityEditor.Pin(), SharedThis(this), SectionIndex);
		AbilitySectionWidgets.Add(AblSection);
		AddSectionSlot(AblSection);
	}
}

int32 SNeAbilityEditorTab_Timeline::GetSectionNum() const
{
	return AbilityAsset->GetSectionNums();
}

FNeAbilitySection* SNeAbilityEditorTab_Timeline::GetSection(int32 InSectionIndex) const
{
	if (AbilityAsset->Sections.IsValidIndex(InSectionIndex))
	{
		return &AbilityAsset->Sections[InSectionIndex];
	}

	return nullptr;
}

void SNeAbilityEditorTab_Timeline::AddNewSection()
{
}

void SNeAbilityEditorTab_Timeline::DeleteSection(int32 InSectionIndex)
{
}

void SNeAbilityEditorTab_Timeline::MoveDownSection(int32 InSectionIndex)
{
}

void SNeAbilityEditorTab_Timeline::MoveUpSection(int32 InSectionIndex)
{
}

void SNeAbilityEditorTab_Timeline::LinkSection(int32 SectionIndex, int32 NextSectionIndex) const
{
	AbilityAsset->LinkSection(SectionIndex, NextSectionIndex);
}

void SNeAbilityEditorTab_Timeline::LinkSection(int32 SectionIndex, FName NextSectionName) const
{
	AbilityAsset->LinkSection(SectionIndex, NextSectionName);
}

void SNeAbilityEditorTab_Timeline::MarkAbilityModify()
{
	AbilityAsset->Modify();
	AbilityEditor.Pin()->GetBlueprintObj()->Modify();
}

void SNeAbilityEditorTab_Timeline::AddSectionSlot(const TSharedPtr<SNeAbilityTimelineSection>& SectionWidget) const
{
	if (!SectionContainer.IsValid()) return;

	SectionContainer->AddSlot()
		.MinSize_Lambda([SectionWidget]()  //AblSection 最小Area尺寸
		{
			return SectionWidget->IsExpanded() ? 200 : 30;
		})
		.SizeRule(SSplitter::ESizeRule::FractionOfParent)
		[
			SectionWidget.ToSharedRef()
		];
}

#undef LOCTEXT_NAMESPACE
