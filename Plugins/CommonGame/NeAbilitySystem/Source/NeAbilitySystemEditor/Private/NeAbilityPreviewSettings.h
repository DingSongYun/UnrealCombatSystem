// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/FrameRate.h"
#include "NeAbilityPreviewSettings.generated.h"

UENUM()
enum EAblAbilityEditorTimeStep
{
	FPS_15 UMETA(DisplayName = "15 FPS"),
	FPS_30 UMETA(DisplayName = "30 FPS"),
	FPS_60 UMETA(DisplayName = "60 FPS"),
};

/**
 * UNeAbilityPreviewSettings
 * 预览参数
 */
UCLASS(config = Editor, defaultconfig)
class UNeAbilityPreviewSettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Config, EditAnywhere, Category = Viewport, meta = (DisplayName = "Field of View", ClampMin = 70.0f, ClampMax = 180.0f))
	float FOV;

	/* Whether to mute the in-game audio of the viewport or not.*/
	UPROPERTY(Config, EditAnywhere, Category = Viewport, meta = (DisplayName = "Mute Audio"))
	bool bMuteAudio;

	/* At what rate to step an Ability when paused and stepping frame by frame.*/
	UPROPERTY(Config, EditAnywhere, Category = Viewport, meta = (DisplayName = "Ability Step Rate"))
	TEnumAsByte<EAblAbilityEditorTimeStep> TimeStepRate = EAblAbilityEditorTimeStep::FPS_30;

	/* If true, Task nodes will use a more dynamic, descriptive title than just the Task name.*/
	UPROPERTY(Config, EditAnywhere, Category = Timeline, meta = (DisplayName = "Descriptive Task Titles"))
	bool ShowDescriptiveTaskTitles;

	/* The unit, in seconds, to use when snapping Tasks. Smallest valid unit is 1 millisecond. */
	UPROPERTY(Config, EditAnywhere, Category = Timeline, meta = (DisplayName = "Snap Unit"))
	float SnapUnit = 0.0f;

	/** 预览播放到结尾时自动重置预览环境 */
	UPROPERTY(Config, EditAnywhere, Category = Preview)
	bool bAutoResetWhenPlayToEnd = true;

	UPROPERTY(Config, EditAnywhere, Category = Preview, meta=(EditCondition="bAutoResetWhenPlayToEnd"))
	float AutoResetDelayTime = 0.5;

	FFrameRate GetFrameRate() const
	{
		switch (TimeStepRate)
		{
		case EAblAbilityEditorTimeStep::FPS_15:
			return FFrameRate(15, 1);
		case EAblAbilityEditorTimeStep::FPS_30:
			return FFrameRate(30, 1);
		case EAblAbilityEditorTimeStep::FPS_60:
			return FFrameRate(60, 1);
		default:
			break;
		}

		return FFrameRate(30, 1);
	}

public:
	UPROPERTY(Config, EditAnywhere, Category = "Common Class")
	TSubclassOf<class AActor> HelperClass;

public:
	UPROPERTY(Config, EditAnywhere, Category = "Collision")
	TArray<TSubclassOf<AActor>> ActorType = {AActor::StaticClass()};

	UPROPERTY(Config, EditAnywhere, Category = "Collision")
	float ShowCollisionBoxDuration = 2.0f;
};