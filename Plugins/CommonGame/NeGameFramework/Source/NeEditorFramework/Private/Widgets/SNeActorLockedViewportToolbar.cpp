// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SNeActorLockedViewportToolbar.h"

#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "SActorLockedViewportToolbar"

FText SNeActorLockedViewportToolbar::GetActiveText() const
{
	const AActor* Pilot = nullptr;
	const auto ViewportPtr = Viewport.Pin();
	if (ViewportPtr.IsValid())
	{
		Pilot = ViewportPtr->GetSimpleViewportClient()->GetLockedActor();
	}
	
	return Pilot ? FText::Format(LOCTEXT("ActiveText", "[ Pilot Active - {0} ]"), FText::FromString(Pilot->GetActorLabel())) : FText();
}

EVisibility SNeActorLockedViewportToolbar::GetLockedTextVisibility() const
{
	const AActor* Pilot = nullptr;
	const auto ViewportPtr = Viewport.Pin();
	if (ViewportPtr.IsValid())
	{
		Pilot = ViewportPtr->GetSimpleViewportClient()->GetLockedActor();
	}

	return Pilot && Pilot->IsLockLocation() ? EVisibility::Visible : EVisibility::Collapsed;
}

void SNeActorLockedViewportToolbar::Construct(const FArguments& InArgs)
{
	SViewportToolBar::Construct(SViewportToolBar::FArguments());

	Viewport = InArgs._Viewport;

	const auto& ViewportCommands = FLevelViewportCommands::Get();

	FToolBarBuilder ToolbarBuilder(InArgs._Viewport->GetCommandList(), FMultiBoxCustomization::None);

	// Use a custom style
	const FName ToolBarStyle = "ViewportMenu";
	ToolbarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
	ToolbarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
	ToolbarBuilder.SetLabelVisibility(EVisibility::Collapsed);

	ToolbarBuilder.BeginSection("ActorPilot");
	ToolbarBuilder.BeginBlockGroup();
	{
		static FName EjectActorPilotName = FName(TEXT("EjectActorPilot"));
		ToolbarBuilder.AddToolBarButton(ViewportCommands.EjectActorPilot, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), EjectActorPilotName);

		//static FName ToggleActorPilotCameraViewName = FName(TEXT("ToggleActorPilotCameraView"));
		//ToolbarBuilder.AddToolBarButton(ViewportCommands.ToggleActorPilotCameraView, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), ToggleActorPilotCameraViewName);
	}
	ToolbarBuilder.EndBlockGroup();
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("ActorPilot_Label");
	ToolbarBuilder.AddWidget(
		SNew(SBox)
		// Nasty hack to make this VAlign_center properly. The parent Box is set to VAlign_Bottom, so we can't fill properly.
		.HeightOverride(24.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SNeActorLockedViewportToolbar::GetActiveText)
			]
			+ SHorizontalBox::Slot()
			.Padding(5.0f, 0.0f)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Red)
				.Text(LOCTEXT("ActorLockedText", "(Locked)"))
				.ToolTipText(LOCTEXT("ActorLockedToolTipText", "This actor has locked movement so it will not be updated based on camera position"))
				.Visibility(this, &SNeActorLockedViewportToolbar::GetLockedTextVisibility)
			]
		]
	);
	ToolbarBuilder.EndSection();

	ChildSlot
	[
		SNew( SBorder )
		.BorderImage( FAppStyle::GetBrush("NoBorder") )
		.Padding(FMargin(4.f, 0.f))
		// Color and opacity is changed based on whether or not the mouse cursor is hovering over the toolbar area
		//.ColorAndOpacity( this, &SViewportToolBar::OnGetColorAndOpacity )
		[
			ToolbarBuilder.MakeWidget()
		]
	];
}

#undef LOCTEXT_NAMESPACE
