// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbility.h"
#include "Beams/NeAbilityBeam.h"
#include "AbilitySystemLog.h"
#include "NeAbilitySegment.h"
#include "NeAbilitySystemComponent.h"
#include "NeAbilitySystemSettings.h"
#include "GameFramework/Character.h"
#include "Misc/DataValidation.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAbility)

UNeAbility::UNeAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bTimelineBasedAbility = true;
	bAutoEndWithTimeline = true;
}

void UNeAbility::PostInitProperties()
{
	Super::PostInitProperties();

	/**
	 * 脚本/蓝图层初始化对象时会根据蓝图进行一次属性复制
	 * 防止改属性被覆盖掉，延迟到construct之后对其进行初始化
	 */
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		TimeController.InitializeFor(this);
	}
}

void UNeAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	ReceiveOnGiveAbility();
}

void UNeAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo,
							FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	if (TriggerEventData != nullptr)
	{
		Instigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
		if (TriggerEventData->TargetData.Num() > 0)
		{
			FGameplayAbilityTargetData* EventData = const_cast<FGameplayAbilityTargetData*>(TriggerEventData->TargetData.Get(0));
			FNeAbilityTargetData_Activation* ActivationData = static_cast<FNeAbilityTargetData_Activation*>(EventData);
			if (ActivationData)
			{
				Targets = ActivationData->Targets;
			}
		}
	}

	// Call Script pre activate
	if (TriggerEventData)
	{
		ReceivePreActivateFromEvent(Handle, *TriggerEventData);
	}
	else
	{
		ReceivePreActivate(Handle);
	}
}

void UNeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PassedSection.Reset();
	TimeController.StartPlaying(GetStartupSectionIndex());
	Advance(0);
}

void UNeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	PassedSection.Reset();
	TimeController.StopPlaying();

	// 处理生命周期还在继续的Segment
	if (SegmentQueue.Evaluating.Num() > 0)
	{
		for (auto It = SegmentQueue.Evaluating.CreateIterator(); It; ++ It)
		{
			if (It->BeamInstance && It->BeamInstance->EffectiveType != EBeamEffectiveType::OnDemand)
			{
				EndSegment(*It, EAbilityBeamEndReason::AbilityEnd);
				It.RemoveCurrent();
			}
		}
		
		UNeAbilitySystemComponent* AbilitySystem = Cast<UNeAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Checked());
		AbilitySystem->PushSegmentQueue(SegmentQueue);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UNeAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	Params.Condition = COND_ReplayOrOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(UNeAbility, SegmentQueue, Params);
}

float UNeAbility::EstimatePlayLength() const
{
	float PlayLength = 0;
	int32 SectionIndex = GetStartupSectionIndex();
	while(IsValidSection(SectionIndex))
	{
		PlayLength += Sections[SectionIndex].SectionDuration;
		SectionIndex = Sections[SectionIndex].NextSection;
	}

	return PlayLength;
}

bool UNeAbility::ShouldTick() const
{
	return IsTimelineBasedAbility();
}

void UNeAbility::Tick(float DeltaTime)
{
	Advance(DeltaTime);
}

ACharacter* UNeAbility::GetOwningCharacter() const
{
	return Cast<ACharacter>(GetOwningActorFromActorInfo());
}

int32 UNeAbility::GetStartupSectionIndex() const
{
	return 0;
}

float UNeAbility::GetPlayingPosition() const
{
	return TimeController.PlayedTime;
}

int32 UNeAbility::GetPlayingSection() const
{
	return TimeController.PlayingSection;
}

void UNeAbility::GetPlayingSection(int32& SectionIndex, float& PlayingPosition) const
{
	SectionIndex = TimeController.PlayingSection;
	PlayingPosition = TimeController.SubPlayedTime;
}

