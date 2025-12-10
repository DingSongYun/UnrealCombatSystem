// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Beams/NeAbilityBeam_PlayNiagara.h"
#include "NeGameplayAbilityLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FX/NeNiagaraExtendLibrary.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Misc/NeAbilityGizmo_Niagara.h"

#if WITH_EDITOR
#include "NiagaraSystemEditorData.h"
#endif

UNeAbilityBeam_PlayNiagara::UNeAbilityBeam_PlayNiagara(const FObjectInitializer& Initializer) : Super(Initializer),
	bNeedUseCurve(false),
	bNeedCheckGround(false),
	bAdjustGroundPosition(true),
	bAdjustRotationByGroungNormal(true)
{
	ReplicationPolicy = EBeamReplicationPolicy::ClientOnly;

#if WITH_EDITORONLY_DATA
	bNeedGizmo = true;
	GizmoType = ANeAbilityGizmo_Niagara::StaticClass();
#endif
}

void UNeAbilityBeam_PlayNiagara::OnActive(FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnActive(EvalContext);

	// Should not execute when niagara is empty
	if (NiagaraAsset.IsNull())
	{
		RequestEnd();
		return;
	}

	NiagaraToPlay = NiagaraAsset.LoadSynchronous();
	// StreamableManager.RequestAsyncLoad(SoftObjectPath)

	// UWGNiagaraManager* WGNiagaraMgr = UWGNiagaraManager::GetInstance(this);
	// if (!WGNiagaraMgr) return;

	SpawnedNiagaraInfos.Empty();
	for (const TArray<FNeAbilityTargetingInfo>& TargetInfos = GetTargetInfos(EvalContext); const FNeAbilityTargetingInfo& TargetInfo : TargetInfos)
	{
		if (!TargetInfo.IsValid()) continue;

		// Update Locating Context
		LocatingData.GetLocatingContextBuilder().BuildFromBeam(this).UpdateTarget(TargetInfo);

		FTransform WorldTransform;
		USceneComponent* AttachedComp = nullptr;

		if (bNeedUseCurve)
		{
			// ToDo: ReMap逻辑
			FVector VectorRotator = RotationCurve.GetValue(0);
			FRotator CurveRotator = FRotator(VectorRotator.Y, VectorRotator.Z, VectorRotator.X);
			FTransform CurveOffset = FTransform(CurveRotator, LocationCurve.GetValue(0), ScaleCurve.GetValue(0));
			LocatingData.SetOffsetTransform(CurveOffset);
		}
		WorldTransform = LocatingData.GetWorldTransform();
		AttachedComp = LocatingData.GetBaselineComponent();

		// 针对基准坐标系为镜头坐标系的特殊逻辑
		if (LocatingData.LocatingType == ENeAbilityLocatingOrigin::ALT_Camera)
		{
			//默认原点为镜头前方0.2m，将1920*1080mm尺寸的特效尺寸适配拉伸铺满整个屏幕

			// 偏移0.2m
			FTransform OffsetTransform = LocatingData.TransformAdd;
			FTransform BaseTransform = OffsetTransform.Inverse() * WorldTransform;
			FVector Location = FVector(OffsetTransform.GetLocation());
			Location.X += 20;
			OffsetTransform.SetLocation(Location);
			WorldTransform = OffsetTransform * BaseTransform;

			// 对于一些比如指示器特效，需要考虑是否适配屏幕
			APlayerCameraManager* CameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
			switch (ScaleType)
			{
			case EFxScaleType::Fill:
				{
					float FovDegree = CameraManager->GetFOVAngle();
					float AspectRatio = CameraManager->GetCameraCacheView().AspectRatio;
					float ScaleX = (FMath::Tan(FMath::DegreesToRadians(FovDegree / 2)) * 40) / 1920;
					float ScaleY = (FMath::Tan(FMath::DegreesToRadians(FovDegree / 2)) * 40) / (1080 * AspectRatio);
					WorldTransform.SetScale3D(FVector(ScaleX, ScaleY, 1));
					break;
				}
				case EFxScaleType::FitX:
				{
					float FovDegree = CameraManager->GetFOVAngle();
					float ScaleX = (FMath::Tan(FMath::DegreesToRadians(FovDegree / 2)) * 40) / 1920;
					WorldTransform.SetScale3D(FVector(ScaleX, ScaleX, 1));
					break;
				}
			case EFxScaleType::FitY:
				{
					float FovDegree = CameraManager->GetFOVAngle();
					float AspectRatio = CameraManager->GetCameraCacheView().AspectRatio;
					float ScaleY = (FMath::Tan(FMath::DegreesToRadians(FovDegree / 2)) * 40) / (1080 * AspectRatio);
					WorldTransform.SetScale3D(FVector(ScaleY, ScaleY, 1));
					break;
				}
			case EFxScaleType::None:
			default: ;
			}
		}

		// 检测地面
		if (bNeedCheckGround)
		{
			TArray<AActor*> IgnoreActors;	FHitResult HitResult;
			FVector CurWorldLocation = WorldTransform.GetLocation();

			bool CheckSucess = UKismetSystemLibrary::LineTraceSingleForObjects
			(
				this, CurWorldLocation, CurWorldLocation + FVector(0.f, 0.f, 1.f) * -1000.0f,
				GroundCheckObjectTypes, false, IgnoreActors, EDrawDebugTrace::None,
				HitResult, true
			);

			if (CheckSucess)
			{
				if (bAdjustGroundPosition)
				{
					WorldTransform.SetLocation(HitResult.ImpactPoint);
				}

				if (bAdjustRotationByGroungNormal)
				{
					//FQuat LocalQuat = FQuat::FindBetweenVectors(WorldTransform., HitResult.ImpactNormal);
					FVector ForwardVec = WorldTransform.GetRotation().GetForwardVector();
					FVector RightVec = FVector::CrossProduct(HitResult.ImpactNormal, ForwardVec).GetSafeNormal();
					WorldTransform.SetRotation(UKismetMathLibrary::MakeRotFromYZ(RightVec, HitResult.ImpactNormal).Quaternion());
				}

				// 应用贴地偏移
				WorldTransform = GroundOffset * WorldTransform;
			}
		}

		UNiagaraComponent* NewNiagara = nullptr;
		FNiagaraSpawnParameters SpawnParameters;
		SpawnParameters.Spawner = GetOwnerActor();
		SpawnParameters.bSlomoWithSpawner = bSlomoWithOwner;
		SpawnParameters.bNeedAttach = bNeedAttach;
		SpawnParameters.AttachPolicy = AttachPolicy;
		SpawnParameters.FxSpawnInfo.SystemTemplate = NiagaraAsset.Get();
		SpawnParameters.FxSpawnInfo.AttachToComponent = AttachedComp;
		SpawnParameters.FxSpawnInfo.AttachPointName = LocatingData.Socket;
		SpawnParameters.FxSpawnInfo.Location = WorldTransform.GetLocation();
		SpawnParameters.FxSpawnInfo.Rotation = WorldTransform.GetRotation().Rotator();
		SpawnParameters.FxSpawnInfo.Scale = WorldTransform.GetScale3D();
		SpawnParameters.FxSpawnInfo.LocationType = EAttachLocation::KeepWorldPosition;
		SpawnParameters.FxSpawnInfo.bAutoDestroy = bAutoDestroy;
		SpawnParameters.FxSpawnInfo.PoolingMethod = PoolingMethod;

		NewNiagara = UNeNiagaraExtendLibrary::SpawnNiagaraWithParams(SpawnParameters);

		if (NewNiagara)
		{
			NewNiagara->Activate(true);
			FSpawnedNiagaraInfo& NiagaraInfo = SpawnedNiagaraInfos.AddDefaulted_GetRef();
			NiagaraInfo.NiagaraComponent = NewNiagara;
			NiagaraInfo.TargetActor = TargetInfo.SourceActor;
			NiagaraInfo.TimeStamp = GetWorld()->GetTimeSeconds();
			if (bNeedUseCurve)
			{
				NiagaraInfo.OriginTransform = LocatingData.TransformAdd.Inverse() * WorldTransform;
			}

			ApplyNiagaraUserVariable(NiagaraInfo);
		}
	}
}

