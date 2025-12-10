// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityEditorPlayer.h"
#include "NeAbility.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityPreviewInstance.h"
#include "NeAbilityPreviewScene.h"
#include "NeAbilityPreviewSettings.h"
#include "NeAbilitySystemComponent.h"
#include "GameFramework/WorldSettings.h"

//=============================================================================
/**
 * FNeAbilityPreviewProxyBase
 */
TSharedPtr<FNeAbilityEditorPlayerBase> FNeAbilityEditorPlayerBase::Create(UNeAbility* InAsset, const TSharedPtr<class FNeAbilityBlueprintEditor>& InHostEditor)
{
	return MakeShareable(new FNeAbilityEditorPlayerBase(InAsset, InHostEditor));
}

FNeAbilityEditorPlayerBase::FNeAbilityEditorPlayerBase(UNeAbility* InAsset, const TSharedPtr<class FNeAbilityBlueprintEditor>& InHostEditor)
	: AbilityAsset(InAsset), HostEditor(InHostEditor)
{
	check(InHostEditor.IsValid());
	PreviewScene = InHostEditor->GetAbilityPreviewScene();
	check(PreviewScene.Pin());
	PreviewScene.Pin()->OnCreatePlayerDelegate.AddRaw(this, &FNeAbilityEditorPlayerBase::OnCreatePreviewPlayer);
	InitializeFor(GetAbilityComponent());
	CreatePreviewInstance(InAsset);
}

FNeAbilityEditorPlayerBase::~FNeAbilityEditorPlayerBase()
{
	DestroyPreviewInstance();
}

void FNeAbilityEditorPlayerBase::InitializeFor(UNeAbilitySystemComponent* AbilitySystemComponent)
{
	ensureMsgf(AbilitySystemComponent, TEXT("Player has no ability system component."));
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GiveAbility(AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityAsset->GetClass()));
		AbilitySystemComponent->OnAbilityEnded.AddRaw(this, &FNeAbilityEditorPlayerBase::OnPreviewReachEnd);
	}
}

void FNeAbilityEditorPlayerBase::ResetAsset(UNeAbility* InAsset)
{
	AbilityAsset = InAsset;
}

void FNeAbilityEditorPlayerBase::OnResetPreviewWorld()
{
	CleanPreviewTask();
	DestroyPreviewInstance();
	CreatePreviewInstance(AbilityAsset.Get());
	SetPosition(INDEX_NONE, 0);
}

void FNeAbilityEditorPlayerBase::Tick(float DeltaTime)
{
	if (bPlaying)
	{
		const UNeAbility* PlayingInstance = GetPlayingInstance();
		check(PlayingInstance);
		ScrubPosition = PlayingInstance->GetPlayingPosition();
	}
}

UNeAbility* FNeAbilityEditorPlayerBase::GetPlayingInstance() const
{
	if (!PlayingAbilitySpecHandle.IsValid()) return nullptr;

	const UNeAbilitySystemComponent* AbilitySystemComponent = GetAbilityComponent();

	TArray<UGameplayAbility*> AbilitiyInstances = AbilitySystemComponent->GetPlayingInstance(PlayingAbilitySpecHandle);

	return AbilitiyInstances.Num() ? Cast<UNeAbility>(AbilitiyInstances[0]) : nullptr;
}

UNeAbility* FNeAbilityEditorPlayerBase::GetPreviewInstance() const
{
	return PreviewInstance.Get();
}

FNeAbilitySegmentEvalContext* FNeAbilityEditorPlayerBase::GetPreviewSegmentEvalContext(const FWeakAbilitySegmentPtr& InSegmentPtr)
{
	if (PreviewInstance.IsValid())
	{
		return PreviewInstance->GetSegmentPreviewContext(InSegmentPtr);
	}

	return nullptr;
}

