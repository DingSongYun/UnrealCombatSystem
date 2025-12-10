// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/Timeline/SNeTimelineTransportControls.h"
#include "EditorWidgetsModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Timeline/NeTimelineMode.h"

void SNeTimelineTransportControls::Construct(const FArguments& InArgs, const TSharedRef<FNeTimelineMode>& InTimelineMode)
{
	TimelineMode = InTimelineMode;

	FEditorWidgetsModule& EditorWidgetsModule = FModuleManager::LoadModuleChecked<FEditorWidgetsModule>("EditorWidgets");

	FTransportControlArgs TransportControlArgs;

	if (!InTimelineMode->IsDebuggerMode())
	{
		TransportControlArgs.OnForwardPlay = InArgs._OnForwardPlay;
		TransportControlArgs.OnForwardStep = InArgs._OnForwardStep;
		TransportControlArgs.OnToggleLooping = InArgs._OnToggleLooping;
		TransportControlArgs.OnGetLooping = InArgs._OnGetLooping;
		TransportControlArgs.OnGetPlaybackMode = InArgs._OnGetPlaybackMode;
	}

	ChildSlot
	[
		EditorWidgetsModule.CreateTransportControl(TransportControlArgs)
	];
}