void UNeAbility::Advance(float DeltaTime)
{
	if (TimeController.IsPlaying() == false)
	{
		return ;
	}

	TimeController.Update(DeltaTime);

	if (UE_LOG_ACTIVE(VLogAbilitySystem, Log))
	{
		UE_VLOG(GetOwningActorFromActorInfo(), VLogAbilitySystem, Log, TEXT("Ability playing position: %f"), TimeController.PlayedTime);
		UE_VLOG(GetOwningActorFromActorInfo(), VLogAbilitySystem, Log, TEXT("	Playing section: %d, section position: %f"), TimeController.PlayingSection, TimeController.SubPlayedTime);
	}

	// 执行Segment
	EvaluateSegments(DeltaTime);

	// 时间轴执行完毕
	if (TimeController.HasReachedEnd())
	{
		if (bAutoEndWithTimeline)
		{
			// 处理那些时间轴跟技能时间轴不一致的Segment
			bool bReplicateEndAbility = true;
			bool bWasCancelled = false;
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
		}
		else
		{
			PassedSection.Reset();
			TimeController.StopPlaying();
		}
	}
}

void UNeAbility::UpdateSectionTimeCursor(float CurrentPosition, float PreviousPosition, float DeltaTime, int32 SectionIndex)
{
	if (!ensureMsgf(Sections.IsValidIndex(SectionIndex), TEXT("Invalid section."))) return;

	// 第一次Update(0), 这里对Previous做一次trick操作，以解决start time=0的节点的边界问题
	if (CurrentPosition == PreviousPosition && DeltaTime == 0)
	{
		PreviousPosition = CurrentPosition - 0.001;
	}

	FNeAbilitySection& Section = Sections[SectionIndex];
	for (int32 Index = 0, Num = Section.Segments.Num(); Index < Num; ++ Index)
	{
		const FNeAbilitySegment& Segment = Section.Segments[Index];
		if (!Segment.IsEnable()) continue;

		// Child Segment等待被触发
		if (Segment.HasParent()) continue;
		
		const float StartTime = Segment.GetStartTime();
		if (StartTime > PreviousPosition && StartTime <= CurrentPosition)
		{
			SegmentReadyToActive(SectionIndex, Index);
		}
	}
}

void UNeAbility::OnReachSectionEnd(int32 SectionIndex)
{
	PassedSection.Add(SectionIndex);
}

void UNeAbility::EvaluateSegments(float DeltaTime)
{
	TArray<FNeAbilitySegmentReference> FinishedSegment;

	// 处理执行中的队列
	for (FNeAbilitySegmentEvalContext& SegmentEval : SegmentQueue.Evaluating)
	{
		if (SegmentEval.BeamInstance->HasFinished())
		{
			ABILITY_LOG(Warning, TEXT("Segment(%s) has already finished."), *SegmentEval.ToString());
			FinishedSegment.Add(SegmentEval.Segment);
			continue;
		}

		AdvanceSegment(SegmentEval, DeltaTime);

		if (SegmentEval.BeamInstance->ShouldFinish())
		{
			FinishedSegment.Add(SegmentEval.Segment);
		}
	}

	// 处理当帧需要执行的队列
	SegmentQueue.IncrementSegmentReadyListLock();
	auto ProcessReadyQueue = [&] ()
	{
		for (FNeAbilitySegmentReference& SegmentReference: SegmentQueue.Ready)
		{
			FNeAbilitySegmentEvalContext& SegmentEval = SegmentQueue.Evaluating.Add_GetRef({SegmentReference});
			SegmentEval.Ability = this;
			ActivateSegment(SegmentEval);

			if (SegmentEval.BeamInstance->IsInstant())
			{
				FinishedSegment.Add(SegmentEval.Segment);
			}
		}
		SegmentQueue.Ready.Reset();
	};
	ProcessReadyQueue();
	while(SegmentQueue.PendingReady.Num() > 0)
	{
		SegmentQueue.Ready.Append(SegmentQueue.PendingReady);
		ProcessReadyQueue();
		SegmentQueue.PendingReady.Empty();
	}
	SegmentQueue.DecrementSegmentReadyListLock();

	// 处理Finish队列
	const bool bHasAbilityReachEnd = TimeController.HasReachedEnd() && bAutoEndWithTimeline;
	for (auto It = SegmentQueue.Evaluating.CreateIterator(); It; ++ It)
	{
		if (FinishedSegment.Contains(It->Segment))
		{
			EndSegment(*It, EAbilityBeamEndReason::Finished);
			It.RemoveCurrent();
			break;
		}

		const EBeamEffectiveType EffectiveType = It->BeamInstance->EffectiveType;

		// 技能时间轴播放完毕
		// 非OnDemand的Segment都应该结束掉
		if (bHasAbilityReachEnd && EffectiveType < EBeamEffectiveType::OnDemand)
		{
			EndSegment(*It, EAbilityBeamEndReason::AbilityEnd);
			It.RemoveCurrent();
			break;
		}

		// 生命周期检测
		// 如果所属的Section结束，结束生命周期在时间轴内的Segment
		const int32 SectionIndex = It->Segment.GetSectionIndex();
		const bool bIsSectionPassed = PassedSection.Contains(SectionIndex);
		if (bIsSectionPassed && EffectiveType == EBeamEffectiveType::WithinTimeline)
		{
			EndSegment(*It, EAbilityBeamEndReason::Interrupt);
			It.RemoveCurrent();
			break;
		}
	}

	FinishedSegment.Empty();
}

