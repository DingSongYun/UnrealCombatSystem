// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilitySystemComponent.h"

#include "AbilitySystemLog.h"
#include "NeAbility.h"
#include "NeAbilityTargetData.h"
#include "Beams/NeAbilityBeam.h"
#include "Engine/BlueprintGeneratedClass.h"

bool UNeAbilitySystemComponent::GetShouldTick() const
{
	// TODO: 判断有没有在运行的TimelineBased的技能
	const bool bHasReplicatedTimelineBasedAbility = true;
	if (bHasReplicatedTimelineBasedAbility) return true;

	return Super::GetShouldTick();
}

void UNeAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Tick Activating ability
	IncrementAbilityListLock();
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (IsAbilityPlaying(AbilitySpec))
		{
			TArray<UGameplayAbility*> AbilityInstances = GetPlayingInstance(AbilitySpec);
			for (UGameplayAbility* AbilityInstance : AbilityInstances)
			{
				UNeAbility* NeAbilityInstance = Cast<UNeAbility>(AbilityInstance);
				if (NeAbilityInstance && NeAbilityInstance->ShouldTick())
				{
					NeAbilityInstance->Tick(DeltaTime);
				}
			}
		}
	}
	DecrementAbilityListLock();

	// Evaluate Segment if need
	if (SegmentQueue.Evaluating.Num())
	{
		EvaluateSegments(DeltaTime);
	}
}

void UNeAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
	if (BPClass != NULL)
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UNeAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	AbilityEndedEvent.Broadcast(FAbilityEndedData(Ability, Handle, false, bWasCancelled));
}

bool UNeAbilitySystemComponent::TryActivateAbilityWithData(FGameplayAbilitySpecHandle AbilityToActivate, const FGameplayEventData& GameplayEventData)
{
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate);
	if (!ensureMsgf(Spec, TEXT("Failed to find gameplay ability spec")))
	{
		return false;
	}

	if (!InternalTryActivateAbility(AbilityToActivate, FPredictionKey(), nullptr, nullptr, &GameplayEventData))
	{
		return false;
	}

	return true;
}

bool UNeAbilitySystemComponent::EndAbility(const FGameplayAbilitySpecHandle& InSpecHandle, bool bWasCancelled)
{
	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(InSpecHandle))
	{
		ClientEndAbility(InSpecHandle, Spec->ActivationInfo);
	}

	return false;
}

bool UNeAbilitySystemComponent::IsAbilityPlaying(const FGameplayAbilitySpec& AbilitySpec) const
{
	return AbilitySpec.IsActive();
}

bool UNeAbilitySystemComponent::IsAbilityPlaying(const FGameplayAbilitySpecHandle& InSpecHandle) const
{
	if (const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(InSpecHandle))
	{
		return IsAbilityPlaying(*Spec);
	}

	return false;
}

int32 UNeAbilitySystemComponent::SendGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& Payload, bool bTriggerAbility)
{
	check(EventTag.IsValid());
	FScopedPredictionWindow NewScopedWindow(this, true);
	if (bTriggerAbility)
	{
		return HandleGameplayEvent(EventTag, &Payload);
	}

	if (const FGameplayEventMulticastDelegate* Delegate = GenericGameplayEventCallbacks.Find(EventTag))
	{
		// Make a copy before broadcasting to prevent memory stomping
		FGameplayEventMulticastDelegate DelegateCopy = *Delegate;
		DelegateCopy.Broadcast(&Payload);
	}

	// Make a copy in case it changes due to callbacks
	TArray<TPair<FGameplayTagContainer, FGameplayEventTagMulticastDelegate>> LocalGameplayEventTagContainerDelegates = GameplayEventTagContainerDelegates;
	for (TPair<FGameplayTagContainer, FGameplayEventTagMulticastDelegate>& SearchPair : LocalGameplayEventTagContainerDelegates)
	{
		if (SearchPair.Key.IsEmpty() || EventTag.MatchesAny(SearchPair.Key))
		{
			SearchPair.Value.Broadcast(EventTag, &Payload);
		}
	}

	return 0;
}

TArray<UGameplayAbility*> UNeAbilitySystemComponent::GetPlayingInstance(const FGameplayAbilitySpec& InSpec) const
{
	check(IsAbilityPlaying(InSpec));

	TArray<UGameplayAbility*> Abilities;

	if (InSpec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		Abilities.Add(InSpec.Ability);
	}
	else
	{
		for (const TObjectPtr<UGameplayAbility>& Ability : InSpec.ReplicatedInstances)
		{
			if (Ability->IsActive())
			{
				Abilities.Add(Ability);
			}
		}

		for (const TObjectPtr<UGameplayAbility>& Ability : InSpec.NonReplicatedInstances)
		{
			if (Ability->IsActive())
			{
				Abilities.Add(Ability);
			}
		}
	}

	return Abilities;
}

