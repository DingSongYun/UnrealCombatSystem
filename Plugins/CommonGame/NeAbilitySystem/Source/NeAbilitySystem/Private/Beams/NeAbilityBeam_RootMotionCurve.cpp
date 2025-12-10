// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_RootMotionCurve.h"

#include "NeAbilitySegmentEvalQueue.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/NeRootMotionSource.h"

void UNeAbilityBeam_RootMotionCurve::OnActive(FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnActive(EvalContext);

	if (TranslationCurve == nullptr && RotationCurve == nullptr)
	{
		RequestEnd();
		return;
	}

	for (const FNeAbilityTargetingInfo& TargetingInfo : EvalContext.TargetingInfos)
	{
		ACharacter* TargetCharacter = Cast<ACharacter>(TargetingInfo.SourceActor);
		if (TargetCharacter == nullptr) continue;

		FRootMotionActorMoveInfo& ActorMoveInfo = ActorMoveInfos.Add_GetRef({TargetCharacter, TargetCharacter->GetCharacterMovement()});
		FString SourceInstanceName = FString::Printf(TEXT("AbilityBeam_RootMotionCurve"));
		TSharedPtr<FNeRootMotionSource_MoveAlongCurve> MoveAlongCurve = MakeShareable(new FNeRootMotionSource_MoveAlongCurve());
		if (MoveAlongCurve)
		{
			InitRootMotionSource(MoveAlongCurve, *SourceInstanceName);
			MoveAlongCurve->TranslationCurve = TranslationCurve;
			MoveAlongCurve->RotationCurve = RotationCurve;
			MoveAlongCurve->TimeCofficient = TimeCofficient;
			ActorMoveInfo.RootMotionSourceId = ActorMoveInfo.MovementComponent->ApplyRootMotionSource(MoveAlongCurve);
			ActorMoveInfo.RootMotionSourcePtr = MoveAlongCurve;
		}
	}
}

void UNeAbilityBeam_RootMotionCurve::OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnUpdate(DeltaTime, EvalContext);
}

void UNeAbilityBeam_RootMotionCurve::OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason)
{
	Super::OnEnd(EvalContext, EndReason);
	Super::OnEnd(EvalContext, EndReason);
	for (const FRootMotionActorMoveInfo& ActorMoveInfo : ActorMoveInfos)
	{
		if (not IsValid(ActorMoveInfo.MovementComponent)) continue;
		ActorMoveInfo.MovementComponent->RemoveRootMotionSourceByID(ActorMoveInfo.RootMotionSourceId);
		if (ActorMoveInfo.PreviousMovementMode != EMovementMode::MOVE_None)
		{
			ActorMoveInfo.MovementComponent->SetMovementMode(ActorMoveInfo.PreviousMovementMode);
		}
	}
	ActorMoveInfos.Empty();
}

void UNeAbilityBeam_RootMotionCurve::InitRootMotionSource(TSharedPtr<FRootMotionSource> RootMotionSource, const FName& InstanceName) const
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