void FNeAbilityEditorPlayerBase::AddReferencedObjects(FReferenceCollector& Collector)
{
	UObject* PreviewInstanceObj = PreviewInstance.Get();
	Collector.AddReferencedObject(PreviewInstanceObj);
}

void FNeAbilityEditorPlayerBase::InternalPlay(UNeAbility* InAsset)
{
	UNeAbilitySystemComponent* AbilitySystemComponent = GetAbilityComponent();
	check(AbilitySystemComponent);

	AActor* PlayerActor = GetPlayerActor();
	TArray<AActor*> Targets;
	for (auto TargetActor : PreviewScene.Pin()->GetTargetActors())
	{
		if (TargetActor.IsValid())
		{
			Targets.Add(TargetActor.Get());
		}
	}

	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(InAsset->GetClass(), 0, INDEX_NONE, InAsset);
	const FGameplayEventData Payload = AbilitySystemComponent->MakeActivateEventData(PlayerActor, Targets,  EActivateAbilityCheckMethod::AlwaysActivate);
	AbilitySystemComponent->CancelAllAbilities();
	PlayingAbilitySpecHandle = AbilitySystemComponent->GiveAbilityAndActivateOnce(AbilitySpec, &Payload);
}

void FNeAbilityEditorPlayerBase::InternalPlaySection(int32 Index)
{
	Play();

	CurrentSection = INDEX_NONE;

	UNeAbility* Ability = GetPlayingInstance();
	check(Ability);
	Ability->JumpToSectionForPIE(Index);
}

bool FNeAbilityEditorPlayerBase::IsPlayingSection(UNeAbility* InAsset, int32 Index) const
{
	return CurrentSection == Index;
}

void FNeAbilityEditorPlayerBase::SetPosition(int32 InSectionIndex, float NewPosition)
{
	if (bPlaying)
	{
		Stop();
	}

	CurrentSection = InSectionIndex;
	ScrubPosition = NewPosition;

	if (PreviewInstance.IsValid())
	{
		PreviewInstance->SetPosition(InSectionIndex, ScrubPosition);
	}
}

float FNeAbilityEditorPlayerBase::GetPosition() const
{
	if (bPlaying)
	{
		const UNeAbility* PlayingInstance = GetPlayingInstance();
		return PlayingInstance->GetPlayingPosition();
	}
	return ScrubPosition;
}

int32 FNeAbilityEditorPlayerBase::GetCurrentSection() const
{
	if (bPlaying)
	{
		const UNeAbility* PlayingInstance = GetPlayingInstance();
		int32 SectionIndex; float SectionPosition;
		PlayingInstance->GetPlayingSection(SectionIndex, SectionPosition);
		return SectionIndex;
	}
	return CurrentSection;
}

float FNeAbilityEditorPlayerBase::GetCurrentSectionPosition() const
{
	if (IsPlaying())
	{
		const UNeAbility* PlayingInstance = GetPlayingInstance();
		int32 SectionIndex; float SectionPosition;
		PlayingInstance->GetPlayingSection(SectionIndex, SectionPosition);
		return SectionPosition;
	}
	return ScrubPosition;
}

bool FNeAbilityEditorPlayerBase::IsPlaying() const
{
	return bPlaying && !bPause;
}

bool FNeAbilityEditorPlayerBase::IsPaused() const
{
	return bPlaying && bPause;
}

bool FNeAbilityEditorPlayerBase::IsStopped() const
{
	return !bPlaying;
}

//bool FGBSPreviewProxy::IsLooping() const
//{
//	return bLooping;
//}

void FNeAbilityEditorPlayerBase::Play()
{
	UNeAbility* PreviewAsset = GetPreviewAsset();
	check(PreviewAsset);

	//处理预览模式下TaskIns
	CleanPreviewTask();

	bPlaying = true;
	bPause = false;
	CurrentSection = INDEX_NONE;

	// 如果是技能
	InternalPlay(PreviewAsset);
}

void FNeAbilityEditorPlayerBase::Resume()
{
	bPause = false;
}