void UNeAbility::SegmentReadyToActive(int32 SectionIndex, int32 SegmentIndex)
{
	const FNeAbilitySegment& Segment = Sections[SectionIndex].Segments[SegmentIndex];
	const EBeamReplicationPolicy BeamReplicationPolicy = Segment.Beam->ReplicationPolicy;
	const EBeamEvalSpace EvalSpace = Segment.Beam->GetBeamEvalSpace(*CurrentActorInfo);
	if (EvalSpace == EBeamEvalSpace::Local)
	{
		SegmentQueue.AddSegment(SectionIndex, SegmentIndex);
	}
}

void UNeAbility::TriggerSegmentChildren(const FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	const int32 SectionIndex = SegmentEvalContext.Segment.GetSectionIndex();
	FNeAbilitySection& Section = GetSection(SectionIndex);
	const FNeAbilitySegment& Segment = SegmentEvalContext.GetSegment();
	TArray<uint32> Children = Segment.GetAllChildSegments();
	for (const uint32& ChildID : Children)
	{
		const FNeAbilitySegment* ChildSegment = Section.GetSegmentByID(ChildID);
		if (ChildSegment == nullptr) continue;
		if (ChildSegment->TriggerMode == ESegmentTriggerMode::FollowTimeline)
		{
			// TODO:
			// 尝试之后改造成多Timeline驱动
		}
		else if (ChildSegment->TriggerMode == ESegmentTriggerMode::Immediately)
		{
			SegmentQueue.AddSegment(SectionIndex, Section.FindSegmentIndexByID(ChildID));
		}
	}
}

void UNeAbility::EvaluatePropertyBindings(FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	if (GFrameCounter == SegmentEvalContext.LastEvalPropertyBindingFrame)
	{
		// 当帧计算过，不重复计算
		return ;
	}

	for (TSharedRef<FNeAbilityPropertyBindingEval>& PropertyBinding : SegmentEvalContext.RuntimeBindings)
	{
		PropertyBinding->EvalProperty(this, SegmentEvalContext);
	}

	SegmentEvalContext.LastEvalPropertyBindingFrame = GFrameCounter;
}

void UNeAbility::ActivateSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	if (!SegmentEvalContext.Segment.IsValid())
	{
		ABILITY_LOG(Warning, TEXT("Invalid Segment Eval context"));
		return ;
	}

	const FNeAbilitySegment& Segment = SegmentEvalContext.GetSegment();

	CreateBeamEvalInstance(SegmentEvalContext, Segment.GetAbilityBeam());
	check(SegmentEvalContext.BeamInstance);

	// Initialize Property Bindings
	const FName BeamName = Segment.Beam->GetFName();
	SegmentEvalContext.RuntimeBindings.Empty();
	for (const FNeAbilityPropertyBinding& PropertyBinding : PropertyBindings)
	{
		if (PropertyBinding.ObjectName == BeamName)
		{
			TSharedRef<FNeAbilityPropertyBindingEval>& Eval = SegmentEvalContext.RuntimeBindings.Add_GetRef(FNeAbilityPropertyBindingEval::Create(PropertyBinding).ToSharedRef());
			Eval->InitializePropertyBinding(this, Segment.Beam, SegmentEvalContext.BeamInstance);
			Eval->EvalProperty(this, SegmentEvalContext);
		}
	}

	AssembleTargets(SegmentEvalContext.BeamInstance->TargetPolicy, SegmentEvalContext.TargetingInfos, SegmentEvalContext.BeamInstance->CustomTargetKeys);

	SegmentEvalContext.BeamInstance->Active(SegmentEvalContext);

	if (UE_LOG_ACTIVE(VLogAbilitySystem, Log))
	{
		UE_VLOG(GetOwningActorFromActorInfo(), VLogAbilitySystem, Log, TEXT("Active segment %s"), *Segment.GetName().ToString());
	}
}

