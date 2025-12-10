// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityTimeController.generated.h"

class UNeAbility;

/**
 * FNeAbilityTimeController
 * 技能时间轴控制
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityTimeController
{
	GENERATED_BODY()

public:
	void InitializeFor(UNeAbility* InAbility);
	/** 开始播放*/
	void StartPlaying(int32 FromSection);

	/** 结束播放 */
	void StopPlaying();

	/** update */
	void Update(float DeltaTime);

	/** 是否在播放 */
	bool IsPlaying() const { return PlayedTime >= 0; }

	/** 是否播放到技能最后 */
	bool HasReachedEnd() const { return PlayingSection == INDEX_NONE; }

protected:
	/**
	 * 更新Section内的时间轴
	 *
	 * @param Position				当前帧Section时间轴的位置
	 * @param PreviousPosition		上一帧Section时间轴的位置
	 * @param DeltaTime				当前帧的dt，需要注意的是当帧时间轴的帧差并不一定等于dt
	 * @param SectionIndex			技能中Section的索引
	 */
	void UpdateSectionTimeCursor(float Position, float PreviousPosition, float DeltaTime, int32 SectionIndex);

	/** Section播放结束 */
	void ReachSectionEnd(int32 SectionIndex);

	/** 到达技能时间轴尾端 */
	void ReachAbilityEnd();

	void MoveToSection(int32 NextSection);
public:
	UPROPERTY()
	TObjectPtr<UNeAbility> Ability;

	/** 当前播放的时长 */
	UPROPERTY()
	float PlayedTime = -1;

	/** 当前Section播放时长 */
	UPROPERTY()
	float SubPlayedTime = -1;

	UPROPERTY()
	int32 PlayingSection = -1;
};
