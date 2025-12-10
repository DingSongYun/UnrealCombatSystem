// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/Timeline/SNeTimeline.h"
#include "Styling/ISlateStyle.h"
#include "Widgets/SWidget.h"
#include "Widgets/Timeline/SNeTimelineOutliner.h"
#include "Widgets/Timeline/SNeTimelineTrackArea.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Timeline/SNeTimelineOverlay.h"
#include "Widgets/Timeline/SNeTimelineSplitterOverlay.h"
#include "ISequencerWidgetsModule.h"
#include "FrameNumberNumericInterface.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Fonts/FontMeasure.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Modules/ModuleManager.h"
#include "Preferences/PersonaOptions.h"
#include "AnimatedRange.h"
#include "SequencerWidgetsDelegates.h"
#include "TimeSliderArgs.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/STextEntryPopup.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Timeline/SNeTimelineTransportControls.h"
#include "Animation/AnimSequence.h"
#include "Widgets/Timeline/NeTimeSliderController.h"
#include "Widgets/Timeline/NeTimelineMode.h"
#include "MovieSceneFwd.h"

#define LOCTEXT_NAMESPACE "SNeAnimTimeline"

const FName SNeTimeline::InSnapName = TEXT("SNeTimeline");

void SNeTimeline::Construct(const FArguments& InArgs, const TSharedRef<FNeTimelineMode>& InTimelineMode, int32 InSectionIndex)
{
	TWeakPtr<FNeTimelineMode> WeakModel = InTimelineMode;

	TimelineMode = InTimelineMode;
	OnReceivedFocus = InArgs._OnReceivedFocus;

	SectionIndex = InSectionIndex;

	/*if (InModel->GetPreviewScene()->GetPreviewMeshComponent()->PreviewInstance)
	{
		InModel->GetPreviewScene()->GetPreviewMeshComponent()->PreviewInstance->AddKeyCompleteDelegate(FSimpleDelegate::CreateSP(this, &SNeAnimTimeline::HandleKeyComplete));
	}*/

	ViewRange = MakeAttributeLambda([this](){ return TimelineMode.IsValid() ? TimelineMode.Pin()->GetViewRange() : FAnimatedRange(0.0, 0.0); });

	TAttribute<EFrameNumberDisplayFormats> DisplayFormat = MakeAttributeLambda([](){ return GetDefault<UPersonaOptions>()->TimelineDisplayFormat; });

	TAttribute<EFrameNumberDisplayFormats> DisplayFormatSecondary = MakeAttributeLambda([]()
	{
		return GetDefault<UPersonaOptions>()->TimelineDisplayFormat == EFrameNumberDisplayFormats::Frames ? EFrameNumberDisplayFormats::Seconds : EFrameNumberDisplayFormats::Frames;
	});
	TAttribute<FFrameRate> TickResolution = MakeAttributeLambda([this](){ return FFrameRate(TimelineMode.Pin()->GetTickResolution(), 1); });
	TAttribute<FFrameRate> DisplayRate = MakeAttributeLambda([this]() {return FFrameRate(FMath::RoundToInt(TimelineMode.Pin()->GetFrameRate()), 1);});

	// Create our numeric type interface so we can pass it to the time slider below.
	NumericTypeInterface = MakeShareable(new FFrameNumberInterface(DisplayFormat, 0, TickResolution, DisplayRate));
	SecondaryNumericTypeInterface = MakeShareable(new FFrameNumberInterface(DisplayFormatSecondary, 0, TickResolution, DisplayRate));

	FTimeSliderArgs TimeSliderArgs;
	{
		TimeSliderArgs.ScrubPosition = MakeAttributeLambda([this](){ return TimelineMode.IsValid() ? TimelineMode.Pin()->GetScrubPosition() : FFrameTime(0); });
		TimeSliderArgs.ViewRange = ViewRange;
		TimeSliderArgs.PlaybackRange = MakeAttributeLambda([this](){ return TimelineMode.IsValid() ? TimelineMode.Pin()->GetPlaybackRange() : TRange<FFrameNumber>(0, 0); });
		TimeSliderArgs.ClampRange = MakeAttributeLambda([this](){ return TimelineMode.IsValid() ? TimelineMode.Pin()->GetWorkingRange() : FAnimatedRange(0.0, 0.0); });
		TimeSliderArgs.DisplayRate = DisplayRate;
		TimeSliderArgs.TickResolution = TickResolution;
		TimeSliderArgs.OnViewRangeChanged = FOnViewRangeChanged::CreateSP(&InTimelineMode.Get(), &FNeTimelineMode::HandleViewRangeChanged);
		TimeSliderArgs.OnClampRangeChanged = FOnTimeRangeChanged::CreateSP(&InTimelineMode.Get(), &FNeTimelineMode::HandleWorkingRangeChanged);
		TimeSliderArgs.IsPlaybackRangeLocked = true;
		TimeSliderArgs.PlaybackStatus = EMovieScenePlayerStatus::Stopped;
		TimeSliderArgs.NumericTypeInterface = NumericTypeInterface;
		TimeSliderArgs.OnScrubPositionChanged = FOnScrubPositionChanged::CreateSP(this, &SNeTimeline::HandleScrubPositionChanged);
	}

	TimeSliderController = MakeShareable(new FNeTimeSliderController(TimeSliderArgs, InTimelineMode, SharedThis(this), SecondaryNumericTypeInterface));
	
	TSharedRef<FNeTimeSliderController> TimeSliderControllerRef = TimeSliderController.ToSharedRef();

	// Create the top slider
	const bool bMirrorLabels = false;
	ISequencerWidgetsModule& SequencerWidgets = FModuleManager::Get().LoadModuleChecked<ISequencerWidgetsModule>("SequencerWidgets");
	TopTimeSlider = SequencerWidgets.CreateTimeSlider(TimeSliderControllerRef, bMirrorLabels);

	// Create bottom time range slider
	TSharedRef<ITimeSlider> BottomTimeRange = SequencerWidgets.CreateTimeRange(
		FTimeRangeArgs(
			EShowRange::ViewRange | EShowRange::WorkingRange | EShowRange::PlaybackRange,
			EShowRange::ViewRange | EShowRange::WorkingRange,
			TimeSliderControllerRef,
			EVisibility::Visible,
			NumericTypeInterface.ToSharedRef()
		),
		SequencerWidgets.CreateTimeRangeSlider(TimeSliderControllerRef)
	);

	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5.0f, 5.0f));

	InTimelineMode->RefreshTracks();

	TrackArea = SNew(SNeTimelineTrackArea, InTimelineMode, TimeSliderControllerRef);
	Outliner = SNew(SNeTimelineOutliner, InTimelineMode, TrackArea.ToSharedRef())
		.ExternalScrollbar(ScrollBar)
		.Clipping(EWidgetClipping::ClipToBounds)
		.FilterText_Lambda([this](){ return FilterText; });

	TrackArea->SetOutliner(Outliner);

	ColumnFillCoefficients[0] = 0.2f;
	ColumnFillCoefficients[1] = 0.8f;

	TAttribute<float> FillCoefficient_0, FillCoefficient_1;
	{
		FillCoefficient_0.Bind(TAttribute<float>::FGetter::CreateSP(this, &SNeTimeline::GetColumnFillCoefficient, 0));
		FillCoefficient_1.Bind(TAttribute<float>::FGetter::CreateSP(this, &SNeTimeline::GetColumnFillCoefficient, 1));
	}

	const int32 Column0 = 0, Column1 = 1;
	const int32 Row0 = 0, Row1 = 1, Row2 = 2, Row3 = 3, Row4 = 4;

	const float CommonPadding = 3.f;
	const FMargin ResizeBarPadding(4.f, 0, 0, 0);

	ChildSlot
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SNew(SBorder)
						.BorderImage( FAppStyle::GetBrush("ToolPanel.GroupBorder") )
						.BorderBackgroundColor( FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
				]
				+SOverlay::Slot()
				[
					SNew(SGridPanel)
					.FillRow(1, 1.0f)
					.FillColumn(0, FillCoefficient_0)
					.FillColumn(1, FillCoefficient_1)

					//左上角, 播放控制按钮
					+SGridPanel::Slot(Column0, Row0, SGridPanel::Layer(10))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.FillWidth(1.0f)
						.Padding(0)
						[
							SNew(SBox)
							.MaxDesiredHeight(25)
							[
								InArgs._TransportControl ? InArgs._TransportControl.ToSharedRef() : SNew(SNeTimelineTransportControls, InTimelineMode)
							]
						]

						+SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Right)
						.AutoWidth()
						.Padding(0)
						[
							SNew(SBox)
							.MinDesiredWidth(30.0f)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								// Current Play Time 
								SNew(SSpinBox<double>)
								.Style(&FAppStyle::GetWidgetStyle<FSpinBoxStyle>("Sequencer.PlayTimeSpinBox"))
								.Value_Lambda([this]() -> double
								{
									return TimelineMode.Pin()->GetScrubPosition().Value;
								})
								.OnValueChanged(this, &SNeTimeline::SetPlayTime)
								.OnValueCommitted_Lambda([this](double InFrame, ETextCommit::Type)
								{
									SetPlayTime(InFrame);
								})
								.MinValue(TOptional<double>())
								.MaxValue(TOptional<double>())
								.TypeInterface(NumericTypeInterface)
								.Delta(this, &SNeTimeline::GetSpinboxDelta)
								.LinearDeltaSensitivity(25)
							]
						]
					]
	
					//左侧Track面板+ 中间timeline track
					// main timeline area
					+SGridPanel::Slot(Column0, Row1, SGridPanel::Layer(10))
					.ColumnSpan(2)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							SNew(SOverlay)
							+SOverlay::Slot()
							[
								SNew(SScrollBorder, Outliner.ToSharedRef())
								[
									SNew(SHorizontalBox)

									// outliner tree 左侧Track面板
									// outliner tree
									+SHorizontalBox::Slot()
									.FillWidth(FillCoefficient_0)
									[
										SNew(SBox)
										[
											Outliner.ToSharedRef()
										]
									]

									// track area
									+SHorizontalBox::Slot()
									.FillWidth(FillCoefficient_1)
									[
										SNew(SBox)
										.Padding(ResizeBarPadding)
										.Clipping(EWidgetClipping::ClipToBounds)
										[
											TrackArea.ToSharedRef()
										]
									]
								]
							]

							+SOverlay::Slot()
							.HAlign(HAlign_Right)
							[
								ScrollBar
							]
						]
					]

					//左下角 播放/暂停 操作按钮
					 //Transport controls
					/*+SGridPanel::Slot(Column0, Row3, SGridPanel::Layer(10))
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SNeAnimTimelineTransportControls, InEditorTimelineController->GetBattleSystemEditor())
					]*/

					//最上面的时间刻度
					// Second column
					+SGridPanel::Slot(Column1, Row0)
					.Padding(ResizeBarPadding)
					.RowSpan(2)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SSpacer)
						]
					]

					+SGridPanel::Slot(Column1, Row0, SGridPanel::Layer(10))
					.Padding(ResizeBarPadding)
					[
						SNew( SBorder )
						.BorderImage( FAppStyle::GetBrush("ToolPanel.GroupBorder") )
						.BorderBackgroundColor( FLinearColor(.50f, .50f, .50f, 1.0f ) )
						.Padding(0)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							TopTimeSlider.ToSharedRef()
						]
					]

					//最左+最右 帧竖线
					// Overlay that draws the tick lines
					+SGridPanel::Slot(Column1, Row1, SGridPanel::Layer(10))
					.Padding(ResizeBarPadding)
					[
						SNew(SNeTimelineOverlay, TimeSliderControllerRef)
						.Visibility( EVisibility::HitTestInvisible )
						.DisplayScrubPosition( false )
						.DisplayTickLines( true )
						.Clipping(EWidgetClipping::ClipToBounds)
						.PaintPlaybackRangeArgs(FPaintPlaybackRangeArgs(FAppStyle::GetBrush("Sequencer.Timeline.PlayRange_L"), FAppStyle::GetBrush("Sequencer.Timeline.PlayRange_R"), 6.f))
					]

					//--中间红色 可拖动的帧竖线
					// Overlay that draws the scrub position
					+SGridPanel::Slot(Column1, Row1, SGridPanel::Layer(20))
					.Padding(ResizeBarPadding)
					[
						SNew(SNeTimelineOverlay, TimeSliderControllerRef)
						.Visibility( EVisibility::HitTestInvisible )
						.DisplayScrubPosition( true )
						.DisplayTickLines( false )
						.Clipping(EWidgetClipping::ClipToBounds)
					]

					//=下方滑动条
					// play range slider
					+SGridPanel::Slot(Column1, Row3, SGridPanel::Layer(10))
					.Padding(ResizeBarPadding)
					[
						SNew(SBorder)
						.BorderImage( FAppStyle::GetBrush("ToolPanel.GroupBorder") )
						.BorderBackgroundColor( FLinearColor(0.5f, 0.5f, 0.5f, 1.0f ) )
						.Clipping(EWidgetClipping::ClipToBounds)
						.Padding(0)
						[
							BottomTimeRange
						]
					]
				]
				+SOverlay::Slot()
				[
					// track area virtual splitter overlay
					SNew(SNeTimelineSplitterOverlay)
					.Style(FAppStyle::Get(), "AnimTimeline.Outliner.Splitter")
					.Visibility(EVisibility::SelfHitTestInvisible)

					+ SSplitter::Slot()
					.Value(FillCoefficient_0)
					.OnSlotResized(SSplitter::FOnSlotResized::CreateSP(this, &SNeTimeline::OnColumnFillCoefficientChanged, 0))
					[
						SNew(SSpacer)
					]

					+ SSplitter::Slot()
					.Value(FillCoefficient_1)
					.OnSlotResized(SSplitter::FOnSlotResized::CreateSP(this, &SNeTimeline::OnColumnFillCoefficientChanged, 1))
					[
						SNew(SSpacer)
					]
				]
			]
		]
	];
}

