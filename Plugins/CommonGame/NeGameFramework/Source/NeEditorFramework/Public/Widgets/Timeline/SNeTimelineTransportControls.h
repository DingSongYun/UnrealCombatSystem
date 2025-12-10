// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ITransportControl.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Framework/SlateDelegates.h"

class FNeTimelineMode;

class NEEDITORFRAMEWORK_API SNeTimelineTransportControls : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeTimelineTransportControls) {}
		SLATE_EVENT(FOnClicked, OnForwardPlay)
		SLATE_EVENT(FOnClicked, OnForwardStep)
		SLATE_EVENT(FOnClicked, OnToggleLooping)
		SLATE_EVENT(FOnGetLooping, OnGetLooping)
		SLATE_EVENT(FOnGetPlaybackMode, OnGetPlaybackMode)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<FNeTimelineMode>& InTimelineMode);
private:

	TWeakPtr<FNeTimelineMode> TimelineMode;
};