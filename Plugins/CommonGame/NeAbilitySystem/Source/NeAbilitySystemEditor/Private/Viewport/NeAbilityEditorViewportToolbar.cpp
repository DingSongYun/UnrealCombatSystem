// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityEditorViewportToolbar.h"
#include "SEditorViewportViewMenu.h"
#include "SEditorViewportToolBarMenu.h"
#include "STransformViewportToolbar.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/STextComboBox.h"
#include "SceneOutlinerPublicTypes.h"
#include "SceneOutlinerModule.h"
#include "EditorViewportCommands.h"
#include "BoneSelectionWidget.h"
#include "LevelViewportActions.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityEditorViewportClient.h"
#include "Widgets/SBoxPanel.h"
#include "NeAbilityEditorViewportPlaybackCommands.h"
#include "ShowFlagMenuCommands.h"
#include "Viewport/NeSimpleEdViewportClient.h"
#include "Widgets/SEditorViewportCameraMenu.h"
#include "Widgets/SNeAbilityEditorViewport.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

#define LOCTEXT_NAMESPACE "AbilityEditorViewportToolbar"

const FName DefaultSkillBone = FName(TEXT("DEFAULT_Bone"));
namespace ENeAbilityPlaybackSpeeds
{
	// Speed scales for animation playback, must match EAnimationPlaybackSpeeds::Type
	float Values[ENeAbilityPlaybackSpeeds::NumPlaybackSpeeds] = { 0.1f, 0.25f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f };
}

