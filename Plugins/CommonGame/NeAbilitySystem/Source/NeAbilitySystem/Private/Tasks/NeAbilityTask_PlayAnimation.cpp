// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Tasks/NeAbilityTask_PlayAnimation.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "MotionWarpingComponent.h"
#include "NeAbilityConsoleVariables.h"
#include "NeGameplayAbilityLibrary.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

#include "DrawDebugHelpers.h"
#include "Beams/NeAbilityBeam_GameplayTask.h"

UNeAbilityTask_PlayAnimation::UNeAbilityTask_PlayAnimation(const FObjectInitializer& Initializer) : Super(Initializer)
																									, bOverride_BlendOut(false)
																									, Rate(1)
																									, bStopWhenEnds(true)
																									, bUseMotionWarp(false)
																									, AnimRootMotionTranslationScale(1)
{
}

void UNeAbilityTask_PlayAnimation::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!SkeletalMeshName.IsNone())
	{
		AActor* Actor = ActorInfo->AvatarActor.Get();
		SkeletalMeshPlayOn = Cast<USkeletalMeshComponent>(UNeGameplayAbilityLibrary::GetComponentByNameOrTag(SkeletalMeshName, Actor));
		ensureMsgf(SkeletalMeshPlayOn || SkeletalMeshPlayOn->GetAnimInstance(), TEXT("Can not play animation on component: %s"), *SkeletalMeshName.ToString());
	}
	if (SkeletalMeshPlayOn == nullptr || SkeletalMeshPlayOn->GetAnimInstance() == nullptr)
	{
		SkeletalMeshPlayOn = ActorInfo->SkeletalMeshComponent.Get();
	}


	bool bPlayedMontage = false;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		UAnimInstance* AnimInstance = GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			// TODO: 目前AbilitySystemComponent 只支持在主SkeletalMesh上播放动画
			if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageAsset, Rate, StartSection, 0) > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}


				BlendingOutDelegate.BindUObject(this, &UNeAbilityTask_PlayAnimation::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageAsset);

				MontageEndedDelegate.BindUObject(this, &UNeAbilityTask_PlayAnimation::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageAsset);

				bPlayedMontage = true;
			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UNeAbilityTask_PlayAnimation call to PlayMontage failed!"));
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UNeAbilityTask_PlayAnimation called on invalid AbilitySystemComponent"));
	}

	if (bPlayedMontage)
	{
		ApplyRootMotionScale();
		PerformMotionWarp();
	}

	if (!bPlayedMontage)
	{
		ABILITY_LOG(Warning, TEXT("UNeAbilityTask_PlayAnimation called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageAsset),*InstanceName.ToString());
	}
}

void UNeAbilityTask_PlayAnimation::SamplePosition(const float Position, const float PreviousPosition)
{
	if (UAnimInstance* Instance = GetAnimInstance())
	{
		if (Instance->Montage_IsPlaying(MontageAsset))
		{
			Instance->Montage_Pause(MontageAsset);
		}
		Instance->Montage_SetPosition(MontageAsset, Position);

		// TODO: RootMotion Sample
#if WITH_EDITOR
		if (MontageAsset && bPreviewRootMotion && MontageAsset->HasRootMotion())
		{
			//TObjectPtr<UAnimSequence> RawData = Cast<UAnimSequence>(Montage->SlotAnimTracks[0].AnimTrack.AnimSegments[0].AnimReference);
			//if (RawData && UAnimationBlueprintLibrary::IsRootMotionEnabled(RawData))
			//{
			//	FTransform Pose;
			//	UAnimationBlueprintLibrary::GetBonePoseForTime(RawData, "root", Position, true, Pose);

			//	//const FTransform ActorToWorld = GetBattleInsOwner()->GetTransform();
			//	//const FTransform ComponentToActor = OriginTransform.GetRelativeTransform(SkeletalMeshComponent->GetComponentTransform());
			//	const FTransform NewComponentToActor =Pose * MeshOriginTransform.Inverse();
			//	SkeletalMeshComponent->SetRelativeTransform(NewComponentToActor/*.GetLocation(), NewComponentToActor.GetRotation()*/);
			//}
			FTransform DeltaTranform = MontageAsset->ExtractRootMotionFromTrackRange(PreviousPosition, Position);
			FTransform DeltaWorldTranform = SkeletalMeshPlayOn->ConvertLocalRootMotionToWorld(DeltaTranform);

			AActor* Actor = GetAvatarActor();
			FTransform OldWorldTranform = Actor->GetRootComponent()->GetComponentTransform();
			OldWorldTranform.Accumulate(DeltaWorldTranform);
			Actor->SetActorTransform(OldWorldTranform);
		}
#endif
	}
}

void UNeAbilityTask_PlayAnimation::OnEndTask()
{
	RestoreRootMotionScale();

	AActor* Actor = GetAvatarActor();
	check(Actor);

	if (bStopWhenEnds)
	{
		StopAnimation();
	}

	if (bUseMotionWarp && MontageAsset->HasRootMotion() /*&& Montage->bEnableRootMotionTranslation*/ /*&& !Task->bFixRootMotionScale*/)
	{
		if (UMotionWarpingComponent* WarpingComp = Cast<UMotionWarpingComponent>(Actor->GetComponentByClass(UMotionWarpingComponent::StaticClass())))
		{
			WarpingComp->RemoveWarpTarget(WarpTargetName);
		}
	}

	SkeletalMeshPlayOn = nullptr;

	Super::OnEndTask();
}

bool UNeAbilityTask_PlayAnimation::StopAnimation()
{
	if (Ability == nullptr)
	{
		return false;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && Ability)
	{
		if (ASC->GetAnimatingAbility() == Ability
			&& ASC->GetCurrentMontage() == MontageAsset)
		{
			// Unbind delegates so they don't get called as well
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageAsset);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			ASC->CurrentMontageStop();
			return true;
		}
	}

	return false;
}

