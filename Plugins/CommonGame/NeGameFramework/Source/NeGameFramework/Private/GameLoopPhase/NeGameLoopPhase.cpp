// Copyright NetEase Games, Inc. All Rights Reserved.

#include "GameLoopPhase/NeGameLoopPhase.h"
#include "GameLoopPhase/NeGameLoopPhaseSystem.h"

void UNeGameLoopPhase::OnBeginPhase()
{
	ReceiveOnBeginPhase();
}

void UNeGameLoopPhase::TickPhase(float Delta)
{
	ReceiveTickPhase(Delta);
}

void UNeGameLoopPhase::OnEndPhase()
{
	ReceiveOnEndPhase();
}

UWorld* UNeGameLoopPhase::GetWorld() const
{
	if (const UNeGameLoopPhaseSystem* GLPSystem = Cast<UNeGameLoopPhaseSystem>(GetOuter()))
	{
		return GLPSystem->GetWorld();
	}

	return UObject::GetWorld();
}