void SNeAbilityEditorViewportToolBar::Construct(const FArguments& InArgs, TSharedPtr<FNeAbilityBlueprintEditor> InHostEditor, TSharedPtr<SNeAbilityEditorViewport> InViewport)
{
	Viewport = InViewport;
	TSharedPtr<FNeSimpleEdViewportClient> SimpeEdViewportClient = StaticCastSharedPtr<FNeSimpleEdViewportClient>(InViewport->GetViewportClient());
	HostEditor = InHostEditor;

	CommandList = InViewport->GetCommandList();
	// Extenders = InArgs._Extenders;
	// Extenders.Add(GetViewMenuExtender(InViewport));

	// If we have no extender, make an empty one
	if (Extenders.Num() == 0)
	{
		Extenders.Add(MakeShared<FExtender>());
	}
	const FMargin ToolbarSlotPadding(4.0f, 1.0f);

	TSharedRef<SHorizontalBox> ToolbarContainer = SNew(SHorizontalBox)
		// Options
		//+ SHorizontalBox::Slot()
		//.AutoWidth()
		//.Padding(2.0f, 2.0f)
		//[
		//	SNew(SEditorViewportOptionsMenu, Viewport.ToSharedRef(), SharedThis(this))
		//	.OnGetFOVValueHandler_Lambda([&] { return this->Viewport.Pin()->GetViewportClient()->ViewFOV; })
		//	.OnFOVValueChangedHandler_Lambda([&](float NewFOV) { this->Viewport.Pin()->GetViewportClient()->ViewFOV = NewFOV; })
		//	.MenuExtenders( GetOptionMenuExtender() )
		//]

		// Generic viewport options
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 2.0f)
		[
			//Menu
			SNew(SEditorViewportToolbarMenu)
			.ParentToolBar(SharedThis(this))
			.Image("EditorViewportToolBar.MenuDropdown")
			.OnGetMenuContent(this, &SNeAbilityEditorViewportToolBar::GenerateViewMenu)
		]

		// Camera Type (Perspective/Top/etc...)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 2.0f)
		[
			SNew(SEditorViewportCameraMenu, Viewport.Pin().ToSharedRef(), SharedThis(this))
			.MenuLabel(this, &SNeAbilityEditorViewportToolBar::GetCameraMenuLabel)
			.MenuIcon(this, &SNeAbilityEditorViewportToolBar::GetCameraMenuLabelIcon)
			.OnActorToggleHandler_Raw(SimpeEdViewportClient.Get(), &FNeSimpleEdViewportClient::OnActorLockToggleFromMenu)
			.IsActorLockerHandler_Raw(SimpeEdViewportClient.Get(), &FNeSimpleEdViewportClient::IsActorLocked)
			.World(HostEditor.Pin()->GetAbilityPreviewScene()->GetWorld())
		]

		// View menu (lit, unlit, etc...)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 2.0f)
		[
			SNew(SEditorViewportViewMenu, Viewport.Pin().ToSharedRef(), SharedThis(this))
		]
	
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding( ToolbarSlotPadding )
		[
			SNew( SEditorViewportToolbarMenu )
			.ParentToolBar( SharedThis( this ) )
			.Cursor(EMouseCursor::Default)
			.Label(LOCTEXT("ShowMenu", "Show"))
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ViewMenuButton")))
			.OnGetMenuContent( this, &SNeAbilityEditorViewportToolBar::GenerateShowMenu ) 
		]
	
		+ SHorizontalBox::Slot()
		.Padding(3.0f, 1.0f)
		.HAlign(HAlign_Right)
		[
			SNew(STransformViewportToolBar)
			.Viewport(Viewport.Pin())
			.CommandList(Viewport.Pin()->GetCommandList())
			.Visibility(this, &SNeAbilityEditorViewportToolBar::GetTransformToolBarVisibility)
		]
		
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 1.0f)
			[
				SNew(SEditorViewportToolbarMenu)
				.ToolTipText(LOCTEXT("PlaybackSpeedMenuTooltip", "Playback Speed Options. Control the time dilation of the scene's update.\nShift-clicking items will 'pin' them to the toolbar."))
			.ParentToolBar(SharedThis(this))
			.Label(this, &SNeAbilityEditorViewportToolBar::GetPlaybackMenuLabel)
			.LabelIcon(FAppStyle::GetBrush("AnimViewportMenu.PlayBackSpeed"))
			.OnGetMenuContent(this, &SNeAbilityEditorViewportToolBar::GeneratePlaybackMenu)
			];

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		// Color and opacity is changed based on whether or not the mouse cursor is hovering over the toolbar area
		//.ColorAndOpacity(this, &SViewportToolBar::OnGetColorAndOpacity)
		.ForegroundColor(FAppStyle::GetSlateColor("DefaultForeground"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[

				ToolbarContainer
			]
		]
	];
		
	TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = StaticCastSharedPtr<FNeAbilityEditorViewportClient>(Viewport.Pin()->GetViewportClient());
	ViewportClient->EngineShowFlags.SetSingleFlag(FEngineShowFlags::SF_EyeAdaptation, true);

	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}


