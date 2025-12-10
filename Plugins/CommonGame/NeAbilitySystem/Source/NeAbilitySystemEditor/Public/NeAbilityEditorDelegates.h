// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "..\..\NeAbilitySystem\Public\NeAbilityWeakPtr.h"

struct FNeAbilitySegment;
class FMenuBuilder;

class NEABILITYSYSTEMEDITOR_API FNeAbilityEditorDelegates
{
public:
	DECLARE_MULTICAST_DELEGATE(FPlayForPreviewDelegate);
	static FPlayForPreviewDelegate PlayForPreviewDelegate;

	DECLARE_MULTICAST_DELEGATE_OneParam(FAddNewSegmentDelegate, const FNeAbilitySegmentPtr& /* New Segment*/);
	static FAddNewSegmentDelegate AddNewSegmentDelegate;

	DECLARE_MULTICAST_DELEGATE_OneParam(FDeleteSegmentDelegate, const FNeAbilitySegmentPtr& /* New Segment*/);
	static FDeleteSegmentDelegate PreDeleteSegmentDelegate;

	DECLARE_MULTICAST_DELEGATE_OneParam(FMoveSegmentDelegate, const FNeAbilitySegmentPtr& /* New Segment*/);
	static FMoveSegmentDelegate MoveSegmentDelegate;

	DECLARE_MULTICAST_DELEGATE_OneParam(FSegmentSelectionChangedDelegate, const FNeAbilitySegmentPtr& /* New Segment*/);
	static FSegmentSelectionChangedDelegate SegmentSelectionChangedDelegate;

	DECLARE_DELEGATE_RetVal_OneParam(class ACameraActor*, FOnCreatePreviewCameraDelegate, class UWorld* /* World */);
	static FOnCreatePreviewCameraDelegate OnCreatePreviewCamera;

	/** Segment 右键菜单，供外部扩展 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FBuildSegmentContextMenuDelegate, FMenuBuilder& /*MenuBuilder*/, const FNeAbilitySegment& /*Segment*/);
	static FBuildSegmentContextMenuDelegate BuildSegmentContextMenuDelegate;

	/** Detail 面板变化 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FDetailPropertyChangedDelegate, UObject*, const FPropertyChangedEvent&);
	static FDetailPropertyChangedDelegate OnDetailPropertyChanged;
};