void UNeAbility::AdvanceSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, float DeltaTime)
{
	check(SegmentEvalContext.BeamInstance);
	SegmentEvalContext.BeamInstance->Update(DeltaTime, SegmentEvalContext);
}

void UNeAbility::EndSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, EAbilityBeamEndReason EndReason)
{
	check(SegmentEvalContext.BeamInstance);

	SegmentEvalContext.BeamInstance->End(SegmentEvalContext, EndReason);

	if (UE_LOG_ACTIVE(VLogAbilitySystem, Log))
	{
		UE_VLOG(GetOwningActorFromActorInfo(), VLogAbilitySystem, Log, TEXT("End segment %s. End Reason: %d"), *SegmentEvalContext.GetSegment().GetName().ToString(), EndReason);
	}
}

UNeAbilityBeam* UNeAbility::CreateBeamEvalInstance(FNeAbilitySegmentEvalContext& SegmentEvalContext, UNeAbilityBeam* ArcheType)
{
	check(IsValid(ArcheType));
	SegmentEvalContext.BeamInstance = NewObject<UNeAbilityBeam>(this, ArcheType->GetClass(), NAME_None, EObjectFlags::RF_NoFlags, ArcheType);
	check(SegmentEvalContext.BeamInstance);
	SegmentEvalContext.BeamInstance->InitInstanceFor(this, SegmentEvalContext);
	return SegmentEvalContext.BeamInstance;
}

void UNeAbility::EvaluateSingleSegment(int32 SectionIndex, int32 SegmentID)
{
	check(IsValidSection(SectionIndex));
	FNeAbilitySection& Section = Sections[SectionIndex];
	SegmentQueue.AddSegment(SectionIndex, Section.FindSegmentIndexByID(SegmentID));
}