TSharedRef<SWidget> SNeAbilityEditorViewportToolBar::GenerateShowMenu() const
{
	static const FName MenuName("AbilityEditor.AbilityEViewportToolbar.Show");


	
	if (!UToolMenus::Get()->IsMenuRegistered(MenuName))
	{
		UToolMenu* Menu = UToolMenus::Get()->RegisterMenu(MenuName);

		const FLevelViewportCommands& Actions = FLevelViewportCommands::Get();
		{
			FToolMenuSection& Section = Menu->AddSection("UseDefaultShowFlags");
			Section.AddMenuEntry(Actions.UseDefaultShowFlags);
		}

		Menu->AddDynamicSection("AbilityDynamicSection", FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
		{
			// Only include helpful show flags.
			static const FShowFlagFilter ShowFlagFilter = FShowFlagFilter(FShowFlagFilter::ExcludeAllFlagsByDefault)
				// General
				.IncludeFlag(FEngineShowFlags::SF_AntiAliasing)
				.IncludeFlag(FEngineShowFlags::SF_Collision)
				.IncludeFlag(FEngineShowFlags::SF_Grid)
				.IncludeFlag(FEngineShowFlags::SF_Particles)
				.IncludeFlag(FEngineShowFlags::SF_Translucency)
				// Post Processing
				.IncludeFlag(FEngineShowFlags::SF_Bloom)
				.IncludeFlag(FEngineShowFlags::SF_DepthOfField)
				.IncludeFlag(FEngineShowFlags::SF_EyeAdaptation)
				.IncludeFlag(FEngineShowFlags::SF_HMDDistortion)
				.IncludeFlag(FEngineShowFlags::SF_MotionBlur)
				.IncludeFlag(FEngineShowFlags::SF_Tonemapper)
				// Lighting Components
				.IncludeGroup(SFG_LightingComponents)
				// Lighting Features
				.IncludeFlag(FEngineShowFlags::SF_AmbientCubemap)
				.IncludeFlag(FEngineShowFlags::SF_DistanceFieldAO)
				.IncludeFlag(FEngineShowFlags::SF_IndirectLightingCache)
				.IncludeFlag(FEngineShowFlags::SF_LightFunctions)
				.IncludeFlag(FEngineShowFlags::SF_LightShafts)
				.IncludeFlag(FEngineShowFlags::SF_ReflectionEnvironment)
				.IncludeFlag(FEngineShowFlags::SF_ScreenSpaceAO)
				.IncludeFlag(FEngineShowFlags::SF_ContactShadows)
				.IncludeFlag(FEngineShowFlags::SF_ScreenSpaceReflections)
				.IncludeFlag(FEngineShowFlags::SF_SubsurfaceScattering)
				.IncludeFlag(FEngineShowFlags::SF_TexturedLightProfiles)
				// Developer
				.IncludeFlag(FEngineShowFlags::SF_Refraction)
				// Advanced
				.IncludeFlag(FEngineShowFlags::SF_DeferredLighting)
				.IncludeFlag(FEngineShowFlags::SF_Selection)
				.IncludeFlag(FEngineShowFlags::SF_SeparateTranslucency)
				.IncludeFlag(FEngineShowFlags::SF_TemporalAA)
				.IncludeFlag(FEngineShowFlags::SF_VertexColors)
				.IncludeFlag(FEngineShowFlags::SF_MeshEdges)
				;
			static const FShowFlagFilter ShowAllFlagFilter = FShowFlagFilter(FShowFlagFilter::IncludeAllFlagsByDefault);
			FShowFlagMenuCommands::Get().BuildShowFlagsMenu(InMenu, ShowAllFlagFilter);

			// FEngineShowFlags::SetSingleFlag(FEngineShowFlags::SF_EyeAdaptation, true);
		}));
	}
	
	TSharedPtr<FExtender> MenuExtender = FExtender::Combine(Extenders);
	FToolMenuContext MenuContext(CommandList);

	return UToolMenus::Get()->GenerateWidget(MenuName, MenuContext);
}

TSharedPtr<FExtender> SNeAbilityEditorViewportToolBar::GetOptionMenuExtender()
{
	if (OptionsMenuExtender == nullptr)
	{
		OptionsMenuExtender = MakeShared<FExtender>();
	}
	OptionsMenuExtender->AddMenuExtension(FName("Ability"), EExtensionHook::After, nullptr, 
		FMenuExtensionDelegate::CreateSP(this, &SNeAbilityEditorViewportToolBar::GenerateOptionMenu));
	return OptionsMenuExtender;
}

void SNeAbilityEditorViewportToolBar::GenerateOptionMenu(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddWidget(GenerateViewMenu(), FText::FromString(""));
}