void UNeAbilityBeam_PlayNiagara::OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext)
{
	Super::OnUpdate(DeltaTime, EvalContext);

	// Update curves
	if (bNeedUseCurve)
	{
		const float CurveTime = RunningTime;

		// 更新Niagara的位置
		FTransform WorldTransform;
		for (const auto& SpawnedNiagara : SpawnedNiagaraInfos)
		{
			if (!IsValid(SpawnedNiagara.NiagaraComponent)) continue;

			// 是否Attach影响Transform的计算
			if (bNeedAttach)
			{
				const FVector VectorRotator = RotationCurve.GetValue(CurveTime);
				FRotator CurveRotator = FRotator(VectorRotator.Y, VectorRotator.Z, VectorRotator.X);
				const FTransform CurveOffset = FTransform(CurveRotator, LocationCurve.GetValue(CurveTime), ScaleCurve.GetValue(CurveTime));
				LocatingData.TransformAdd = CurveOffset;
				LocatingData.GetLocatingContextBuilder().UpdateTarget(SpawnedNiagara.TargetActor.Get());
				WorldTransform = LocatingData.GetWorldTransform();
			}
			else
			{
				const FVector VectorRotator = RotationCurve.GetValue(CurveTime);
				FRotator CurveRotator = FRotator(VectorRotator.Y, VectorRotator.Z, VectorRotator.X);
				FTransform CurveOffset = FTransform(CurveRotator, LocationCurve.GetValue(CurveTime), ScaleCurve.GetValue(CurveTime));
				WorldTransform = CurveOffset * SpawnedNiagara.OriginTransform;
			}

			SpawnedNiagara.NiagaraComponent->SetWorldTransform(WorldTransform);
		}
	}
}