EAbilityDataAccessResult UNeAbility::GetDataInteger(const FNeAbilityDataBoardKey& Key, int64& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataFloat(const FNeAbilityDataBoardKey& Key, float& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataDouble(const FNeAbilityDataBoardKey& Key, double& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataBool(const FNeAbilityDataBoardKey& Key, bool& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataVector(const FNeAbilityDataBoardKey& Key, FVector& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataLinearColor(const FNeAbilityDataBoardKey& Key, FLinearColor& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataRotator(const FNeAbilityDataBoardKey& Key, FRotator& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataObject(const FNeAbilityDataBoardKey& Key, UObject*& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataHitResult(const FNeAbilityDataBoardKey& Key, FHitResult& OutValue) const
{
	return DataBoard.GetValue(Key, OutValue);
}

EAbilityDataAccessResult UNeAbility::GetDataStruct(const FNeAbilityDataBoardKey& Key, FInstancedStruct& OutValue) const
{
	return DataBoard.GetValueStruct(Key, OutValue);
}

void UNeAbility::SetDataInteger(const FNeAbilityDataBoardKey& Key, int64 InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataFloat(const FNeAbilityDataBoardKey& Key, float InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataDouble(const FNeAbilityDataBoardKey& Key, double InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataBool(const FNeAbilityDataBoardKey& Key, bool InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataVector(const FNeAbilityDataBoardKey& Key, const FVector& InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataRotator(const FNeAbilityDataBoardKey& Key, const FRotator& InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataObject(const FNeAbilityDataBoardKey& Key, UObject* InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataHitResult(const FNeAbilityDataBoardKey& Key, const FHitResult& InValue)
{
	DataBoard.SetValue(Key, InValue);
}

void UNeAbility::SetDataStruct(const FNeAbilityDataBoardKey& Key, const FInstancedStruct& InValue)
{
	DataBoard.SetValue(Key, InValue.GetScriptStruct(), InValue.GetMemory());
}

void UNeAbility::AssembleTargets(int32 TargetPolicyFlags, TArray<FNeAbilityTargetingInfo>& OutTargets, const FGameplayTagContainer& CustomTargetKeys) const
{
	if (TargetPolicyFlags <= 0) return ;

	for (EBeamTargetPolicy Policy : TEnumRange<EBeamTargetPolicy>())
	{
		const int32 EnumValue = 1 << static_cast<int32>(Policy);
		if ((TargetPolicyFlags & EnumValue) == 0) continue;

		switch (Policy)
		{
		case EBeamTargetPolicy::Owner:
		{
			OutTargets.AddUnique(GetOwningActorFromActorInfo());
			break;
		}
		case EBeamTargetPolicy::Instigator:
		{
			OutTargets.AddUnique(Instigator.Get());
			break;
		}
		case EBeamTargetPolicy::LockTargets:
		{
			OutTargets.Append(Targets);
			break;
		}
		case EBeamTargetPolicy::CollisionResult:
		{
			for (const TArray<FHitResult> HitResults = GetCachedCollisionQueryResults(); const FHitResult& HitResult : HitResults)
			{
				OutTargets.AddUnique(HitResult.GetActor());
			}
			break;
		}
		case EBeamTargetPolicy::CustomTarget:
		{
			for (TArray<FGameplayTag>::TConstIterator It = CustomTargetKeys.CreateConstIterator(); It; ++ It)
			{
				const FGameplayTag& KeyName = *It;
				// 查询KeyName的数据类型定义

				// 如果是 Actor
				UObject* DataObject = nullptr;
				EAbilityDataAccessResult Result = GetDataObject(KeyName, DataObject);
				if (Result == EAbilityDataAccessResult::Success)
				{
					if (AActor* Actor = Cast<AActor>(DataObject))
					{
						OutTargets.AddUnique(Actor);
					}
				}
			}
			break;
		}
		default: check(0);
		} // End Switch
	}
}

#if WITH_EDITOR
void UNeAbility::PostAssetCreate()
{
	if (Sections.Num() == 0)
	{
		const UNeAbilitySystemSettings* AbilitySystemSettings = GetDefault<UNeAbilitySystemSettings>();
		const UClass* PlayerType = AbilitySystemSettings->PlayerType.LoadSynchronous() ? AbilitySystemSettings->PlayerType.Get() : UNeAbilityPreviewActorCommon::StaticClass();
		const UClass* TargetType = AbilitySystemSettings->TargetType.LoadSynchronous() ? AbilitySystemSettings->TargetType.Get() : UNeAbilityPreviewActorCommon::StaticClass();

		AbilityPreviewActors.PlayerInfo = NewObject<UNeAbilityPreviewActorType>(this, PlayerType, NAME_None);
		AbilityPreviewActors.TargetInfo = NewObject<UNeAbilityPreviewActorType>(this, TargetType, NAME_None);

		// Add Default Section
		AddMainSection();
	}
}

void UNeAbility::AddMainSection()
{
	Sections.Empty(1);
	FNeAbilitySection& MainSec = Sections.Add_GetRef({"Main"});
	MainSec.AddDefaultTrackGroup();
}

void UNeAbility::LinkSection(int32 SectionIndex, int32 NextSectionIndex)
{
	check(Sections.IsValidIndex(SectionIndex));
	Sections[SectionIndex].NextSection = NextSectionIndex;
}

void UNeAbility::LinkSection(int32 SectionIndex, FName NextSectionName)
{
	check(Sections.IsValidIndex(SectionIndex));

	for (int32 i = 0; i < Sections.Num(); ++ i)
	{
		if (Sections[i].SectionName == NextSectionName)
		{
			Sections[SectionIndex].NextSection = i;
			break;
		}
	}
}

void UNeAbility::JumpToSectionForPIE(int32 SectionIndex)
{
}

FNeAbilitySegment& UNeAbility::AddNewSegment(const FNeAbilitySegmentDef& SegmentDef, int32 SectionIndex)
{
	check(Sections.IsValidIndex(SectionIndex));
	FNeAbilitySegment& NewSeg = Sections[SectionIndex].AddNewSegment(++ SegmentIDGenerator);
	// Do some initialize of segment
	NewSeg.Construct(this, SegmentDef);

	return NewSeg;
}

void UNeAbility::RemoveSegment(FNeAbilitySegment& Seg)
{
	for (int32 Index = 0; Index < Sections.Num(); ++ Index)
	{
		if (Sections[Index].FindSegmentIndexByID(Seg.GetID()))
		{
			RemoveSegment(Index, Seg);
			return;
		}
	}
}

void UNeAbility::RemoveSegment(int32 SectionIndex, const FNeAbilitySegment& Seg)
{
	check(Sections.IsValidIndex(SectionIndex));
	check(Seg.GetID());
	Sections[SectionIndex].RemoveSegment(Seg);
}

EDataValidationResult UNeAbility::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// Segment ID检查
	for (int32 SectionIndex = 0, Num = Sections.Num(); SectionIndex < Num; ++ SectionIndex)
	{
		const FNeAbilitySection& Section = Sections[SectionIndex];
		for (const FNeAbilitySegment& Segment : Section.Segments)
		{
			if (!Segment.ValidateAsset())
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Segmemt(%d-%d) is validate failed"), SectionIndex, Segment.ID)));
				Result = EDataValidationResult::Invalid;
				break;
			}

			if (Section.FindTrackOfSegment(Segment) == nullptr)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Segmemt(%d-%d) is not in any track."), SectionIndex, Segment.ID)));
			}
		}
	}

	// 其他检查

	return Result;
}

void UNeAbility::TryRepairAsset()
{
	for (FNeAbilitySection& Section : Sections)
	{
		TArray<const FNeAbilitySegment*> SegmentsToRemove;
		
		// 修复为归属Group的Track
		const FNeAbilityTrackGroup& Group = Section.GetDefaultTrackGroup();
		for (FNeAbilityTrack& Track : Section.Tracks)
		{
			if (Track.GroupName.IsNone())
			{
				Track.GroupName = Group.GroupName;
			}
		}

		for (const FNeAbilitySegment& Segment : Section.Segments)
		{
			if (!Segment.ValidateAsset())
			{
				SegmentsToRemove.Add(&Segment);
				break;
			}

			// 删除不在任何Track上的Segment
			if (Section.FindTrackOfSegment(Segment) == nullptr)
			{
				SegmentsToRemove.Add(&Segment);
				break;
			}
		}

		for (const FNeAbilitySegment* Segment : SegmentsToRemove)
		{
			Section.RemoveSegment(*Segment);
		}
		SegmentsToRemove.Empty();
	}
}

void UNeAbility::NotifyPropertyBindingChanged(const FNeAbilityPropertyBinding& BindingChanged)
{
	// Reconstruct PropertyBinding Eval
	for (FNeAbilitySegmentEvalContext& Context : SegmentQueue.Evaluating)
	{
		const FNeAbilitySegment& Segment = Context.GetSegment();
		const FName BeamName = Segment.Beam->GetFName();
		Context.RuntimeBindings.Empty();
		for (const FNeAbilityPropertyBinding& PropertyBinding : PropertyBindings)
		{
			if (PropertyBinding.ObjectName == BeamName)
			{
				TSharedRef<FNeAbilityPropertyBindingEval>& Eval = Context.RuntimeBindings.Add_GetRef(FNeAbilityPropertyBindingEval::Create(PropertyBinding).ToSharedRef());
				Eval->InitializePropertyBinding(this, Segment.Beam, Context.BeamInstance);
			}
		}
	}

}
#endif
