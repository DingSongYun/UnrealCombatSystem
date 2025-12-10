// Copyright NetEase Games, Inc. All Rights Reserved.

#include "GameLoopPhase/NeGameLoopPhaseSystem.h"
#include "GameLoopPhase/NeGameLoopPhase.h"
#include "GameLoopPhase/NeGameLoopPhaseSet.h"

DEFINE_LOG_CATEGORY(LogGameLoopPhase);

void UNeGameLoopPhaseSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!DefaultGameLoopPhaseSet.IsNull())
	{
		if (UNeGameLoopPhaseSet* GameLoopPhaseSet = DefaultGameLoopPhaseSet.LoadSynchronous())
		{
			for (const TSubclassOf<UNeGameLoopPhase>& GameLoopPhase : GameLoopPhaseSet->GameLoopPhases)
			{
				RegisterGameLoopPhase(GameLoopPhase);
			}
		}
	}
}

void UNeGameLoopPhaseSystem::BeginPhase(const FGameplayTag& InPhase)
{
	check(InPhase.IsValid());

	UE_LOG(LogGameLoopPhase, Display, TEXT(">>>>>>>>>>>>>>>> [GameLoop Phase] Begin Phase: <<<<<<<<<<<<<<<<"), *InPhase.ToString());

	UClass* GameLoopPhaseClass = GameLoopPhaseRegistration.Contains(InPhase) ? GameLoopPhaseRegistration[InPhase] : nullptr;
	if (!GameLoopPhaseClass)
	{
		UE_LOG(LogGameLoopPhase, Warning, TEXT("Found no invalid registed game loop phase."));
		return ;
	}

	// 首先结束一些互斥的phase
	TArray<UNeGameLoopPhase*> EndArray;
	ForEach(ActivateGameLoopPhases, [&](UNeGameLoopPhase* PhaseObject)
	{
		if (!InPhase.MatchesTag(PhaseObject->GetGameLoopPhase()))
		{
			EndArray.Add(PhaseObject);
		}
	});
	ForEach(EndArray, [this](UNeGameLoopPhase* PhaseObject) {EndPhase(PhaseObject);});
	EndArray.Empty();

	// 创建GameLoopPhase对象
	UNeGameLoopPhase* NewPhase = NewObject<UNeGameLoopPhase>(this,GameLoopPhaseClass, MakeGameLoopPhaseObjectName(InPhase, GameLoopPhaseClass));
	check(NewPhase);

	NewPhase->OnBeginPhase();

	NotifyPhaseBegin(InPhase);
	ActivateGameLoopPhases.Add(NewPhase);

	UE_LOG(LogGameLoopPhase, Display, TEXT(">>>>>>>>>>>>>>>> [GameLoop Phase] Finish phase beginning. <<<<<<<<<<<<<<<<"));
}

void UNeGameLoopPhaseSystem::EndPhase(UNeGameLoopPhase* InPhase)
{
	check(IsValid(InPhase));
	const FGameplayTag& PhaseTag = InPhase->GetGameLoopPhase();
	UE_LOG(LogGameLoopPhase, Display, TEXT(">>>>>>>>>>>>>>>> [GameLoop Phase] End Phase: %s <<<<<<<<<<<<<<<<"), *PhaseTag.ToString());

	InPhase->OnEndPhase();
	ActivateGameLoopPhases.Remove(InPhase);
	NotifyPhaseEnd(PhaseTag);
	UE_LOG(LogGameLoopPhase, Display, TEXT(">>>>>>>>>>>>>>>> [GameLoop Phase] Finish phase Ending. <<<<<<<<<<<<<<<<"));
}

void UNeGameLoopPhaseSystem::ForEach(const TArray<UNeGameLoopPhase*>& PhaseArray, const TFunction<void(UNeGameLoopPhase*)>& ExeFunc)
{
	for (UNeGameLoopPhase* PhaseObject : PhaseArray)
	{
		ExeFunc(PhaseObject);
	}
}

void UNeGameLoopPhaseSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (UNeGameLoopPhase* Phase : ActivateGameLoopPhases)
	{
		if ( IsValid(Phase)&& Phase->IsTickEnable() )
		{
			Phase->TickPhase(DeltaTime);
		}
	}
}

bool UNeGameLoopPhaseSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

bool UNeGameLoopPhaseSystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UNeGameLoopPhaseSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNeGameLoopPhaseSystem, STATGROUP_Tickables);
}

void UNeGameLoopPhaseSystem::RegisterGameLoopPhase(const TSubclassOf<UNeGameLoopPhase>& NewPhaseClass)
{
	if (!NewPhaseClass)
	{
		return;
	}

	if (const UNeGameLoopPhase* PhaseCDO = NewPhaseClass.GetDefaultObject())
	{
		const FGameplayTag& PhaseTag = PhaseCDO->GetGameLoopPhase();
		if (!PhaseTag.IsValid())
		{
			UE_LOG(LogGameLoopPhase, Warning, TEXT("Invalid game loop phase tag in %s"), *NewPhaseClass->GetName());
			return ;
		}

		if (GameLoopPhaseRegistration.Contains(PhaseTag))
		{
			UE_LOG(LogGameLoopPhase, Warning, TEXT("Duplicate game loop phase tag %s when register %s"), *PhaseTag.ToString(), *NewPhaseClass->GetName());
			return ;
		}
		GameLoopPhaseRegistration.Add(PhaseTag, NewPhaseClass.Get());
	}
}