void UNeAbilityBeam_PlayNiagara::OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason)
{
#if WITH_EDITOR
	// if (TaskTemplate && IsValid(TaskTemplate->GizmoActor))
	// {
	// 	TaskTemplate->GizmoActor->SetIsTemporarilyHiddenInEditor(true);
	// }
#endif

	const bool bManualDestroy = !bAutoDestroy || EndReason == EAbilityBeamEndReason::PreviewOver;

	if (bManualDestroy)
	{
		// 销毁Niagara
		for (int32 i = 0; i < SpawnedNiagaraInfos.Num(); ++i)
		{
			UNiagaraComponent* TheNiagara = SpawnedNiagaraInfos[i].NiagaraComponent;
			if (IsValid(TheNiagara))
			{
#if WITH_EDITOR
				if (EndReason == EAbilityBeamEndReason::PreviewOver)
				{
					TheNiagara->SetAgeUpdateMode(ENiagaraAgeUpdateMode::TickDeltaTime);
					TheNiagara->SetLockDesiredAgeDeltaTimeToSeekDelta(true);
					TheNiagara->SetForceSolo(false);
				}
#endif
				TheNiagara->DeactivateImmediate();
				TheNiagara->ReleaseToPool();
			}
		}
	}

	SpawnedNiagaraInfos.Empty();

	Super::OnEnd(EvalContext, EndReason);
}

void UNeAbilityBeam_PlayNiagara::SamplePosition(const float Position, const float PreviousPosition)
{
	for (const FSpawnedNiagaraInfo& SpawnedNiagara : SpawnedNiagaraInfos)
	{
		UNiagaraComponent* TheNiagara = IsValid(SpawnedNiagara.NiagaraComponent) ? SpawnedNiagara.NiagaraComponent: nullptr;
		if (TheNiagara)
		{
			TheNiagara->SetAgeUpdateMode(ENiagaraAgeUpdateMode::DesiredAge);
			TheNiagara->SetLockDesiredAgeDeltaTimeToSeekDelta(false);
			TheNiagara->SetForceSolo(true);
			// TheNiagara->SetDesiredAge(Position);
			TheNiagara->SeekToDesiredAge(Position);
		}
	}
}

FString UNeAbilityBeam_PlayNiagara::GetDebugString() const
{
	return FString::Printf(TEXT("PlayNiagara. Niagara to play: %s"), *GetNameSafe(NiagaraAsset.Get()));
}

#if WITH_EDITOR
void UNeAbilityBeam_PlayNiagara::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UNeAbilityBeam_PlayNiagara, NiagaraAsset)
		|| PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UNeAbilityBeam_PlayNiagara, PlayRate))
	{
		if (UNiagaraSystem* NiagaraSystem = NiagaraAsset.LoadSynchronous())
		{
			const UNiagaraSystemEditorData* SystemEditorData = Cast<UNiagaraSystemEditorData>(NiagaraSystem->GetEditorData());
			float PlayLength = 5.0;
			if (SystemEditorData != nullptr && SystemEditorData->GetPlaybackRange().HasLowerBound() && SystemEditorData->GetPlaybackRange().HasUpperBound())
			{
				PlayLength = SystemEditorData->GetPlaybackRange().Size<float>();
			}

			SetDuration(PlayLength);
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FText UNeAbilityBeam_PlayNiagara::GetDisplayText() const
{
	return FText::FromString(FString::Printf(TEXT("%s: %s"), *GetClass()->GetDisplayNameText().ToString(), *NiagaraAsset.GetAssetName()));
}
#endif

void UNeAbilityBeam_PlayNiagara::ApplyNiagaraUserVariable(const FSpawnedNiagaraInfo& SpawnedNiagara) const
{
	for (TMap<FString, UStaticMesh*>::TConstIterator It(OverrideStaticMesh); It; ++It)
	{
		const FString& ParamName = It.Key();
		UStaticMesh* StaticMesh = It.Value();
		if (ParamName.IsEmpty()) continue;
		UNiagaraFunctionLibrary::OverrideSystemUserVariableStaticMesh(SpawnedNiagara.NiagaraComponent, ParamName, StaticMesh);
	}

	for (TMap<FString, FName>::TConstIterator It(OverrideSkeletalMeshComponent); It; ++It)
	{
		const FString& ParamName = It.Key();
		FName SkeletalMeshName = It->Value;
		if (ParamName.IsEmpty() || SkeletalMeshName == NAME_None) continue;
		
		if (USkeletalMeshComponent* SkelMeshComp = Cast<USkeletalMeshComponent>(UNeGameplayAbilityLibrary::GetComponentByNameOrTag(SkeletalMeshName, SpawnedNiagara.TargetActor.Get())))
		{
			UNiagaraFunctionLibrary::OverrideSystemUserVariableSkeletalMeshComponent(SpawnedNiagara.NiagaraComponent, ParamName, SkelMeshComp);
		}
	}
}