void FNeAbilityEditorPlayerBase::Pause()
{
	bPause = true;
}

void FNeAbilityEditorPlayerBase::Stop()
{
	bPlaying = false;
	bPause = false;
}

//void FGBSPreviewProxy::SetLooping(bool bIsLooping)
//{
//	bLooping = bIsLooping;
//}

void FNeAbilityEditorPlayerBase::PlaySection(int32 Index)
{
	InternalPlaySection(Index);
}

bool FNeAbilityEditorPlayerBase::IsPlayingSection(int32 Index) const
{
	UNeAbility* PreviewAsset = GetPreviewAsset();
	check(PreviewAsset);

	return IsPlayingSection(PreviewAsset, Index);
}


void FNeAbilityEditorPlayerBase::CleanPreviewTask()
{
}

void FNeAbilityEditorPlayerBase::OnPreviewReachEnd(const FAbilityEndedData& AbilityEndedData)
{
	if (AbilityEndedData.AbilitySpecHandle != PlayingAbilitySpecHandle)
		return;

	bPlaying = false;
	bPause = false;
	if (UNeAbility* Ability = Cast<UNeAbility>(AbilityEndedData.AbilityThatEnded))
	{
		ScrubPosition = Ability->GetPlayingPosition();
		CurrentSection = Ability->GetPlayingSection();
	}

	if (HostEditor.IsValid())
	{
		const UWorld* World = HostEditor.Pin()->GetPreviewWorld();
		const UNeAbilityPreviewSettings* PreviewSettings = HostEditor.Pin()->GetPreviewSettings();
		if (PreviewSettings->bAutoResetWhenPlayToEnd)
		{
			World->GetTimerManager().ClearTimer(ResetTimerHandler);
			World->GetTimerManager().SetTimer(ResetTimerHandler, [this] {
					if (this->HostEditor.IsValid())
					{
						this->HostEditor.Pin()->ResetPreviewWorld();
					}
				},
			PreviewSettings->AutoResetDelayTime, false);
		}
	}

	PlayingAbilitySpecHandle = FGameplayAbilitySpecHandle();
}

void FNeAbilityEditorPlayerBase::OnCreatePreviewPlayer(AActor* PlayerActor)
{
	InitializeFor(GetAbilityComponent());
}

void FNeAbilityEditorPlayerBase::CreatePreviewInstance(UNeAbility* InAsset)
{
	check(PreviewInstance == nullptr);
	UNeAbilityPreviewInstance* NewInst = NewObject<UNeAbilityPreviewInstance>(PreviewScene.Pin()->GetWorld()->GetWorldSettings());
	if (NewInst)
	{
		UNeAbilitySystemComponent* AbilitySystemComponent = GetAbilityComponent();
		if (AbilitySystemComponent)
		{
			FGameplayAbilitySpec PreviewAbilitySpec = FGameplayAbilitySpec(NewInst, 0, INDEX_NONE, nullptr);
			AbilitySystemComponent->GiveAbility(PreviewAbilitySpec);
			NewInst->InitializeFor(AbilitySystemComponent, InAsset, HostEditor.Pin());
		}
		// NewInst->CallActivateAbility();
	}

	check(NewInst);
	PreviewInstance = NewInst;
}

void FNeAbilityEditorPlayerBase::DestroyPreviewInstance()
{
	if (PreviewInstance.IsValid())
	{
		PreviewInstance->MarkAsGarbage();
		PreviewInstance = nullptr;
	}
}

UNeAbilitySystemComponent* FNeAbilityEditorPlayerBase::GetAbilityComponent() const
{
	if (const AActor* PlayerActor = GetPlayerActor())
	{
		return PlayerActor->FindComponentByClass<UNeAbilitySystemComponent>();
	}

	return nullptr;
}

AActor* FNeAbilityEditorPlayerBase::GetPlayerActor() const
{
	return PreviewScene.Pin()->GetPlayerActor();
}