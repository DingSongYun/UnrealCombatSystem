// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeam.h"
#include "NeAbilityBeam_RootMotion.h"
#include "NeAbilityBeam_RootMotionCurve.generated.h"

/**
 *
 */
UCLASS(Category="Movement", DisplayName="RootMotionCurve(曲线根运动)")
class NEABILITYSYSTEM_API UNeAbilityBeam_RootMotionCurve : public UNeAbilityBeam
{
	GENERATED_BODY()

public:
	/** 时间系数 */
	UPROPERTY(EditAnywhere, Category="RootMotion")
	float TimeCofficient = 1;

	/** 位移曲线 */
	UPROPERTY(EditAnywhere, Category="RootMotion")
	UCurveVector* TranslationCurve = nullptr;

	/** 旋转曲线 */
	UPROPERTY(EditAnywhere, Category="RootMotion")
	UCurveVector* RotationCurve = nullptr;

	/** 自定义RootMotion AccumulateMode应用模式 */
	UPROPERTY(EditAnywhere, Category="RootMotion")
	ERootMotionAccumulateMode AccumulateMode = ERootMotionAccumulateMode::Override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RootMotion|OnFinish")
	ERootMotionFinishVelocityMode VelocityModeOnFinish= ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;

	// Set Velocity if Mode == SetVelocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RootMotion|OnFinish")
	FVector SetVelocityOnFinish;

	// Clamp Velocity if Move == ClampVelocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RootMotion|OnFinish")
	float ClampVelocityOnFinish;

public:
	virtual void OnActive(FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason) override;

protected:
	void InitRootMotionSource(TSharedPtr<FRootMotionSource> RootMotionSource, const FName& InstanceName) const;

protected:
	UPROPERTY()
	TArray<FRootMotionActorMoveInfo> ActorMoveInfos;
};