void UNeAbilityTask_PlayAnimation::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	const bool bPlayingThisMontage = (Montage == MontageAsset) && Ability && Ability->GetCurrentMontage() == MontageAsset;

	if (bPlayingThisMontage && bInterrupted )
	{
		if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
		{
			ASC->ClearAnimatingAbility(Ability);
		}
	}

}

void UNeAbilityTask_PlayAnimation::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	const bool bPlayingThisMontage = (Montage == MontageAsset) && Ability && Ability->GetCurrentMontage() == MontageAsset;

	if (bPlayingThisMontage)
	{
		EndTask();
	}
}

UAnimInstance* UNeAbilityTask_PlayAnimation::GetAnimInstance() const
{
	return SkeletalMeshPlayOn ? SkeletalMeshPlayOn->GetAnimInstance() : nullptr;
}

void UNeAbilityTask_PlayAnimation::ApplyRootMotionScale()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					  (Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
	{
		Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
	}
}

void UNeAbilityTask_PlayAnimation::RestoreRootMotionScale()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					  (Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
	{
		// TODO: 这里可能缓存之前存储值更好，虽然改的地方不会很多
		Character->SetAnimRootMotionTranslationScale(1.0);
	}
}

void UNeAbilityTask_PlayAnimation::PerformMotionWarp()
{
	if (!bUseMotionWarp || !MontageAsset->HasRootMotion())
	{
		return ;
	}

	const AActor* OwnerActor = GetAvatarActor();
	bool bShouldMotionWarp = false;
	if (OwnerActor && (OwnerActor->GetLocalRole() == ROLE_Authority ||
		(OwnerActor->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
	{
		bShouldMotionWarp = true;
	}

	if (bShouldMotionWarp == false) return;

	if (UMotionWarpingComponent* WarpingComp = Cast<UMotionWarpingComponent>(OwnerActor->GetComponentByClass(UMotionWarpingComponent::StaticClass())))
	{
		// 查找目标的Transform
		if (UNeAbilityBeam_GameplayTask* TaskBeam = Cast<UNeAbilityBeam_GameplayTask>(LinkedBeam))
		{
			FLocatingContextBuilder Builder = WarpTargetOffset.GetLocatingContextBuilder();
			Builder.BuildFromBeam(LinkedBeam);
			if (TaskBeam->TargetingInfos.Num())
			{
				Builder.UpdateTarget(TaskBeam->TargetingInfos[0]);
			}
		}
		FTransform WarpTargetWorldTransform = WarpTargetOffset.GetWorldTransform();
		// 对Warp Target位置进行限制
		const FVector V_Actor_Target = WarpTargetWorldTransform.GetLocation() - GetAvatarActor()->GetActorLocation();
		float DistanceToWarpTarget = V_Actor_Target.Size();
		if (DistanceToWarpTarget >= MinDistanceToWarp && (MaxDistanceToWarp <= 0 || DistanceToWarpTarget <= MaxDistanceToWarp))
		{
			if (WarpTargetGapDistance > 0) DistanceToWarpTarget = FMath::Max(0.f, DistanceToWarpTarget - WarpTargetGapDistance);
			if (WarpScaleLimit > 0) DistanceToWarpTarget = FMath::Clamp(DistanceToWarpTarget, 0.f, RootMotionOriginDistance * WarpScaleLimit);
			WarpTargetWorldTransform.SetLocation(OwnerActor->GetActorLocation() + V_Actor_Target.GetSafeNormal() * DistanceToWarpTarget);

			WarpingComp->AddOrUpdateWarpTargetFromTransform(WarpTargetName, WarpTargetWorldTransform);

			// Draw debug for Motion Warp
#if ENABLE_DRAW_DEBUG
			if (AbilityConsoleVars::bDrawMotionWarpingPoint)
			{
				const float DrawTime = AbilityConsoleVars::MotionWarpingPointDrawTime;
				DrawDebugSphere(GetWorld(), WarpTargetWorldTransform.GetLocation(), 5.f, 32, FColor::Purple, false, DrawTime, 5.0f);
				// 画Debug坐标系
				// X
				DrawDebugDirectionalArrow(
					GetWorld(), WarpTargetWorldTransform.GetLocation(),
					WarpTargetWorldTransform.GetLocation() + WarpTargetWorldTransform.GetUnitAxis(EAxis::X) * 25,
					25.f, FColor::Red, false, DrawTime, 5.0f);

				// Y
				DrawDebugDirectionalArrow(
					GetWorld(), WarpTargetWorldTransform.GetLocation(),
					WarpTargetWorldTransform.GetLocation() + WarpTargetWorldTransform.GetUnitAxis(EAxis::Y) * 25,
					25.f, FColor::Green, false, DrawTime, 5.0f);

				// Z
				DrawDebugDirectionalArrow(
					GetWorld(), WarpTargetWorldTransform.GetLocation(),
					WarpTargetWorldTransform.GetLocation() + WarpTargetWorldTransform.GetUnitAxis(EAxis::Z) * 25,
					25.f, FColor::Blue, false, DrawTime, 5.0f);
			}
#endif
		}
	}
}

FString UNeAbilityTask_PlayAnimation::GetDebugString() const
{
	const UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		const UAnimInstance* AnimInstance = GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageAsset) ? ToRawPtr(MontageAsset) : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWait. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageAsset), *GetNameSafe(PlayingMontage));
}

#if WITH_EDITOR
void UNeAbilityTask_PlayAnimation::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UNeAbilityTask_PlayAnimation, MontageAsset)
		|| PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UNeAbilityTask_PlayAnimation, Rate))
	{
		NotifyAnimRelevanceChanged();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

float UNeAbilityTask_PlayAnimation::EvalAnimRelevanceDuration() const
{
	if (MontageAsset)
	{
		return Rate > 0 ? MontageAsset->GetPlayLength() / Rate : 0.f;
	}

	return 0.f;
}

FText UNeAbilityTask_PlayAnimation::GetDisplayText() const
{
	const FString AnimName = MontageAsset ? MontageAsset->GetName() : FString(TEXT("None"));
	return FText::FromString(FString::Printf(TEXT("%s: %s"), *GetClass()->GetDisplayNameText().ToString(), *AnimName));
}
#endif