TArray<UGameplayAbility*> UNeAbilitySystemComponent::GetPlayingInstance(const FGameplayAbilitySpecHandle& InSpecHandle) const
{
	if (const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(InSpecHandle))
	{
		return GetPlayingInstance(*Spec);
	}

	return TArray<UGameplayAbility*>();
}

UGameplayAbility* UNeAbilitySystemComponent::GetPlayingAbilityInstance(const FGameplayAbilitySpecHandle& InSpecHandle) const
{
	TArray<UGameplayAbility*> Abilities = GetPlayingInstance(InSpecHandle);

	if (!Abilities.IsEmpty())
	{
		return Abilities[0];
	}

	return nullptr;
}

void UNeAbilitySystemComponent::PushSegmentQueue(const FAbilitySegmentQueue& Queue)
{
	SegmentQueue.Evaluating.Append(Queue.Evaluating);
}

void UNeAbilitySystemComponent::EvaluateSegments(float DeltaTime)
{
	// TODO: 待实现
		// for (FNeAbilitySegmentEvalContext& SegmentEval : SegmentQueue.Evaluating)
	for (auto It = SegmentQueue.Evaluating.CreateIterator(); It; ++ It)
	{
		FNeAbilitySegmentEvalContext& SegmentEval = *It;
		if (SegmentEval.BeamInstance->HasFinished())
		{
			ABILITY_LOG(Warning, TEXT("Segment(%s) has already finished."), *SegmentEval.ToString());
			It.RemoveCurrent();
			continue;
		}

		SegmentEval.BeamInstance->Update(DeltaTime, SegmentEval);

		if (SegmentEval.BeamInstance->ShouldFinish())
		{
			It.RemoveCurrent();
			SegmentEval.BeamInstance->End(SegmentEval, EAbilityBeamEndReason::Finished);

			if (UE_LOG_ACTIVE(VLogAbilitySystem, Log))
			{
				UE_VLOG(GetOwner(), VLogAbilitySystem, Log, TEXT("End segment %s. End Reason: %d"), *SegmentEval.GetSegment().GetName().ToString(), EAbilityBeamEndReason::Finished);
			}
		}
	}

}

FGameplayEventData UNeAbilitySystemComponent::MakeActivateEventData(AActor* Instigator, AActor* Target, EActivateAbilityCheckMethod InCheckMethod) const
{
	TArray<AActor*> Targets = { Target };
    return MakeActivateEventData(Instigator, Targets, InCheckMethod);
}

FGameplayEventData UNeAbilitySystemComponent::MakeActivateEventData(AActor* Instigator, TArray<AActor*> Targets, EActivateAbilityCheckMethod InCheckMethod) const
{
	FGameplayEventData Payload;
	Payload.Instigator = Instigator;
	Payload.Target = GetAvatarActor();

	FNeAbilityTargetData_Activation* ActivationTargetData = new FNeAbilityTargetData_Activation();
	for (AActor* Target : Targets)
	{
		ActivationTargetData->Targets.Add( {Target} );
	}
	ActivationTargetData->ActivateCheckMethod = InCheckMethod;

	Payload.TargetData = FGameplayAbilityTargetDataHandle(ActivationTargetData);
	return Payload;
}

FGameplayEventData UNeAbilitySystemComponent::MakeActivateEventData(const FNeAbilityActivateParameters& ActivateParameters) const
{
	FGameplayEventData Payload;
	// Payload.EventTag = NeAbilityTags::GameplayEvent_Activate;
	Payload.Instigator = ActivateParameters.Instigator;
	Payload.Target = GetAvatarActor();

	FNeAbilityTargetData_Activation* ActivationTargetData = new FNeAbilityTargetData_Activation();
	ActivationTargetData->Targets = ActivateParameters.Targets;
	ActivationTargetData->ActivateCheckMethod = ActivateParameters.ActivateCheckMethod;
	ActivationTargetData->Data = ActivateParameters.Data;

	Payload.TargetData = FGameplayAbilityTargetDataHandle(ActivationTargetData);
	return Payload;
}