FReply SNeTimeline::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();

		const bool bCloseAfterSelection = true;
		FMenuBuilder MenuBuilder(bCloseAfterSelection, nullptr);

		MenuBuilder.BeginSection("TimelineOptions", LOCTEXT("TimelineOptions", "Timeline Options"));
		{

			MenuBuilder.AddMenuEntry
			(
				LOCTEXT("TimeFormat_SnapText", "SnapMode"),
				LOCTEXT("TimeFormat_SnapText", "Display the time SnapMode"),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([this]()
					{
						this->ToggleTimelineEnabledSnaps();
					}),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this]() { return this->IsDisplayTimelineEnabledSnapsChecked(); })
				),
				NAME_None,
				EUserInterfaceActionType::RadioButton
				);

			MenuBuilder.AddMenuEntry
			(
				LOCTEXT("TimeFormat_PlayFinish_Reset", "PlayEnd_Reset"),
				LOCTEXT("TimeFormat_PlayFinish_Reset", "when play finished will reset world"),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([this]()
					{
					}),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this]()
					{
						return true;
					})
				),
				NAME_None,
				EUserInterfaceActionType::RadioButton
				);


			MenuBuilder.AddSubMenu(
				LOCTEXT("TimeFormat", "Time Format"),
				LOCTEXT("TimeFormatTooltip", "Choose the format of times we display in the timeline"),
				FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InMenuBuilder)
			{
				InMenuBuilder.BeginSection("TimeFormat", LOCTEXT("TimeFormat", "Time Format"));
				{
					InMenuBuilder.AddMenuEntry
					(
						LOCTEXT("TimeFormat_SecondsText", "Seconds"),
						LOCTEXT("TimeFormat_SecondsText", "Display the time in seconds"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([this]() 
							{
								this->SetDisplayFormat(EFrameNumberDisplayFormats::Seconds);
							}),
							FCanExecuteAction(),
							FIsActionChecked::CreateLambda([this]() { return this->IsDisplayFormatChecked(EFrameNumberDisplayFormats::Seconds); })
						),
						NAME_None,
						EUserInterfaceActionType::RadioButton
					);

					InMenuBuilder.AddMenuEntry
					(
						LOCTEXT("TimeFormat_FrameText", "Frames"),
						LOCTEXT("TimeFormat_FrameText", "Display the time in frames"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([this]()
							{
								this->SetDisplayFormat(EFrameNumberDisplayFormats::Frames);
							}),
							FCanExecuteAction(),
							FIsActionChecked::CreateLambda([this]() { return this->IsDisplayFormatChecked(EFrameNumberDisplayFormats::Frames); })
							),
							NAME_None,
							EUserInterfaceActionType::RadioButton
						);
				}
				InMenuBuilder.EndSection();

				InMenuBuilder.BeginSection("TimelineAdditional", LOCTEXT("TimelineAdditional", "Additional Display"));
				{
					InMenuBuilder.AddMenuEntry
					(
						LOCTEXT("TimeFormat_PercentageText", "Percentage"),
						LOCTEXT("TimeFormat_PercentageText", "Display the percentage along with the time with the scrubber"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([this]()
							{
								this->ToggleDisplayPercentage();
							}),
							FCanExecuteAction(),
							FIsActionChecked::CreateLambda([this]() { return this->IsDisplayPercentageChecked(); })
						),
						NAME_None,
						EUserInterfaceActionType::RadioButton
					);

					InMenuBuilder.AddMenuEntry
					(
						LOCTEXT("TimeFormat_SecondaryText", "Secondary"),
						LOCTEXT("TimeFormat_SecondaryText", "Display the time or frame count as a secondary format along with the scrubber"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([this]()
							{
								this->ToggleDisplaySecondary();
							}),
							FCanExecuteAction(),
							FIsActionChecked::CreateLambda([this]() { return this->IsDisplaySecondaryChecked(); })
						),
						NAME_None,
						EUserInterfaceActionType::RadioButton
					);
				}
				InMenuBuilder.EndSection();
			})
			);
		}
		MenuBuilder.EndSection();

		FSlateApplication::Get().PushMenu(SharedThis(this), WidgetPath, MenuBuilder.MakeWidget(), FSlateApplication::Get().GetCursorPos(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

void SNeTimeline::OnCropAnimSequence( bool bFromStart, float CurrentTime )
{
	/*
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if(PreviewInstance)
	{
		float Length = PreviewInstance->GetLength();
		if (PreviewInstance->GetCurrentAsset())
		{
			UAnimSequence* AnimSequence = Cast<UAnimSequence>( PreviewInstance->GetCurrentAsset() );
			if( AnimSequence )
			{
				const FScopedTransaction Transaction( LOCTEXT("CropAnimSequence", "Crop Animation Sequence") );

				//Call modify to restore slider position
				PreviewInstance->Modify();

				//Call modify to restore anim sequence current state
				AnimSequence->Modify();

				// Crop the raw anim data.
				AnimSequence->CropRawAnimData( CurrentTime, bFromStart );

				//Resetting slider position to the first frame
				PreviewInstance->SetPosition( 0.0f, false );

				Model.Pin()->RefreshTracks();
			}
		}
	}
	*/
}

void SNeTimeline::OnAppendAnimSequence( bool bFromStart, int32 NumOfFrames )
{
	/*
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if(PreviewInstance && PreviewInstance->GetCurrentAsset())
	{
		UAnimSequence* AnimSequence = Cast<UAnimSequence>(PreviewInstance->GetCurrentAsset());
		if(AnimSequence)
		{
			const FScopedTransaction Transaction(LOCTEXT("InsertAnimSequence", "Insert Animation Sequence"));

			//Call modify to restore slider position
			//PreviewInstance->Modify();

			//Call modify to restore anim sequence current state
		//	AnimSequence->Modify();

			// Crop the raw anim data.
			int32 StartFrame = (bFromStart)? 0 : AnimSequence->GetRawNumberOfFrames() - 1;
			int32 EndFrame = StartFrame + NumOfFrames;
			int32 CopyFrame = StartFrame;
			AnimSequence->InsertFramesToRawAnimData(StartFrame, EndFrame, CopyFrame);

			Model.Pin()->RefreshTracks();
		}
	}
	*/
}

void SNeTimeline::OnInsertAnimSequence( bool bBefore, int32 CurrentFrame )
{
	/*
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if(PreviewInstance && PreviewInstance->GetCurrentAsset())
	{
		UAnimSequence* AnimSequence = Cast<UAnimSequence>(PreviewInstance->GetCurrentAsset());
		if(AnimSequence)
		{
			const FScopedTransaction Transaction(LOCTEXT("InsertAnimSequence", "Insert Animation Sequence"));

			//Call modify to restore slider position
			//PreviewInstance->Modify();

			//Call modify to restore anim sequence current state
			//AnimSequence->Modify();

			// Crop the raw anim data.
			int32 StartFrame = (bBefore)? CurrentFrame : CurrentFrame + 1;
			int32 EndFrame = StartFrame + 1;
			AnimSequence->InsertFramesToRawAnimData(StartFrame, EndFrame, CurrentFrame);

			Model.Pin()->RefreshTracks();
		}
	}
	*/
}

void SNeTimeline::OnReZeroAnimSequence(int32 FrameIndex)
{
	/*
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if(PreviewInstance)
	{
		UDebugSkelMeshComponent* PreviewSkelComp =  Model.Pin()->GetPreviewScene()->GetPreviewMeshComponent();

		if (PreviewInstance->GetCurrentAsset() && PreviewSkelComp )
		{
			UAnimSequence* AnimSequence = Cast<UAnimSequence>( PreviewInstance->GetCurrentAsset() );
			if( AnimSequence )
			{
				const FScopedTransaction Transaction( LOCTEXT("ReZeroAnimation", "ReZero Animation Sequence") );

				//Call modify to restore anim sequence current state
			//	AnimSequence->Modify();

				// As above, animations don't have any idea of hierarchy, so we don't know for sure if track 0 is the root bone's track.
				FRawAnimSequenceTrack& RawTrack = AnimSequence->GetRawAnimationTrack(0);

				// Find vector that would translate current root bone location onto origin.
				FVector FrameTransform = FVector::ZeroVector;
				if (FrameIndex == INDEX_NONE)
				{
					// Use current transform
					FrameTransform = PreviewSkelComp->GetComponentSpaceTransforms()[0].GetLocation();
				}
				else if(RawTrack.PosKeys.IsValidIndex(FrameIndex))
				{
					// Use transform at frame
					FrameTransform = RawTrack.PosKeys[FrameIndex];
				}

				FVector ApplyTranslation = -1.f * FrameTransform;

				// Convert into world space
				FVector WorldApplyTranslation = PreviewSkelComp->GetComponentTransform().TransformVector(ApplyTranslation);
				ApplyTranslation = PreviewSkelComp->GetComponentTransform().InverseTransformVector(WorldApplyTranslation);

				for(int32 i=0; i<RawTrack.PosKeys.Num(); i++)
				{
					RawTrack.PosKeys[i] += ApplyTranslation;
				}

				// Handle Raw Data changing
				AnimSequence->MarkRawDataAsModified();
				AnimSequence->OnRawDataChanged();

				//AnimSequence->MarkPackageDirty();

				Model.Pin()->RefreshTracks();
			}
		}
	}
	*/
}

void SNeTimeline::OnShowPopupOfAppendAnimation(FWidgetPath WidgetPath, bool bBegin)
{
	TSharedRef<STextEntryPopup> TextEntry =
		SNew(STextEntryPopup)
		.Label(LOCTEXT("AppendAnim_AskNumFrames", "Number of Frames to Append"))
		.OnTextCommitted(this, &SNeTimeline::OnSequenceAppendedCalled, bBegin);

	// Show dialog to enter new track name
	FSlateApplication::Get().PushMenu(
		SharedThis(this),
		WidgetPath,
		TextEntry,
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup)
		);
}

void SNeTimeline::OnSequenceAppendedCalled(const FText & InNewGroupText, ETextCommit::Type CommitInfo, bool bBegin)
{
	// just a concern
	const static int32 MaxFrame = 1000;

	// handle only onEnter. This is a big thing to apply when implicit focus change or any other event
	if (CommitInfo == ETextCommit::OnEnter)
	{
		int32 NumFrames = FCString::Atoi(*InNewGroupText.ToString());
		if (NumFrames > 0 && NumFrames < MaxFrame)
		{
			OnAppendAnimSequence(bBegin, NumFrames);
			FSlateApplication::Get().DismissAllMenus();
		}
	}
}

TSharedRef<INumericTypeInterface<double>> SNeTimeline::GetNumericTypeInterface() const
{
	return NumericTypeInterface.ToSharedRef();
}

// FFrameRate::ComputeGridSpacing doesnt deal well with prime numbers, so we have a custom impl here
static bool ComputeGridSpacing(const FFrameRate& InFrameRate, float PixelsPerSecond, double& OutMajorInterval, int32& OutMinorDivisions, float MinTickPx, float DesiredMajorTickPx)
{
	// First try built-in spacing
	bool bResult = InFrameRate.ComputeGridSpacing(PixelsPerSecond, OutMajorInterval, OutMinorDivisions, MinTickPx, DesiredMajorTickPx);
	if(!bResult || OutMajorInterval == 1.0)
	{
		if (PixelsPerSecond <= 0.f)
		{
			return false;
		}

		const int32 RoundedFPS = FMath::RoundToInt(InFrameRate.AsDecimal());

		if (RoundedFPS > 0)
		{
			// Showing frames
			TArray<int32, TInlineAllocator<10>> CommonBases;

			// Divide the rounded frame rate by 2s, 3s or 5s recursively
			{
				const int32 Denominators[] = { 2, 3, 5 };

				int32 LowestBase = RoundedFPS;
				for (;;)
				{
					CommonBases.Add(LowestBase);
	
					if (LowestBase % 2 == 0)      { LowestBase = LowestBase / 2; }
					else if (LowestBase % 3 == 0) { LowestBase = LowestBase / 3; }
					else if (LowestBase % 5 == 0) { LowestBase = LowestBase / 5; }
					else
					{ 
						int32 LowestResult = LowestBase;
						for(int32 Denominator : Denominators)
						{
							int32 Result = LowestBase / Denominator;
							if(Result > 0 && Result < LowestResult)
							{
								LowestResult = Result;
							}
						}

						if(LowestResult < LowestBase)
						{
							LowestBase = LowestResult;
						}
						else
						{
							break;
						}
					}
				}
			}

			Algo::Reverse(CommonBases);

			const int32 Scale     = FMath::CeilToInt(DesiredMajorTickPx / PixelsPerSecond * InFrameRate.AsDecimal());
			const int32 BaseIndex = FMath::Min(Algo::LowerBound(CommonBases, Scale), CommonBases.Num()-1);
			const int32 Base      = CommonBases[BaseIndex];

			int32 MajorIntervalFrames = FMath::CeilToInt(Scale / float(Base)) * Base;
			OutMajorInterval  = MajorIntervalFrames * InFrameRate.AsInterval();

			// Find the lowest number of divisions we can show that's larger than the minimum tick size
			OutMinorDivisions = 0;
			for (int32 DivIndex = 0; DivIndex < BaseIndex; ++DivIndex)
			{
				if (Base % CommonBases[DivIndex] == 0)
				{
					int32 MinorDivisions = MajorIntervalFrames/CommonBases[DivIndex];
					if (OutMajorInterval / MinorDivisions * PixelsPerSecond >= MinTickPx)
					{
						OutMinorDivisions = MinorDivisions;
						break;
					}
				}
			}
		}
	}

	return OutMajorInterval != 0;
}

bool SNeTimeline::GetGridMetrics(float PhysicalWidth, double& OutMajorInterval, int32& OutMinorDivisions) const
{
	FSlateFontInfo SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 8);
	TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	FFrameRate DisplayRate(FMath::RoundToInt(TimelineMode.Pin()->GetFrameRate()), 1);
	double BiggestTime = ViewRange.Get().GetUpperBoundValue();
	FString TickString = NumericTypeInterface->ToString((BiggestTime * DisplayRate).FrameNumber.Value);
	FVector2D MaxTextSize = FontMeasureService->Measure(TickString, SmallLayoutFont);

	static float MajorTickMultiplier = 2.f;

	float MinTickPx = MaxTextSize.X + 5.f;
	float DesiredMajorTickPx = MaxTextSize.X * MajorTickMultiplier;

	if (PhysicalWidth > 0)
	{
		return ComputeGridSpacing(
			DisplayRate,
			PhysicalWidth / ViewRange.Get().Size<double>(),
			OutMajorInterval,
			OutMinorDivisions,
			MinTickPx,
			DesiredMajorTickPx);
	}

	return false;
}