TSharedRef<SWidget> SNeAbilityEditorViewportToolBar::GenerateViewMenu()
{
	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ViewMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList());

	// 相机模式
	ViewMenuBuilder.BeginSection("ViewportCamera", LOCTEXT("ViewMenu_CameraLabel", "Camera"));
	{
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().FocusViewportToSelection);

		ViewMenuBuilder.AddSubMenu(
			LOCTEXT("CameraFollowModeLabel", "Camera Follow Mode"),
			LOCTEXT("CameraFollowModeTooltip", "Set various camera follow modes"),
			FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InSubMenuBuilder)
			{
				InSubMenuBuilder.BeginSection("SkillViewportCameraFollowMode", LOCTEXT("ViewMenu_CameraFollowModeLabel", "Camera Follow Mode"));
				{
					InSubMenuBuilder.PushCommandList(Viewport.Pin()->GetCommandList().ToSharedRef());

					InSubMenuBuilder.AddMenuEntry(
						GetCameraModeEntryLable(NeAbilityEditorViewportCameraMode::Free),
						//FText::FromString("Free Camera"), 
						FText::FromString(""), 
						FSlateIcon(), 
						FUIAction(FExecuteAction::CreateRaw(this, &SNeAbilityEditorViewportToolBar::SetCameraMode, NeAbilityEditorViewportCameraMode::Free))
					);
					InSubMenuBuilder.AddMenuEntry(
						GetCameraModeEntryLable(NeAbilityEditorViewportCameraMode::RealGame),
						//FText::FromString("In Game Camera"), 
						FText::FromString(""), 
						FSlateIcon(), 
						FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityEditorViewportToolBar::SetCameraMode, NeAbilityEditorViewportCameraMode::RealGame))
					);
					InSubMenuBuilder.AddSubMenu(
						TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateRaw(
							this, &SNeAbilityEditorViewportToolBar::GetCameraModeEntryLable, NeAbilityEditorViewportCameraMode::CameraAttach, false)
						),
						FText::FromString(""), 
						FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InSubMenuBuilder)
						{
#if false // TODO
							// Set up a menu entry to add any arbitrary actor to the sequencer
							FSceneOutlinerInitializationOptions InitOptions;
							{
								InitOptions.Mode = ESceneOutlinerMode::ActorPicker;

								InitOptions.bShowHeaderRow = false;
								InitOptions.bShowSearchBox = true;
								InitOptions.bShowCreateNewFolder = false;
								InitOptions.bFocusSearchBoxWhenOpened = true;

								InitOptions.ColumnMap.Add(FBuiltInColumnTypes::Label(), FColumnInfo(EColumnVisibility::Visible, 0));
								InitOptions.Filters->AddFilterPredicate(SceneOutliner::FActorFilterPredicate::CreateLambda([] (const AActor* const ParentActor)
								{
									return ParentActor->IsA<ACameraActor>();
								}));

								if (HostEditor.IsValid())
								{
									InitOptions.SpecifiedWorldToDisplay = HostEditor.Pin()->GetAbilityPreviewScene()->GetWorld();
								}
							}

							FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");
							TSharedRef< SWidget > MiniSceneOutliner =
							SNew(SBox)
							.MaxDesiredHeight(400.0f)
							.WidthOverride(300.0f)
							[
								SceneOutlinerModule.CreateSceneOutliner(
									InitOptions,
									FOnActorPicked::CreateLambda([this](AActor* Actor){
										FSlateApplication::Get().DismissAllMenus();
										this->SetCameraMode(NeAbilityEditorViewportCameraMode::CameraAttach);
										TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = StaticCastSharedPtr<FNeAbilityEditorViewportClient>(Viewport.Pin()->GetViewportClient());
										ViewportClient->SetViewLocation(Actor->GetActorLocation());
										ViewportClient->SetViewRotation(Actor->GetActorRotation());
										ViewportClient->SetLockedActor(Actor);
										ACameraActor* CameraActor = Cast<ACameraActor>(Actor);
										UCameraComponent* CameraComponent = CameraActor ? CameraActor->GetCameraComponent() : nullptr;
										if (CameraComponent)
										{
											ViewportClient->AspectRatio = CameraComponent->AspectRatio == 0 ? 1.7f : CameraComponent->AspectRatio;
											ViewportClient->ViewFOV = CameraComponent->FieldOfView;
										}
										ViewportClient->Viewport->InvalidateHitProxy();
									})
								)
							];
							InSubMenuBuilder.AddWidget(MiniSceneOutliner, FText::GetEmpty(), true);
#endif
						}),
						false,
						FSlateIcon()
					);
