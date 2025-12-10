// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_RootMotion.h"
#include "NeAbilitySegmentEvalQueue.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/NeRootMotionSource.h"

UNeAbilityBeam_RootMotion::UNeAbilityBeam_RootMotion( const FObjectInitializer& Initializer )
	: Super(Initializer)
{
	bNeedTraceTarget = false;
	bNeedCheckGround = true;
	ClampVelocityOnFinish = 0.f;
}

void UNeAbilityBeam_RootMotion::OnActive(FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnActive(EvalContext);
	for (const FNeAbilityTargetingInfo& TargetingInfo : EvalContext.TargetingInfos)
	{
		ACharacter* TargetCharacter = Cast<ACharacter>(TargetingInfo.SourceActor);
		if (TargetCharacter == nullptr) continue;

		FRootMotionActorMoveInfo& ActorMoveInfo = ActorMoveInfos.Add_GetRef({TargetCharacter, TargetCharacter->GetCharacterMovement()});
		LocatingData.GetLocatingContextBuilder().BuildFromBeam(this).UpdateTarget(TargetCharacter);
		FTransform WorldTransform = LocatingData.GetWorldTransform();
		ActorMoveInfo.StartLocation = WorldTransform.GetLocation();
		ActorMoveInfo.TargetLocation = ActorMoveInfo.StartLocation + WorldTransform.GetRotation().Vector() * Distance;
		// 检测地面
		if (bNeedCheckGround)
		{
			FVector OnGroundLocation;
			if (DoGroundTest(ActorMoveInfo, ActorMoveInfo.TargetLocation, OnGroundLocation))
			{
				ActorMoveInfo.TargetLocation = OnGroundLocation;
			}
		}

		// 切换移动模式
		if (NewMovementMode != EMovementMode::MOVE_None)
		{
			ActorMoveInfo.PreviousMovementMode = ActorMoveInfo.MovementComponent->MovementMode;
			ActorMoveInfo.MovementComponent->SetMovementMode(NewMovementMode);
		}

		FString SourceInstanceName = FString::Printf(TEXT("AbilityBeam_RootMotion"));
		if (bNeedTraceTarget)
		{
			TSharedPtr<FNeRootMotionSource_MoveToDynamicForce> MoveToDynamicForce = MakeShareable(new FNeRootMotionSource_MoveToDynamicForce());
			if (MoveToDynamicForce)
			{
				InitRootMotionSource(MoveToDynamicForce, *SourceInstanceName);
				MoveToDynamicForce->bRestrictSpeedToExpected = false;
				MoveToDynamicForce->PathOffsetCurve = PathOffsetVectorCurve;
				MoveToDynamicForce->TimeMappingCurve = TimeMappingFloatCurve;
				MoveToDynamicForce->StartLocation = ActorMoveInfo.StartLocation;
				MoveToDynamicForce->TargetLocation = ActorMoveInfo.TargetLocation;
				MoveToDynamicForce->InitialTargetLocation = ActorMoveInfo.TargetLocation;
				MoveToDynamicForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::UseSensitiveLiftoffCheck);
				ActorMoveInfo.RootMotionSourceId = ActorMoveInfo.MovementComponent->ApplyRootMotionSource(MoveToDynamicForce);
				ActorMoveInfo.RootMotionSourcePtr = MoveToDynamicForce;
			}
		}
		else
		{
			TSharedPtr<FNeRootMotionSource_MoveToForce> MoveToForce = MakeShareable(new FNeRootMotionSource_MoveToForce());
			if (MoveToForce)
			{
				InitRootMotionSource(MoveToForce, *SourceInstanceName);
				MoveToForce->bRestrictSpeedToExpected = false;
				MoveToForce->PathOffsetCurve = PathOffsetVectorCurve;
				MoveToForce->TimeMappingCurve = TimeMappingFloatCurve;
				MoveToForce->StartLocation = ActorMoveInfo.StartLocation;
				MoveToForce->TargetLocation = ActorMoveInfo.TargetLocation;
				ActorMoveInfo.RootMotionSourceId = ActorMoveInfo.MovementComponent->ApplyRootMotionSource(MoveToForce);
				ActorMoveInfo.RootMotionSourcePtr = MoveToForce;
			}
		}
		
	}
}

