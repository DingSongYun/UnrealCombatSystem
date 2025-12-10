// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam.h"

#include "AbilitySystemComponent.h"
#include "NeAbility.h"
#include "NeAbilityTargetData.h"
#include "Misc/NeAbilityGizmoActor.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAbilityBeam)

UNeAbilityBeam::UNeAbilityBeam(const FObjectInitializer& UNeAbilityBeam) : Super(UNeAbilityBeam)
{
	DurationType = EAbilityDurationPolicy::HasDuration;
	bRequestFinished = false;
	bCompound = false;

#if WITH_EDITORONLY_DATA
	bNeedGizmo = false;
	GizmoType = ANeAbilityTransformGizmo::StaticClass();
#endif
}

void UNeAbilityBeam::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	SetFlags(GetFlags() | RF_Transactional);

	ValidateAsset();
#endif
}

void UNeAbilityBeam::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	// 这里计算当前是否有属性绑定
#if WITH_EDITOR
	bHasPropertyBinding = false;
	for (TFieldIterator<FProperty> It(this->GetClass()); It; ++ It)
	{
		if (It->HasMetaData(Name_PropBindingMeta) && It->GetBoolMetaData(Name_PropBindingMeta))
		{
			bHasPropertyBinding = true;
			break;
		}
	}
#endif
}

void UNeAbilityBeam::SetDuration(float NewDuration)
{
	Duration = NewDuration;
}

bool UNeAbilityBeam::IsInstant() const
{
	return DurationType == EAbilityDurationPolicy::Instant || (DurationType == EAbilityDurationPolicy::HasDuration && Duration <= 0);
}

void UNeAbilityBeam::InitInstanceFor(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& EvalContext)
{
	OwnerAbility = InAbility;
}

void UNeAbilityBeam::Active(FNeAbilitySegmentEvalContext& EvalContext)
{
	State = EAbilityBeamState::Running;
	if (bHasPropertyBinding)
	{
		EvalContext.EvaluatePropertyBindings();
	}
	OnActive(EvalContext);
}

void UNeAbilityBeam::Update(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext)
{
	RunningTime += DeltaTime;
	if (bHasPropertyBinding)
	{
		EvalContext.EvaluatePropertyBindings();
	}
	OnUpdate(DeltaTime, EvalContext);
}

void UNeAbilityBeam::End(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason)
{
	if (bHasPropertyBinding)
	{
		EvalContext.EvaluatePropertyBindings();
	}
	State = EndReason == EAbilityBeamEndReason::Interrupt ? EAbilityBeamState::Cancelled : EAbilityBeamState::Finished;
	OnEnd(EvalContext, EndReason);
}

void UNeAbilityBeam::SamplePosition(const float Position, const float PreviousPosition)
{
	// 各个子类实现
}

bool UNeAbilityBeam::ShouldFinish() const
{
	return bRequestFinished
		|| DurationType == EAbilityDurationPolicy::Instant
		|| (DurationType == EAbilityDurationPolicy::HasDuration && RunningTime >= Duration);
}

bool UNeAbilityBeam::HasFinished() const
{
	return State == EAbilityBeamState::Finished || State == EAbilityBeamState::Cancelled;
}

bool UNeAbilityBeam::ShouldReplicate() const
{
	return ReplicationPolicy != EBeamReplicationPolicy::ClientOnly && ReplicationPolicy != EBeamReplicationPolicy::ServerOnly;
}

EBeamEvalSpace UNeAbilityBeam::GetBeamEvalSpace(const FGameplayAbilityActorInfo& ActorInfo) const
{
	const ENetMode NetMode = ActorInfo.OwnerActor->GetNetMode();
	// 单机情况下，所有Beam都本地执行
	if (NetMode == ENetMode::NM_Standalone)
	{
		return EBeamEvalSpace::Local;
	}

	const bool bIsClient = NetMode == ENetMode::NM_Client || NetMode == ENetMode::NM_Standalone;
	// 只在客户端执行
	if (!bIsClient && ReplicationPolicy == EBeamReplicationPolicy::ClientOnly)
	{
		return EBeamEvalSpace::Abort;
	}

	// 仅在服务器执行
	if (bIsClient && ReplicationPolicy == EBeamReplicationPolicy::ServerOnly)
	{
		return EBeamEvalSpace::Abort;
	}

	const bool bHasAuthority = ActorInfo.IsNetAuthority();
	if (!bHasAuthority && ReplicationPolicy == EBeamReplicationPolicy::ServerExec)
	{
		return EBeamEvalSpace::Abort;
	}

	const bool bLocal = ActorInfo.IsLocallyControlled();
	if (ReplicationPolicy == EBeamReplicationPolicy::ClientPredicted && !bLocal && !bHasAuthority)
	{
		return EBeamEvalSpace::Abort;
	}

	return EBeamEvalSpace::Local;
}

FString UNeAbilityBeam::GetDebugString() const
{
	return FString::Printf(TEXT("%s"), *GetName());
}

UWorld* UNeAbilityBeam::GetWorld() const
{
	if (IsValid(OwnerAbility))
	{
		return OwnerAbility->GetWorld();
	}

	return nullptr;
}

AActor* UNeAbilityBeam::GetInstigator() const
{
	return OwnerAbility->GetInstigator();
}

AActor* UNeAbilityBeam::GetOwnerActor() const
{
	return OwnerAbility->GetOwningActorFromActorInfo();
}

AActor* UNeAbilityBeam::GetAvatarActor() const
{
	return OwnerAbility->GetAvatarActorFromActorInfo();
}

TArray<AActor*> UNeAbilityBeam::GetTargetActors(const FNeAbilitySegmentEvalContext& EvalContext) const
{
	TArray<AActor*> TargetActors;
	for (const TArray<FNeAbilityTargetingInfo>& TargetingInfos = GetTargetInfos(EvalContext); const auto& TargetInfo : TargetingInfos)
	{
		if (TargetInfo.SourceActor)
		{
			TargetActors.AddUnique(TargetInfo.SourceActor);
		}
	}

	return TargetActors;
}

AActor* UNeAbilityBeam::GetTargetActor(const FNeAbilitySegmentEvalContext& EvalContext) const
{
	for (const TArray<FNeAbilityTargetingInfo>& TargetingInfos = GetTargetInfos(EvalContext); const auto& TargetInfo : TargetingInfos)
	{
		if (TargetInfo.SourceActor)
		{
			return TargetInfo.SourceActor;
		}
	}

	return nullptr;
}

const TArray<FNeAbilityTargetingInfo>& UNeAbilityBeam::GetTargetInfos(const FNeAbilitySegmentEvalContext& EvalContext) const
{
	return EvalContext.TargetingInfos;
}

void UNeAbilityBeam::DeclareOutDataSlots(int32 OutDataNum)
{
	check(OutDataNum >= 0);
	OutDataEntries.SetNum(OutDataNum);
}

void UNeAbilityBeam::SetOutDataSlot(int32 Index, const FGameplayTag& SlotTag)
{
	if (OutDataEntries.IsValidIndex(Index))
	{
		OutDataEntries[Index] = SlotTag;
	}
}

#if WITH_EDITOR

FText UNeAbilityBeam::GetDisplayText() const
{
	FText DisplayText = GetClass()->GetDisplayNameText();
	return GetClass()->GetDisplayNameText();
}

#endif
