// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityAssetNodeFactory.h"

#include "NeAbilitySection.h"
#include "NiagaraSystemEditorData.h"
#include "NiagaraSystem.h"
#include "AssetRegistry/AssetData.h"
#include "Beams/NeAbilityBeam_GameplayTask.h"
#include "Beams/NeAbilityBeam_PlayNiagara.h"
#include "Tasks/NeAbilityTask_PlayAnimation.h"
#include "Timeline/NeAbilityTimelineMode.h"

TArray<TWeakObjectPtr<UClass>> UNeAbilityNodeFactory::FactoryTypeMap = {};

UNeAbilityNodeFactory::UNeAbilityNodeFactory(const FObjectInitializer& Initializer) : Super(Initializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		if (UClass* Class = GetClass(); !Class->HasAllClassFlags(EClassFlags::CLASS_Abstract))
		{
			FactoryTypeMap.Add(Class);
		}
	}
}

const UNeAbilityNodeFactory* UNeAbilityNodeFactory::FindFactory(const FAssetData& AssetData)
{
	for (const TWeakObjectPtr<UClass>& FactoryType : FactoryTypeMap)
	{
		UClass* FactoryClass = FactoryType.Get();
		const UNeAbilityNodeFactory* FactoryDefault = GetDefault<UNeAbilityNodeFactory>(FactoryClass);
		if (FactoryDefault->CanCreateNodeFrom(AssetData))
		{
			return FactoryDefault;
		}

	}
	return nullptr;
}

bool UNeAbilityNodeFactory::CanCreateNodeFrom(const FAssetData& AssetData) const
{
	return false;
}

FNeAbilityTrack* UNeAbilityNodeFactory::CreateNewTrack(UObject* InAsset, FNeAbilityTimelineMode& TimelineMode, const FName& OnGroup) const
{
	if (InAsset == nullptr)
	{
		check(false);
		return nullptr;
	}

	FNeAbilitySectionPtr SectionPtr = TimelineMode.GetAbilitySectionPtr();
	const FNeAbilityTrackGroup* Group = SectionPtr->FindGroupOfName(OnGroup);
	if (Group == nullptr) Group = &SectionPtr->GetDefaultTrackGroup();
	check(Group);

	FNeAbilityTrack& NewTrack = TimelineMode.AddNewTrack(*Group, ActionClass);

	return &NewTrack;
}

void UNeAbilityNodeFactory::PostCreateNewTrack(UObject* InAsset, FNeAbilityTrack& NewTrack, FNeAbilitySegment& NewSegment) const
{
}

/**
 * UNeAbilityNodeFactory_Animation
 */
UNeAbilityNodeFactory_Animation::UNeAbilityNodeFactory_Animation(const FObjectInitializer& Initializer) : Super(Initializer)
{
	ActionClass = UNeAbilityTask_PlayAnimation::StaticClass();
}

bool UNeAbilityNodeFactory_Animation::CanCreateNodeFrom(const FAssetData& AssetData) const
{
	UObject* Asset = AssetData.GetAsset();
	return Cast<UAnimMontage>(Asset) != nullptr;
}

void UNeAbilityNodeFactory_Animation::PostCreateNewTrack(UObject* InAsset, FNeAbilityTrack& NewTrack, FNeAbilitySegment& NewSegment) const
{
	if (UAnimMontage* Montage = Cast<UAnimMontage>(InAsset))
	{
		if (UNeAbilityBeam_GameplayTask* Beam = Cast<UNeAbilityBeam_GameplayTask>(NewSegment.GetAbilityBeam()))
		{
			Beam->SetDuration(Montage->GetPlayLength());
			if (UNeAbilityTask_PlayAnimation* AnimationTask = Cast<UNeAbilityTask_PlayAnimation>(Beam->TaskTemplate))
			{
				AnimationTask->MontageAsset = Montage;
			}
		}
	}
}

bool UNeAbilityNodeFactory_Niagara::CanCreateNodeFrom(const FAssetData& AssetData) const
{
	UObject* Asset = AssetData.GetAsset();
	return Cast<UNiagaraSystem>(Asset) != nullptr;
}

/**
 * UNeAbilityNodeFactory_Niagara
 */
UNeAbilityNodeFactory_Niagara::UNeAbilityNodeFactory_Niagara(const FObjectInitializer& Initializer) : Super(Initializer)
{
	ActionClass = UNeAbilityBeam_PlayNiagara::StaticClass();
}

void UNeAbilityNodeFactory_Niagara::PostCreateNewTrack(UObject* InAsset, FNeAbilityTrack& NewTrack, FNeAbilitySegment& NewSegment) const
{
	if (const UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(InAsset))
	{
		if (UNeAbilityBeam_PlayNiagara* Beam = Cast<UNeAbilityBeam_PlayNiagara>(NewSegment.GetAbilityBeam()))
		{
			const UNiagaraSystemEditorData* SystemEditorData = Cast<UNiagaraSystemEditorData>(NiagaraSystem->GetEditorData());
			float PlayLength = 5.0;
			if (SystemEditorData != nullptr && SystemEditorData->GetPlaybackRange().HasLowerBound() && SystemEditorData->GetPlaybackRange().HasUpperBound())
			{
				PlayLength = SystemEditorData->GetPlaybackRange().Size<float>();
			}

			Beam->SetDuration(PlayLength);
			Beam->NiagaraAsset = NiagaraSystem;
		}
	}
}