#if false /** Disable CameraAnimLock */
					InSubMenuBuilder.AddMenuEntry(
						TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateRaw(
							this, &SNeAbilityEditorViewportToolBar::GetCameraModeEntryLable, EAblViewportCameraMode::CameraAnimLock, false)
						),
						//FText::FromString("Lock to Camera animation"), 
						FText::FromString(""), 
						FSlateIcon(), 
						FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityEditorViewportToolBar::SetCameraMode, EAblViewportCameraMode::CameraAnimLock))
					);
#endif
#if false /** Disable Camera follow bone */
					InSubMenuBuilder.AddMenuEntry(
						TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateRaw(
							this, &SNeAbilityEditorViewportToolBar::GetCameraModeEntryLable, EAblViewportCameraMode::Bone, true)
						),
						//FText::FromString("Camera Follow Skill"),
						FText::FromString(""),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateLambda([this]() {
							this->SetCameraMode(EAblViewportCameraMode::Bone);
							this->SetCameraLockedBone(DefaultSkillBone);
						}))
					);
#endif

					InSubMenuBuilder.PopCommandList();
				}
				InSubMenuBuilder.EndSection();
		
#if false
				InSubMenuBuilder.BeginSection("ViewportCameraFollowBone", FText());
				{
					InSubMenuBuilder.AddWidget(MakeFollowBoneWidget(), FText(), true);
				}
#endif
				InSubMenuBuilder.EndSection();
			}),
			false,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "AnimViewportMenu.CameraFollow")
			);

		ViewMenuBuilder.AddWidget(MakeFOVWidget(), LOCTEXT("Viewport_FOVLabel", "Field Of View"));

		ViewMenuBuilder.AddMenuEntry(
			FText::FromString("Reset VierwPort"), 
			FText::FromString(""),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &SNeAbilityEditorViewportToolBar::ResetCamaraTransform, GetCameraMode())));
	}
	ViewMenuBuilder.EndSection();

	return ViewMenuBuilder.MakeWidget();
}


EVisibility SNeAbilityEditorViewportToolBar::GetTransformToolBarVisibility() const
{
	return EVisibility::Visible;
}

void SNeAbilityEditorViewportToolBar::SetCameraMode(FName InCameraMode)
{
	TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = StaticCastSharedPtr<FNeAbilityEditorViewportClient>(Viewport.Pin()->GetViewportClient());
	if (ViewportClient.IsValid())
	{
		ViewportClient->SetAblViewMode(InCameraMode);
	}
}

FName SNeAbilityEditorViewportToolBar::GetCameraMode() const
{
	TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = StaticCastSharedPtr<FNeAbilityEditorViewportClient>(Viewport.Pin()->GetViewportClient());
	return ViewportClient->GetAblViewMode();
}

#if false
void SNeAbilityEditorViewportToolBar::SetCameraLockedBone(const FName& BoneName)
{
	TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = StaticCastSharedPtr<FNeAbilityEditorViewportClient>(Viewport.Pin()->GetViewportClient());
	if (ViewportClient.IsValid())
	{
		TWeakPtr<FAblAbilityEditor> AblEditor = Viewport.Pin()->m_AbilityEditor;

		if (AblEditor.IsValid())
		{
			//m_AbilityEditor
			ABaseCharacter* PreviewCharacter = Cast<ABaseCharacter>(AblEditor.Pin()->GetPreviewActor());
			if (PreviewCharacter)
			{
				ViewportClient->SetViewLockedBone(PreviewCharacter->GetMesh(), BoneName);
			}
		}
	}
}
#endif

void SNeAbilityEditorViewportToolBar::ResetCamaraTransform(FName CameraMode)
{
	if (CameraMode == NeAbilityEditorViewportCameraMode::Free)
	{
		Viewport.Pin()->GetViewportClient()->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation);
		Viewport.Pin()->GetViewportClient()->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation);
	}
}


#if false
TSharedRef<SWidget> SNeAbilityEditorViewportToolBar::MakeFollowBoneWidget()
{
	return MakeFollowBoneWidget(nullptr);
}

