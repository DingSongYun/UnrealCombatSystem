// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeam.h"
#include "NeAbilityLocatingData.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "GameFramework/RootMotionSource.h"
#include "NeAbilityBeam_RootMotion.generated.h"

USTRUCT()
struct FRootMotionActorMoveInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TObjectPtr<ACharacter> Character;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	EMovementMode PreviousMovementMode = EMovementMode::MOVE_None;

	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	TSharedPtr<FRootMotionSource> RootMotionSourcePtr;

	uint16 RootMotionSourceId;
};

/**
 * UNeAbilityBeam_RootMotion
 *
 * 添加根运动
 */
UCLASS(Category="Movement", DisplayName="RootMotion(添加根运动)")
class NEABILITYSYSTEM_API UNeAbilityBeam_RootMotion : public UNeAbilityBeam
{
	GENERATED_UCLASS_BODY()

public:
	virtual void OnActive(FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason) override;

protected:
	void InitRootMotionSource(TSharedPtr<FRootMotionSource> RootMotionSource, const FName& InstanceName) const;
	bool DoGroundTest(const FRootMotionActorMoveInfo& ActorMoveInfo, const FVector& TestLocation, FVector& OutOnGroundLocation) const;

public:
	/** 是否追踪Target(注：只在作用在自身情况下该选项生效) */
	UPROPERTY(EditAnywhere, Category="RootMotion")
	uint8 bNeedTraceTarget : 1;

	/** 固定移动距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RootMotion", meta = (EditCondition = "!bNeedTraceTarget"))
	float Distance = 1000.0f;

	/** 追踪目标移动时的最大移动距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RootMotion", meta = (EditCondition = "bNeedTraceTarget"))
	float MaxTraceDistance = 1000.0f;

	// 移动方向的坐标系选取
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RootMotion")
	FNeAbilityLocatingData LocatingData;

	// 完成度曲线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RootMotion")
	TObjectPtr<UCurveFloat> TimeMappingFloatCurve;

	/** 位移偏移曲线 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RootMotion")
	TObjectPtr<UCurveVector> PathOffsetVectorCurve;

	/** 自定义RootMotion过程中的移动模式 */
	UPROPERTY(EditAnywhere, Category="RootMotion")
	TEnumAsByte<EMovementMode> NewMovementMode = EMovementMode::MOVE_None;

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

	// 是否要检测地面，防止角色停在半空中
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RootMotion|Collision")
	bool bNeedCheckGround = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bNeedCheckGround"), Category = "RootMotion|Collision")
	TArray<TEnumAsByte<EObjectTypeQuery> > GroundCheckObjectTypes;

protected:
	UPROPERTY()
	TArray<FRootMotionActorMoveInfo> ActorMoveInfos;
};
