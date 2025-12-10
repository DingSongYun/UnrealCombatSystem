// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityLocatingData.h"
#include "NeAbilityTask.h"
#include "Animation/AnimMontage.h"
#include "NeAbilityTask_PlayAnimation.generated.h"

/**
 * UNeAbilityTask_PlayAnimation
 */
UCLASS(Category="Animation", DisplayName="播放动画")
class NEABILITYSYSTEM_API UNeAbilityTask_PlayAnimation : public UNeAbilityTask
{
	GENERATED_UCLASS_BODY()

public:
	//~BEGIN: UNeAbilityTask interface
	virtual void Activate() override;
	virtual void SamplePosition(const float Position, const float PreviousPosition) override;
	virtual void OnEndTask() override;
	virtual FString GetDebugString() const override;
	//~END: UNeAbilityTask interface

protected:
	/** Stop */
	bool StopAnimation();

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 播放使用的AnimInstance */
	class UAnimInstance* GetAnimInstance() const;

	/** Root Motion Scale */
	void ApplyRootMotionScale();

	/** Motion warp */
	void PerformMotionWarp();

	/** restore root motion */
	void RestoreRootMotionScale();

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual float EvalAnimRelevanceDuration() const override;
	virtual FText GetDisplayText() const override;
#endif

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Overrides, meta=(PinHiddenByDefault, InlineEditConditionToggle))
	uint8 bOverride_BlendOut : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowedClasses = "/Script/Engine.AnimMontage", Time))
	TObjectPtr<UAnimMontage> MontageAsset = nullptr;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float Rate;

	UPROPERTY(EditAnywhere, Category = "Animation")
	FName StartSection = NAME_None;

	/** 想要播放该动画的模型的名称，默认是Character的Mesh */
	UPROPERTY(EditAnywhere, Category = "Animation")
	FName SkeletalMeshName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (editcondition = "bOverride_BlendOut" ))
	float BlendOut = 0.25f;

	/** Task结束时结束 Animation */
	UPROPERTY(EditAnywhere, Category = "Animation")
	uint8 bStopWhenEnds : 1;

	/** 是否使用Motion Warping */
	UPROPERTY(EditAnywhere, Category = "MotionWarping")
	uint8 bUseMotionWarp : 1;

	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "bUseMotionWarp"))
	FName WarpTargetName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "bUseMotionWarp"))
	FNeAbilityLocatingData WarpTargetOffset = FNeAbilityLocatingData();

	/** 该RootMotion蒙太奇最大缩放限制系数, -1 表示无限制 */
	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "bUseMotionWarp"))
	float WarpScaleLimit = -1;

	/** 该RootMotion蒙太奇的原始位移距离 */
	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "bUseMotionWarp && WarpScaleLimit > 0"))
	float RootMotionOriginDistance = 100.0f;

	/** 以目标为圆心，自身距离目标小于此距离不进行MotionWarping */
	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "bUseMotionWarp"))
	float MinDistanceToWarp = 0.f;

	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "bUseMotionWarp"))
	float MaxDistanceToWarp = 0.f;

	/** 防止贴的太近 */
	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "bUseMotionWarp"))
	float WarpTargetGapDistance = 10.f;

	UPROPERTY(EditAnywhere, Category = "MotionWarping", meta = (EditCondition = "!bUseMotionWarp"))
	float AnimRootMotionTranslationScale;

#if WITH_EDITORONLY_DATA
	/** 预览时是否采样根位移 */
	UPROPERTY(EditAnywhere, Category = "Animation", AdvancedDisplay)
	bool bPreviewRootMotion = true;
#endif

private:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshPlayOn = nullptr;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
};
