// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/NeNiagaraExtendLibrary.h"

#include "NiagaraComponent.h"

UNiagaraComponent* UNeNiagaraExtendLibrary::SpawnNiagaraWithParams(const FNiagaraSpawnParameters& SpawnParams)
{
	const bool bNeedAttach = SpawnParams.bNeedAttach;
	if (SpawnParams.FxSpawnInfo.WorldContextObject == nullptr)
	{
		const_cast<FNiagaraSpawnParameters*>(&SpawnParams)->FxSpawnInfo.WorldContextObject = SpawnParams.Spawner;
	}
	UNiagaraComponent* NewNiagara = nullptr;
	if (bNeedAttach)
	{
		NewNiagara = SpawnSystemAttachedWithParams(SpawnParams.FxSpawnInfo);
		switch (SpawnParams.AttachPolicy)
		{
		case EFxAttachPolicy::OnlyLocation:
			NewNiagara->SetUsingAbsoluteRotation(true);
			break;
		case EFxAttachPolicy::OnlyRotation:
			NewNiagara->SetUsingAbsoluteLocation(true);
			break;
		case EFxAttachPolicy::LocationAndRotation:
			// do nothing
			break;
		}
	}
	else
	{
		NewNiagara = SpawnSystemAtLocationWithParams(SpawnParams.FxSpawnInfo);
	}

	if (SpawnParams.bSlomoWithSpawner)
	{
		check(SpawnParams.Spawner);
		// TODO: Handle slo-mo
		// 需要支持Niagara跟随Spawner的Slomo变化
		NewNiagara->SetCustomTimeDilation(SpawnParams.Spawner->CustomTimeDilation);
	}

	return NewNiagara;
}