TSharedRef<SWidget> SNeAbilityEditorViewportToolBar::MakeFollowBoneWidget(TWeakPtr<class SComboButton> InWeakComboButton)
{
	TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = StaticCastSharedPtr<FNeAbilityEditorViewportClient>(Viewport.Pin()->GetViewportClient());

	TSharedPtr<SBoneTreeMenu> BoneTreeMenu;

	TSharedRef<SWidget> MenuWidget =
		SNew(SBox)
		.MaxDesiredHeight(400.0f)
		[
			SAssignNew(BoneTreeMenu, SBoneTreeMenu)
			.Title(LOCTEXT("FollowBoneTitle", "Camera Follow Bone"))
			//.Title(this, &SNeAbilityEditorViewportToolBar::GetCameraModeEntryLable, EAblViewportCameraMode::Bone)
			.bShowVirtualBones(true)
			.OnBoneSelectionChanged_Lambda([this](FName InBoneName)
			{
				SetCameraMode(EAblViewportCameraMode::Bone);
				SetCameraLockedBone(InBoneName);
				FSlateApplication::Get().DismissAllMenus();

			})
			.SelectedBone(ViewportClient->GetCameraLockedBone())
			.OnGetReferenceSkeleton_Lambda([this]() -> const FReferenceSkeleton&
			{
				TWeakPtr<FAblAbilityEditor> AblEditor = Viewport.Pin()->m_AbilityEditor;

				if (AblEditor.IsValid())
				{
					//m_AbilityEditor
					ABaseCharacter* PreviewCharacter = Cast<ABaseCharacter>(AblEditor.Pin()->GetPreviewActor());
					if (PreviewCharacter)
					{
						return PreviewCharacter->GetMesh()->SkeletalMesh->GetRefSkeleton();
					}
				}

				static FReferenceSkeleton EmptySkeleton;
				return EmptySkeleton;
			})
		];

	if(InWeakComboButton.IsValid())
	{
		InWeakComboButton.Pin()->SetMenuContentWidgetToFocus(BoneTreeMenu->GetFilterTextWidget());
	}

	return MenuWidget;
}
#endif

TSharedRef<SWidget> SNeAbilityEditorViewportToolBar::MakeFOVWidget() const
{
	const float FOVMin = 5.f;
	const float FOVMax = 170.f;

	return
		SNew(SBox)
		.HAlign(HAlign_Right)
		[
			SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			.WidthOverride(100.0f)
			[
				SNew(SNumericEntryBox<float>)
				.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.AllowSpin(true)
				.MinValue(FOVMin)
				.MaxValue(FOVMax)
				.MinSliderValue(FOVMin)
				.MaxSliderValue(FOVMax)
				.Value_Lambda([this]() -> float {
					return Viewport.Pin()->GetViewportClient()->ViewFOV;
				})
				.OnValueChanged_Lambda([this](float NewValue) {
					Viewport.Pin()->GetViewportClient()->FOVAngle = NewValue;
					Viewport.Pin()->GetViewportClient()->ViewFOV = NewValue;
					Viewport.Pin()->GetViewportClient()->Invalidate();
				})
			]
		];
}

FText SNeAbilityEditorViewportToolBar::GetCameraModeEntryLable(const FName InMode, bool bSkillBone) const
{
	bool bCurrMode = GetCameraMode() == InMode;
	if (InMode == NeAbilityEditorViewportCameraMode::Free)
	{
		return FText::FromString(bCurrMode ? "* Free Camera" : "Free Camera");
	}
	else if (InMode == NeAbilityEditorViewportCameraMode::RealGame)
	{
		return FText::FromString(bCurrMode ? "* In Game Camera" : "In Game Camera");
	}
	else if (InMode == NeAbilityEditorViewportCameraMode::CameraAttach)
	{
		return FText::FromString(bCurrMode ? "* Attach To Camera " : "Attach To Camera");
	}
#if false
	else if (InMode == NeAbilityEditorViewportCameraMode::Bone)
	{
		if (bSkillBone)
		{
			TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = StaticCastSharedPtr<FNeAbilityEditorViewportClient>(Viewport.Pin()->GetViewportClient());
			bCurrMode = bCurrMode && ViewportClient->GetCameraLockedBone() == DefaultSkillBone;
			return FText::FromString(bCurrMode ? "* Camera Follow Skill" : "Camera Follow Skill");
		}
		return FText::FromString(bCurrMode ? "* Camera Follow Bone" : "Camera Follow Bone");
	}
	else if (InMode == NeAbilityEditorViewportCameraMode::CameraAnimLock)
	{
		return FText::FromString(bCurrMode ? "* Lock to Camera animation" : "Lock to Camera animation");
	}
#endif

	return FText::FromString("");
}

