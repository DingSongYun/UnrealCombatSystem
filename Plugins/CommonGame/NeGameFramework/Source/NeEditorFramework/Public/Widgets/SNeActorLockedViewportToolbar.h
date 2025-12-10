// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SLevelViewport.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "SViewportToolBar.h"
#include "LevelViewportActions.h"
#include "Viewport/SNeSimpleEdViewport.h"

class NEEDITORFRAMEWORK_API SNeActorLockedViewportToolbar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS(SNeActorLockedViewportToolbar){}
		SLATE_ARGUMENT(TSharedPtr<SNeSimpleEdViewport>, Viewport)
	SLATE_END_ARGS()

	FText GetActiveText() const;

	EVisibility GetLockedTextVisibility() const;

	void Construct(const FArguments& InArgs);

private:

	/** The viewport that we are in */
	TWeakPtr<SNeSimpleEdViewport> Viewport;
};

