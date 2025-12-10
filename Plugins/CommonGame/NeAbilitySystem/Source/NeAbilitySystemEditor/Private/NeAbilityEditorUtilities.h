// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClassViewerModule.h"
#include "Framework/SlateDelegates.h"

class UNeAbilitySegment;

/**
 * FNeAbilityEditorUtilities
 * 本文件存放一些在Editor公用的方法
 */
class FNeAbilityEditorUtilities
{
public:
	DECLARE_DELEGATE_OneParam( FOnTaskSegment, UNeAbilitySegment* );

public:
	/** Create SegmentPicker for specific menu*/
	static void MakeNewSegmentPicker(const TSharedRef<class FNeAbilityBlueprintEditor>& AbilityEidtorPtr, class FMenuBuilder& MenuBuilder, const FOnClassPicked& OnTypePicked, bool bUseClassViewer = false);

	static TSharedRef<SWidget> MakeTrackButton(FText HoverText, FOnGetContent MenuContent, const TAttribute<bool>& HoverState);

	/** 从文本导入Segment */
	static void ImportTasksFromText(class UNeAbility* Asset, const FString& ImportText, TArray<UNeAbilitySegment*>& OutTasks);
};