FText SNeAbilityEditorViewportToolBar::GetPlaybackMenuLabel() const
{
	FText Label = LOCTEXT("PlaybackError", "Error");
	if (Viewport.IsValid())
	{
		for (int i = 0; i < ENeAbilityPlaybackSpeeds::NumPlaybackSpeeds; ++i)
		{
			if (Viewport.Pin()->IsPlaybackSpeedSelected(i))
			{
				int32 NumFractionalDigits = (i == ENeAbilityPlaybackSpeeds::Quarter) ? 2 : 1;

				const FNumberFormattingOptions FormatOptions = FNumberFormattingOptions()
					.SetMinimumFractionalDigits(NumFractionalDigits)
					.SetMaximumFractionalDigits(NumFractionalDigits);

				Label = FText::Format(LOCTEXT("AbilityViewportPlaybackMenuLabel", "x{0}"), FText::AsNumber(ENeAbilityPlaybackSpeeds::Values[i], &FormatOptions));
			}
		}
	}
	return Label;
}

TSharedRef<SWidget> SNeAbilityEditorViewportToolBar::GeneratePlaybackMenu() 
{
	const FNeAbilityEditorViewportPlaybackCommands& Actions = FNeAbilityEditorViewportPlaybackCommands::Get();

	//if (OptionsMenuExtender == nullptr)
	//{
	//	OptionsMenuExtender = MakeShared<FExtender>();
	//}
	TSharedPtr<FExtender> MenuExtender = GetOptionMenuExtender();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder InMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList(), MenuExtender);

	InMenuBuilder.PushCommandList(Viewport.Pin()->GetCommandList().ToSharedRef());
	InMenuBuilder.PushExtender(MenuExtender.ToSharedRef());
	{
		// View modes
		{
			InMenuBuilder.BeginSection("AbilityViewportPlaybackSpeed", LOCTEXT("PlaybackMenu_SpeedLabel", "Playback Speed"));
			{
				for (int32 PlaybackSpeedIndex = 0; PlaybackSpeedIndex < ENeAbilityPlaybackSpeeds::NumPlaybackSpeeds; ++PlaybackSpeedIndex)
				{
					//两种流程的尝试
					/*InMenuBuilder.AddMenuEntry(LOCTEXT("", ""), LOCTEXT("", ""), FSlateIcon(),
						FUIAction(FExecuteAction::CreateSP(this, &SNeAbilityEditorViewportToolBar::SetPlaybackSpeed, (ENeAbilityPlaybackSpeeds::Type)PlaybackSpeedIndex)));*/
					InMenuBuilder.AddMenuEntry(Actions.PlaybackSpeedCommands[PlaybackSpeedIndex]);
				}
			}
			InMenuBuilder.EndSection();
		}
	}
	InMenuBuilder.PopCommandList();
	InMenuBuilder.PopExtender();

	return InMenuBuilder.MakeWidget();
}

FText SNeAbilityEditorViewportToolBar::GetCameraMenuLabel() const
{
	return GetCameraMenuLabelFromViewportType(Viewport.Pin()->GetViewportClient()->ViewportType);
}

const FSlateBrush* SNeAbilityEditorViewportToolBar::GetCameraMenuLabelIcon() const
{
	return GetCameraMenuLabelIconFromViewportType(Viewport.Pin()->GetViewportClient()->ViewportType);
}

#undef LOCTEXT_NAMESPACE