void UNeAbilitySystemComponent::AbilityInputPressed(const FGameplayTag& AbilityTag)
{
	FGameplayAbilitySpec* AbilitySpec = nullptr;
	for (FGameplayAbilitySpec& Spec: ActivatableAbilities.Items)
	{
		if (Spec.Ability && (Spec.DynamicAbilityTags.HasTagExact(AbilityTag)))
		{
			AbilitySpec = &Spec;
			break;
		}
	}
	if (AbilitySpec == nullptr)
	{
		if (GameplayEventTriggeredAbilities.Contains(AbilityTag))
		{
			TArray<FGameplayAbilitySpecHandle> TriggeredAbilityHandles = GameplayEventTriggeredAbilities[AbilityTag];
			if (TriggeredAbilityHandles.Num())
			{
				AbilitySpec = FindAbilitySpecFromHandle(TriggeredAbilityHandles[0]);
			}
		}
	}

	if (AbilitySpec)
	{
		AbilitySpecInputPressed(*AbilitySpec);
		// TODO: 这里需要优化, 否则Replicate的东西会有点多
		if (AbilitySpec->IsActive())
		{
			const UNeAbility* Ability = Cast<UNeAbility>(AbilitySpec->Ability);
			if ( Ability && Ability->bReplicateInputDirectly == false && (Ability->InputReplicatePolicy & (1 << static_cast<int32>(EAbilityInputReplicatePolicy::Pressed))) )
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec->Handle, AbilitySpec->ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

void UNeAbilitySystemComponent::AbilityInputReleased(const FGameplayTag& AbilityTag)
{
	FGameplayAbilitySpec* AbilitySpec = nullptr;
	for (FGameplayAbilitySpec& Spec: ActivatableAbilities.Items)
	{
		if (Spec.Ability && (Spec.DynamicAbilityTags.HasTagExact(AbilityTag)))
		{
			AbilitySpec = &Spec;
			break;
		}
	}
	if (AbilitySpec == nullptr)
	{
		if (GameplayEventTriggeredAbilities.Contains(AbilityTag))
		{
			TArray<FGameplayAbilitySpecHandle> TriggeredAbilityHandles = GameplayEventTriggeredAbilities[AbilityTag];
			if (TriggeredAbilityHandles.Num())
			{
				AbilitySpec = FindAbilitySpecFromHandle(TriggeredAbilityHandles[0]);
			}
		}
	}

	if (AbilitySpec)
	{
		AbilitySpecInputReleased(*AbilitySpec);
		if (AbilitySpec->IsActive())
		{
			const UNeAbility* Ability = Cast<UNeAbility>(AbilitySpec->Ability);
			if ( Ability && Ability->bReplicateInputDirectly == false && (Ability->InputReplicatePolicy & (1 << static_cast<int32>(EAbilityInputReplicatePolicy::Released))) )
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec->Handle, AbilitySpec->ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

bool UNeAbilitySystemComponent::K2_InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::Type EventType, FGameplayAbilitySpecHandle AbilityHandle)
{
	// TODO:
	return false;
}

bool UNeAbilitySystemComponent::K2_InvokeReplicatedEventWithPayload(EAbilityGenericReplicatedEvent::Type EventType, FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize100 VectorPayload)
{
	// TODO:
	return true;
}

FGameplayAbilitySpecHandle UNeAbilitySystemComponent::FindAbilitySpecHandleFromClass(TSubclassOf<UGameplayAbility> InAbilityClass, bool bGiveIfNotFound)
{
	if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(InAbilityClass))
	{
		return AbilitySpec->Handle;
	}

	// Give ability if not found
	if (bGiveIfNotFound)
	{
		const FGameplayAbilitySpec AbilitySpec = BuildAbilitySpecFromClass(InAbilityClass, 0, 0);
		return GiveAbility(AbilitySpec);
	}

	static FGameplayAbilitySpecHandle SpecHandle_Invalid;
	return SpecHandle_Invalid;
}

void UNeAbilitySystemComponent::AddDynamicAbilityTag(const FGameplayAbilitySpecHandle& AbilitySpecHandle, const FGameplayTag& Tag) const
{
	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilitySpecHandle))
	{
		Spec->DynamicAbilityTags.AddTag(Tag);
	}
}

void UNeAbilitySystemComponent::RemoveDynamicAbilityTag(const FGameplayAbilitySpecHandle& AbilitySpecHandle, const FGameplayTag& Tag) const
{
	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilitySpecHandle))
	{
		Spec->DynamicAbilityTags.RemoveTag(Tag);
	}
}

void UNeAbilitySystemComponent::RegisterAnyTagChangedEvent(UObject* InBindingObject, const FOnAnyGameplayTagCountChangedDDG& Delegate)
{
	UObject* DelegateObject = const_cast<UObject*>(Delegate.GetUObject());
	DelegateBindingStubs.Add({
		InBindingObject, RegisterGenericGameplayTagEvent().AddUFunction(DelegateObject, Delegate.GetFunctionName())
	});
}

void UNeAbilitySystemComponent::UnregisterAnyTagChangedEvent(UObject* InBindingObject)
{
	for (const FNeScriptDelegateBindingStub& BindingStub: DelegateBindingStubs)
	{
		if (BindingStub.BindingObject == InBindingObject)
		{
			RegisterGenericGameplayTagEvent().Remove(BindingStub.BindingHandler);
		}
	}
}