TSharedPtr<ITimeSliderController> SNeTimeline::GetTimeSliderController() const
{ 
	return TimeSliderController; 
}

void SNeTimeline::OnOutlinerSearchChanged( const FText& Filter )
{
	FilterText = Filter;

	Outliner->RefreshFilter();
}

void SNeTimeline::OnColumnFillCoefficientChanged(float FillCoefficient, int32 ColumnIndex)
{
	ColumnFillCoefficients[ColumnIndex] = FillCoefficient;
}

void SNeTimeline::HandleKeyComplete()
{
	TimelineMode.Pin()->RefreshTracks();
}

class UAnimSingleNodeInstance* SNeTimeline::GetPreviewInstance() const
{
//	UDebugSkelMeshComponent* PreviewMeshComponent = EditorTimelineController.Pin()->GetPreviewScene()->GetPreviewMeshComponent();
	//return PreviewMeshComponent && PreviewMeshComponent->IsPreviewOn()? PreviewMeshComponent->PreviewInstance : nullptr;
	return nullptr;
}

void SNeTimeline::HandleScrubPositionChanged(FFrameTime NewScrubPosition, bool bIsScrubbing, bool bEvaluate)
{
	TimelineMode.Pin()->SetScrubPosition(NewScrubPosition);
}

double SNeTimeline::GetSpinboxDelta() const
{
	return FFrameRate(TimelineMode.Pin()->GetTickResolution(), 1).AsDecimal() * FFrameRate(FMath::RoundToInt(TimelineMode.Pin()->GetFrameRate()), 1).AsInterval();
}

