// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityCondition.h"

#include "Beams/NeAbilityBeam.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

UWorld* UNeAbilityCondition::GetWorld() const
{
	if (ContextObject)
	{
		return GEngine->GetWorldFromContextObject(ContextObject, EGetWorldErrorMode::LogAndReturnNull);
	}
	return nullptr;
}

bool UNeAbilityCondition::CheckCondition() const
{
	bool Result = PerformCheck();
	Result = bIsNot ? !Result : Result;
	return Result;
}

ACharacter* UNeAbilityCondition::GetCharacter() const
{
	return Cast<ACharacter>(GetActor());
}

AActor* UNeAbilityCondition::GetActor() const
{
	if (UNeAbilityBeam* Beam = GetOutterBeam())
	{
		return Beam->GetOwnerActor();
	}

	return nullptr;
}

UNeAbilityBeam* UNeAbilityCondition::GetOutterBeam() const
{
	return Cast<UNeAbilityBeam>(ContextObject);
}

//=============================================================================
/**
 * UNeAbilityConditionGroup
 */
void UNeAbilityConditionGroup::Initialize(UObject* WorldContext)
{
	Super::Initialize(WorldContext);

	for (const FNeAbilityCondGroupItem& CondItem : CondItems)
	{
		if (CondItem.Condition)
		{
			CondItem.Condition->Initialize(WorldContext);
		}
	}}

bool UNeAbilityConditionGroup::PerformCheck() const
{
	int32 Num = CondItems.Num();
	if (Num > 0)
	{
		auto __CheckConditionInternal = [] (const FNeAbilityCondGroupItem& CondItem)
		{
			return CondItem.Condition ? CondItem.Condition->CheckCondition() : false;
		};
		bool Result = __CheckConditionInternal(CondItems[0]);

		for (int32 i = 1; i < Num ; ++i)
		{
			if (CondItems[i].Logic == ECondLogic::AND)
			{
				Result = Result && __CheckConditionInternal(CondItems[i]);
			}
			else if (CondItems[i].Logic == ECondLogic::OR)
			{
				Result = Result || __CheckConditionInternal(CondItems[i]);
			}
		}

		return Result;
	}

	return true;}

void UNeAbilityConditionGroup::ResetState()
{
	for (FNeAbilityCondGroupItem& CondItem : CondItems)
	{
		if (CondItem.Condition)
		{
			CondItem.Condition->ResetState();
		}
	}}

void UNeAbilityConditionGroup::Terminate()
{
	Super::Terminate();

	for (const FNeAbilityCondGroupItem& CondItem : CondItems)
	{
		if (CondItem.Condition)
		{
			CondItem.Condition->Terminate();
		}
	}}

void UNeAbilityConditionGroup::AddCondition(UNeAbilityCondition* NewCond)
{
	FNeAbilityCondGroupItem& CondItem = CondItems.AddDefaulted_GetRef();
	CondItem.Condition = NewCond;
}

bool UNeAbilityConditionGroup::RemoveCondition(UNeAbilityCondition* CondToRemove)
{
	check(CondToRemove);
	for (auto It = CondItems.CreateIterator(); It; ++It)
	{
		UNeAbilityCondition* Cond = It->Condition;
		if (Cond == CondToRemove)
		{
			Cond->MarkAsGarbage();
			It.RemoveCurrent();
			return true;
		}
		else if (UNeAbilityConditionGroup* CondGroup = Cast<UNeAbilityConditionGroup>(Cond))
		{
			if (CondGroup->RemoveCondition(CondToRemove))
			{
				return true;
			}
		}
	}

	return false;
}

void UNeAbilityConditionGroup::GetConditions(TArray<FNeAbilityCondGroupItem>& OutConds)
{
	OutConds = CondItems;
}

void UNeAbilityConditionGroup::ClearConditions()
{
	CondItems.Empty();
}