void UNeGameLoopPhaseSystem::NotifyPhaseBegin(const FGameplayTag& InPhaseTag) const
{
	for (auto It = PhaserObserverMap.CreateConstIterator(); It; ++ It)
	{
		const FGameplayTag& ObserverPhase = It.Key();
		const TArray<FGameLoopPhaseObserver>& Observers = It.Value();
		if (InPhaseTag.MatchesTag(ObserverPhase) && !IsInPhase(ObserverPhase))
		{
			for (const FGameLoopPhaseObserver& Observer : Observers)
			{
				if (Observer.BeginPhaseDelegate.IsBound())
				{
					Observer.BeginPhaseDelegate.Execute(InPhaseTag);
				}
			}
		}
	}
}

void UNeGameLoopPhaseSystem::NotifyPhaseEnd(const FGameplayTag& InPhaseTag) const
{
	for (auto It = PhaserObserverMap.CreateConstIterator(); It; ++ It)
	{
		const FGameplayTag& ObserverPhase = It.Key();
		const TArray<FGameLoopPhaseObserver>& Observers = It.Value();
		if (InPhaseTag.MatchesTag(ObserverPhase) && !IsInPhase(ObserverPhase))
		{
			for (const FGameLoopPhaseObserver& Observer : Observers)
			{
				if (Observer.EndPhaseDelegate.IsBound())
				{
					Observer.EndPhaseDelegate.Execute(InPhaseTag);
				}
			}

		}
	}
}

bool UNeGameLoopPhaseSystem::IsInPhase(const FGameplayTag& PhaseTag) const
{
	for (const UNeGameLoopPhase* Phase : ActivateGameLoopPhases)
	{
		if (IsValid(Phase) && Phase->GetGameLoopPhase().MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

void UNeGameLoopPhaseSystem::AddPhaseBeginObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator, FNeGamePhaseTagDG WhenPhaseBegin)
{
	TArray<FGameLoopPhaseObserver>& PhaseObservers = PhaserObserverMap.FindOrAdd(InPhaseTag);
	FGameLoopPhaseObserver& Observer = PhaseObservers.AddDefaulted_GetRef();
	Observer.Instigator = InInstigator;
	Observer.BeginPhaseDelegate = WhenPhaseBegin;
}

void UNeGameLoopPhaseSystem::AddPhaseEndObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator, FNeGamePhaseTagDG WhenPhaseEnd)
{
	TArray<FGameLoopPhaseObserver>& PhaseObservers = PhaserObserverMap.FindOrAdd(InPhaseTag);
	FGameLoopPhaseObserver& Observer = PhaseObservers.AddDefaulted_GetRef();
	Observer.Instigator = InInstigator;
	Observer.EndPhaseDelegate = WhenPhaseEnd;
}

void UNeGameLoopPhaseSystem::RemovePhaseObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator)
{
	if (!PhaserObserverMap.Contains(InPhaseTag)) return;
	TArray<FGameLoopPhaseObserver>& PhaseObservers = PhaserObserverMap[InPhaseTag];
	for ( auto It = PhaseObservers.CreateIterator(); It; ++It )
	{
		if (It->Instigator == InInstigator)
		{
			It.RemoveCurrent();
		}
	}
}

void UNeGameLoopPhaseSystem::RemovePhaseBeginObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator)
{
	if (!PhaserObserverMap.Contains(InPhaseTag)) return;
	TArray<FGameLoopPhaseObserver>& PhaseObservers = PhaserObserverMap[InPhaseTag];
	for ( auto It = PhaseObservers.CreateIterator(); It; ++It )
	{
		if (It->Instigator == InInstigator && It->BeginPhaseDelegate.IsBound())
		{
			It.RemoveCurrent();
		}
	}
}

void UNeGameLoopPhaseSystem::RemovePhaseEndObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator)
{
	if (!PhaserObserverMap.Contains(InPhaseTag)) return;
	TArray<FGameLoopPhaseObserver>& PhaseObservers = PhaserObserverMap[InPhaseTag];
	for ( auto It = PhaseObservers.CreateIterator(); It; ++It )
	{
		if (It->Instigator == InInstigator && It->BeginPhaseDelegate.IsBound())
		{
			It.RemoveCurrent();
		}
	}
}

void UNeGameLoopPhaseSystem::K2_AddPhaseBeginObserver(const FGameplayTag& PhaseTag, FNeGamePhaseTagDDG WhenPhaseBegin)
{
	const FNeGamePhaseTagDG EndedDelegate = FNeGamePhaseTagDG::CreateWeakLambda(WhenPhaseBegin.GetUObject(), [WhenPhaseBegin](const FGameplayTag& PhaseTag) {
		WhenPhaseBegin.ExecuteIfBound(PhaseTag);
	});

	AddPhaseBeginObserver(PhaseTag, WhenPhaseBegin.GetUObject(), EndedDelegate);
}

void UNeGameLoopPhaseSystem::K2_AddPhaseEndObserver(const FGameplayTag& PhaseTag, FNeGamePhaseTagDDG WhenPhaseEnd)
{
	const FNeGamePhaseTagDG EndedDelegate = FNeGamePhaseTagDG::CreateWeakLambda(WhenPhaseEnd.GetUObject(), [WhenPhaseEnd](const FGameplayTag& PhaseTag) {
		WhenPhaseEnd.ExecuteIfBound(PhaseTag);
	});

	AddPhaseEndObserver(PhaseTag, WhenPhaseEnd.GetUObject(), EndedDelegate);
}

FName UNeGameLoopPhaseSystem::MakeGameLoopPhaseObjectName(const FGameplayTag& InPhase, UClass* PhaseClass) const
{
	return *FString::Printf(TEXT("%s_%s"), *InPhase.ToString(), *PhaseClass->GetName());
}
