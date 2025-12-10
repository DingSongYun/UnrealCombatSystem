// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_GameplayTask.h"
#include "NeAbilitySegment.h"
#include "NeGameplayAbilityLibrary.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Tasks/NeAbilityTask.h"
#include "NeAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAbilityBeam_GameplayTask)

UNeAbilityBeam_GameplayTask::UNeAbilityBeam_GameplayTask(const FObjectInitializer& Initializer) : Super(Initializer)
{
	ReplicationPolicy = EBeamReplicationPolicy::ClientPredicted;
}

void UNeAbilityBeam_GameplayTask::OnActive(FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnActive(EvalContext);

	CreateTaskInstance(EvalContext.Ability);
	check(TaskInstance);

	TargetingInfos = EvalContext.TargetingInfos;

	// Try to activate
	TaskInstance->ReadyForActivation();
}

void UNeAbilityBeam_GameplayTask::OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnUpdate(DeltaTime, EvalContext);
}

void UNeAbilityBeam_GameplayTask::OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason)
{
	Super::OnEnd(EvalContext, EndReason);

	if (IsValid(TaskInstance) && TaskInstance->IsActive())
	{
		TaskInstance->EndTask();
	}
}

void UNeAbilityBeam_GameplayTask::SamplePosition(const float Position, const float PreviousPosition)
{
	UNeAbilityTask* AbilityTask = Cast<UNeAbilityTask>(TaskInstance);
	if (AbilityTask)
	{
		AbilityTask->SamplePosition(Position, PreviousPosition);
	}
}

void UNeAbilityBeam_GameplayTask::CreateTaskInstance(UGameplayAbility* InAbility)
{
	if (bCompactParams)
	{
		CreateTaskInstanceFromPropertySet(InAbility);
	}
	else
	{
		CreateTaskInstanceFromTemplate(InAbility);
	}
}

void UNeAbilityBeam_GameplayTask::CreateTaskInstanceFromTemplate(UGameplayAbility* Ability)
{
	check(TaskTemplate);
	UClass* TaskClass = TaskTemplate->GetClass();
	UNeAbilityTask* NewTaskInstance = UNeAbilityTask::NewAbilityTaskFromTemplate(Ability, TaskClass, TaskTemplate, InstanceName);
	NewTaskInstance->LinkedBeam = this;
	TaskInstance = NewTaskInstance;
}

void UNeAbilityBeam_GameplayTask::CreateTaskInstanceFromPropertySet(UGameplayAbility* Ability)
{
	UFunction* FactoryFunction = nullptr;
	const UClass* LinkedTaskClass = LinkedClass.ResolveClass();
	for (TFieldIterator<UFunction> FuncIt(LinkedTaskClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
	{
		UFunction* Function = *FuncIt;
		if (!UNeGameplayAbilityLibrary::IsTaskFactoryMethod(Function, LinkedTaskClass))
		{
			continue;
		}
		FactoryFunction = Function;
	}

	if (ensureMsgf(FactoryFunction, TEXT("Can not found factory function on task %s"), *LinkedTaskClass->GetName()))
	{
		// TODO:
		// call ProcessEvent() to execute factory function
	}
}

#if WITH_EDITOR

FText UNeAbilityBeam_GameplayTask::GetDisplayText() const
{
	if (const UNeAbilityTask* NeAbilityTask = Cast<UNeAbilityTask>(TaskTemplate))
	{
		return NeAbilityTask->GetDisplayText();
	}
	else if (TaskTemplate)
	{
		return TaskTemplate->GetClass()->GetDisplayNameText();
	}
	return Super::GetDisplayText();
}

#endif

UClass* UNeAbilityBeam_GameplayTask::GetSupportClass() const
{
	return UAbilityTask::StaticClass();
}
