// Copyright NetEase Games, Inc. All Rights Reserved.
// 之后添加一个针对AbilityBlueprint的CompileContext
// 暂时加一个用于构建BeamLinkage的方法类

#pragma once
#include "NeAbilityWeakPtr.h"

class UNeAbilityBeam_GameplayCue;
class UNeAbilityBeam_GameplayEffect;
class UNeAbilityBeam_GameplayTask;
class UNeAbilityBeamLinkage;

class FBeamLinkageCompilerContext
{
public:
	static void PostConstructBeamLinkage(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeamLinkage* BeamLinkage);

private:
	static void PostLinkGameplayTask(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeam_GameplayTask* GameplayTaskBeam);
	static void PostLinkGameplayEffect(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeam_GameplayEffect* GameplayTaskBeam);
	static void PostLinkGameplayCue(const FNeAbilitySegmentPtr& SegmentPtr, UNeAbilityBeam_GameplayCue* GameplayTaskBeam);
};
