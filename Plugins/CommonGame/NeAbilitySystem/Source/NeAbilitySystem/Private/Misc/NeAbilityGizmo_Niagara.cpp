// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Misc/NeAbilityGizmo_Niagara.h"
#include "NeAbilitySegmentEvalQueue.h"
#include "NiagaraComponent.h"
#include "Beams/NeAbilityBeam_PlayNiagara.h"

ANeAbilityGizmo_Niagara::ANeAbilityGizmo_Niagara()
{
	ObservedProperties.Add(GET_MEMBER_NAME_CHECKED(UNeAbilityBeam_PlayNiagara, LocatingData));
	ObservedProperties.Add(GET_MEMBER_NAME_CHECKED(FNeAbilityLocatingData, TransformAdd));
	ObservedProperties.Add(GET_MEMBER_NAME_CHECKED(FNeAbilityLocatingData, LocatingType));
	ObservedProperties.Add(GET_MEMBER_NAME_CHECKED(FNeAbilityLocatingData, DirectionType));
	ObservedProperties.Add(GET_MEMBER_NAME_CHECKED(FNeAbilityLocatingData, Socket));
	ObservedProperties.Add(GET_MEMBER_NAME_CHECKED(FNeAbilityLocatingData, ExtraDirection));
}

void ANeAbilityGizmo_Niagara::OnSynchronizeFromBinding()
{
	const FWeakAbilitySegmentPtr& SegmentPtr = GetBindingSegment();
	UNeAbilityBeam_PlayNiagara* BeamNiagara = Cast<UNeAbilityBeam_PlayNiagara>(SegmentPtr->GetAbilityBeam());
	check(BeamNiagara);
	BeamNiagara->LocatingData.GetLocatingContextBuilder().BuildFromBeam(GetEvalContext().BeamInstance.Get()).UpdateTarget(GetPreviewTarget());
	const FTransform Transform = BeamNiagara->LocatingData.GetWorldTransform();
	SetActorTransform(Transform);

	UpdatePreviewNiagara(Transform);
}

void ANeAbilityGizmo_Niagara::OnGizmoMoved()
{
	const FWeakAbilitySegmentPtr& SegmentPtr = GetBindingSegment();
	UNeAbilityBeam_PlayNiagara* BeamNiagara = Cast<UNeAbilityBeam_PlayNiagara>(SegmentPtr->GetAbilityBeam());
	check(BeamNiagara);
	BeamNiagara->LocatingData.GetLocatingContextBuilder().BuildFromBeam(GetEvalContext().BeamInstance.Get()).UpdateTarget(GetPreviewTarget());
	BeamNiagara->LocatingData.FromWorldTransform(GetActorTransform());

	UpdatePreviewNiagara(GetActorTransform());
}

void ANeAbilityGizmo_Niagara::UpdatePreviewNiagara(const FTransform& NewTransform) const
{
	if (UNeAbilityBeam_PlayNiagara* BeamInstance = Cast<UNeAbilityBeam_PlayNiagara>(GetEvalContext().BeamInstance.Get()))
	{
		const TArray<FSpawnedNiagaraInfo>& NiagaraInfos = BeamInstance->GetSpawnedNiagaraInfos();
		for (const FSpawnedNiagaraInfo& SpawnedNiagara : NiagaraInfos)
		{
			BeamInstance->LocatingData.GetLocatingContextBuilder().BuildFromBeam(BeamInstance).UpdateTarget(SpawnedNiagara.TargetActor.Get());
			SpawnedNiagara.NiagaraComponent->SetWorldTransform(BeamInstance->LocatingData.GetWorldTransform());
		}
	}
}