void SNeTimeline::SetPlayTime(double InFrameTime)
{
	TimelineMode.Pin()->SetScrubPosition(FFrameTime::FromDecimal(InFrameTime ));
}

void SNeTimeline::SetDisplayFormat(EFrameNumberDisplayFormats InFormat)
{
	GetMutableDefault<UPersonaOptions>()->TimelineDisplayFormat = InFormat;
}

bool SNeTimeline::IsDisplayFormatChecked(EFrameNumberDisplayFormats InFormat) const
{
	return GetDefault<UPersonaOptions>()->TimelineDisplayFormat == InFormat;
}


void SNeTimeline::ToggleDisplayPercentage()
{
	GetMutableDefault<UPersonaOptions>()->bTimelineDisplayPercentage = !GetDefault<UPersonaOptions>()->bTimelineDisplayPercentage;
}

bool SNeTimeline::IsDisplayPercentageChecked() const
{
	return GetDefault<UPersonaOptions>()->bTimelineDisplayPercentage;
}

void SNeTimeline::ToggleDisplaySecondary()
{
	GetMutableDefault<UPersonaOptions>()->bTimelineDisplayFormatSecondary = !GetDefault<UPersonaOptions>()->bTimelineDisplayFormatSecondary;
}

bool SNeTimeline::IsDisplaySecondaryChecked() const
{
	return GetDefault<UPersonaOptions>()->bTimelineDisplayFormatSecondary;
}


void  SNeTimeline::ToggleTimelineEnabledSnaps()
{
	if (GetDefault<UPersonaOptions>()->TimelineEnabledSnaps.Contains(InSnapName))
	{
		GetMutableDefault<UPersonaOptions>()->TimelineEnabledSnaps.Remove(InSnapName);
	}
	else
	{
		GetMutableDefault<UPersonaOptions>()->TimelineEnabledSnaps.AddUnique(InSnapName);
	}
}

bool  SNeTimeline::IsDisplayTimelineEnabledSnapsChecked() const
{
	return IsEnabledSnaps();
}

bool SNeTimeline::IsEnabledSnaps()
{
	return !GetDefault<UPersonaOptions>()->TimelineEnabledSnaps.Contains(InSnapName);
}


#undef LOCTEXT_NAMESPACE