void UNeAbilityBeam_RootMotion::OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnUpdate(DeltaTime, EvalContext);

	if (bNeedTraceTarget)
	{
		for (FRootMotionActorMoveInfo& ActorMoveInfo : ActorMoveInfos)
		{
			LocatingData.GetLocatingContextBuilder().UpdateTarget(ActorMoveInfo.Character.Get());
			FTransform WorldTransform = LocatingData.GetWorldTransform();
			ActorMoveInfo.StartLocation = WorldTransform.GetLocation();
			FVector NewTargetLocation = ActorMoveInfo.StartLocation + WorldTransform.GetRotation().Vector() * Distance;
			// 检测地面
			if (bNeedCheckGround)
			{
				FVector OnGroundLocation;
				if (DoGroundTest(ActorMoveInfo, ActorMoveInfo.TargetLocation, OnGroundLocation))
				{
					NewTargetLocation = OnGroundLocation;
				}
			}
			// TODO: 这里可以加lerp过程防止target抖动过快
			ActorMoveInfo.TargetLocation = NewTargetLocation;
			if (ActorMoveInfo.RootMotionSourcePtr->GetScriptStruct() == FNeRootMotionSource_MoveToDynamicForce::StaticStruct())
			{
				TSharedPtr<FNeRootMotionSource_MoveToDynamicForce> MoveToDynamicForce = StaticCastSharedPtr<FNeRootMotionSource_MoveToDynamicForce>(ActorMoveInfo.RootMotionSourcePtr);
				MoveToDynamicForce->SetTargetLocation(ActorMoveInfo.TargetLocation);
			}
		}
	}
}

void UNeAbilityBeam_RootMotion::OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason)
{
	Super::OnEnd(EvalContext, EndReason);
	for (const FRootMotionActorMoveInfo& ActorMoveInfo : ActorMoveInfos)
	{
		ActorMoveInfo.MovementComponent->RemoveRootMotionSourceByID(ActorMoveInfo.RootMotionSourceId);
		if (ActorMoveInfo.PreviousMovementMode != EMovementMode::MOVE_None)
		{
			ActorMoveInfo.MovementComponent->SetMovementMode(ActorMoveInfo.PreviousMovementMode);
		}
	}
	ActorMoveInfos.Empty();
}

void UNeAbilityBeam_RootMotion::InitRootMotionSource(TSharedPtr<FRootMotionSource> RootMotionSource, const FName& InstanceName) const
{
	RootMotionSource->InstanceName = InstanceName;
	RootMotionSource->AccumulateMode = AccumulateMode;
	RootMotionSource->Settings.SetFlag(ERootMotionSourceSettingsFlags::UseSensitiveLiftoffCheck);
	RootMotionSource->Priority = 1000;
	RootMotionSource->Duration = GetDuration();
	RootMotionSource->FinishVelocityParams.Mode = VelocityModeOnFinish;
	RootMotionSource->FinishVelocityParams.SetVelocity = SetVelocityOnFinish;
	RootMotionSource->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;
}

bool UNeAbilityBeam_RootMotion::DoGroundTest(const FRootMotionActorMoveInfo& ActorMoveInfo, const FVector& TestLocation, FVector& OutOnGroundLocation) const
{
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(ActorMoveInfo.Character);
	IgnoreActors.Add(GetOwnerActor());
	FHitResult HitResult;
	if (UKismetSystemLibrary::LineTraceSingleForObjects
	(
		this, TestLocation, TestLocation + ActorMoveInfo.Character->GetActorUpVector() * -3000.0f,
		GroundCheckObjectTypes, false, IgnoreActors, EDrawDebugTrace::None,
		HitResult, true
	))
	{
		OutOnGroundLocation = HitResult.ImpactPoint;

		return true;
	}

	return false;
}
