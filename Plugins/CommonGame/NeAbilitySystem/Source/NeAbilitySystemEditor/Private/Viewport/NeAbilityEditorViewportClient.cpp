// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityEditorViewportClient.h"
#include "UnrealEdGlobals.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/WorldSettings.h"
#include "Editor/UnrealEdEngine.h"
#include "SceneViewExtension.h"
#include "IXRTrackingSystem.h"
#include "AssetViewerSettings.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityPreviewSettings.h"
#include "SceneView.h"
#include "Editor/EditorPerProjectUserSettings.h"

const FName NeAbilityEditorViewportCameraMode::Free = "Free";
const FName NeAbilityEditorViewportCameraMode::RealGame = "RealGame";
const FName NeAbilityEditorViewportCameraMode::CameraAttach = "CameraAttach";


FNeAbilityEditorViewportClient::FNeAbilityEditorViewportClient(TSharedPtr<FNeAbilityBlueprintEditor> InHostEditor, FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InViewportWidget)
	: FNeSimpleEdViewportClient(&GLevelEditorModeTools(), InPreviewScene, InViewportWidget)
	, m_CaptureThumbnail(false)
	, ViewportCameraMode(NeAbilityEditorViewportCameraMode::Free)
	, ViewLockedSkel(nullptr)
	, ViewLockedBoneName(NAME_None)
	, ActorLocked(nullptr)
	, CustomDilation(1.0f)
{
	check(InHostEditor.IsValid());
	HostEditor = InHostEditor;

	PlaybackSpeedMode = ENeAbilityPlaybackSpeeds::Normal;
	//CustomDilation = 1.0f;

	const UNeAbilityPreviewSettings* PreviewSettings = InHostEditor->GetPreviewSettings();
	ViewFOV = FMath::Clamp(PreviewSettings->FOV, 70.0f, 180.0f);

	// DrawHelper set up
	DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;
	DrawHelper.AxesLineThickness = 0.0f;
	DrawHelper.bDrawGrid = true;

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(0);

	SetRealtime(true);

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSeparateTranslucency(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetParticles(true);
#if false
	bool bAdvancedShowFlags = UAssetViewerSettings::Get()->Profiles[GetMutableDefault<UEditorPerProjectUserSettings>()->AssetViewerProfileIndex].bPostProcessingEnabled;
	if (bAdvancedShowFlags)
	{
		EngineShowFlags.EnableAdvancedFeatures();
	}
	else
	{
		EngineShowFlags.DisableAdvancedFeatures();
	}
#endif

	if (UWorld* PreviewWorld = InPreviewScene->GetWorld())
	{
		PreviewWorld->bAllowAudioPlayback = !PreviewSettings->bMuteAudio;
	}

}


FNeAbilityEditorViewportClient::~FNeAbilityEditorViewportClient(){}

void FNeAbilityEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);

	// TODO: 暂时先关掉下面逻辑，之后考虑通过扩展的方式实现
#if false
	// 将Spine组件注册到场景绘制体当中
	TSharedPtr<FGreatBattleSystemEditor> GBSEditor = HostEditor.IsValid() ? HostEditor.Pin() : NULL;
	if (GBSEditor.IsValid() && GUnrealEd && !GBSEditor->IsPlayingAbility())
	{
		UGBSAblAsset* TheAbility = GBSEditor->GetEditingAbility();
		if (TheAbility)
		{
			const TArray<UTask*>* TaskArray = TheAbility->GetTasks();
			for (int32 i = 0; i < TaskArray->Num(); ++i)
			{
				if ((*TaskArray)[i] != GBSEditor->GetCurrentlySelectedAbilityTask())
					continue;

				if (UTaskPlayNiagara* CurNiaTask = Cast<UTaskPlayNiagara>((*TaskArray)[i]))
				{
					for (TMap<FName, USplineComponent*>::TIterator It(CurNiaTask->SplineInstanceInEditor); It; ++It)
					{
						if (!It->Value && It->Value->GetOwner() && It->Value->GetOwner()->IsA<AActor>())
							continue;

						TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(USplineComponent::StaticClass());
						if (Visualizer.IsValid())
							Visualizer->DrawVisualization(It->Value, View, PDI);
					}
				}
			}
		}
	}
#endif
}

void FNeAbilityEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	CachedViewPort = InViewport;

	FNeSimpleEdViewportClient::Draw(InViewport, Canvas);
}

void FNeAbilityEditorViewportClient::Tick(float DeltaSeconds)
{
	TSharedPtr<FNeAbilityBlueprintEditor> AbilityEditor = HostEditor.IsValid() ? HostEditor.Pin() : nullptr;

	FNeSimpleEdViewportClient::Tick(DeltaSeconds);

	if (AbilityEditor.IsValid())
	{
		UNeAbilityPreviewSettings* PreviewSettings = HostEditor.Pin()->GetPreviewSettings();
		PreviewScene->GetWorld()->bAllowAudioPlayback = !PreviewSettings->bMuteAudio;

		if (!AbilityEditor->IsWorldPaused())
		{
			DeltaSeconds *= CustomDilation;
			TickWorld(DeltaSeconds);
		}
	}
}

void FNeAbilityEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	FNeSimpleEdViewportClient::ProcessClick(View, HitProxy, Key, Event, HitX, HitY);

	// TODO: check whether the logic is need or not
#if false
	if (HitProxy && HitProxy->IsA(HActor::StaticGetType()))
	{
		HActor* ActorHitProxy = (HActor*)HitProxy;
		AActor* ConsideredActor = ActorHitProxy->Actor;
		if (ConsideredActor)
		{
			if (HostEditor.IsValid())
			{
				HostEditor.Pin()->ChangeSelectedActor(ConsideredActor);
			}
		}
	}
#endif
}

bool FNeAbilityEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	if (GUnrealEd->ComponentVisManager.HandleInputDelta(this, InViewport, InDrag, InRot, InScale))
		return true;

	bool Result = FNeSimpleEdViewportClient::InputWidgetDelta(InViewport, InCurrentAxis, InDrag, InRot, InScale);

	// TODO: check whether the logic is need or not
#if false
	AActor* SelectedActor = GetSelectedActor();
	if (SelectedActor && InCurrentAxis != EAxisList::None)
		HostEditor.Pin()->ChangeSelectedActorTransformTemplate();
#endif

	return Result;
}

void FNeAbilityEditorViewportClient::CaptureThumbnail()
{
	if (HostEditor.IsValid())
	{
		m_CaptureThumbnail = true;
	}
}

void FNeAbilityEditorViewportClient::TickWorld(float DeltaSeconds)
{
	HandlerPreviewScenePreTick();
	PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	HostEditor.Pin()->Tick(DeltaSeconds);
	HandlerPreviewScenePostTick();
}

void FNeAbilityEditorViewportClient::HandlerPreviewScenePreTick()
{
}

void FNeAbilityEditorViewportClient::HandlerPreviewScenePostTick()
{
	if (ViewportCameraMode == NeAbilityEditorViewportCameraMode::RealGame)
	{
		APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		SetViewLocation(CameraManager->GetCameraLocation());
		SetViewRotation(CameraManager->GetCameraRotation());
		ViewFOV = CameraManager->GetFOVAngle();
		Invalidate();
	}
#if false
	else if (ViewportCameraMode == GBSViewportCameraMode::Bone)
	{
		SetViewLocation(ViewLockedSkel->GetBoneLocation(ViewLockedBoneName));
		SetViewRotation((ViewLockedSkel->GetBoneQuaternion(ViewLockedBoneName) * FQuat(FVector(0.0f, 1.0f, 0.0f), PI * 0.5f)).Rotator());
		Invalidate();
	}
#endif
}

void FNeAbilityEditorViewportClient::SetAblViewMode(const FName& InCameraMode)
{
	ViewportCameraMode = InCameraMode;
	ViewLockedSkel = nullptr;
	ViewLockedBoneName = NAME_None;

	// Reset Rotation
	FRotator RawRot = GetViewRotation();
	RawRot.Roll = 0.f;
	SetViewRotation(RawRot);
	EnableCameraLock(false);

	if (InCameraMode == NeAbilityEditorViewportCameraMode::Free)
	{
		SetLockedActor(nullptr);
	}
	else if (InCameraMode == NeAbilityEditorViewportCameraMode::RealGame)
	{
		SetLockedActor(nullptr);
	}
#if false
	else if (InCameraMode == GBSViewportCameraMode::Bone)
	{
		SetLockedActor(nullptr);
		SetLockedMatineeAnim(nullptr);
	}
	else if (InCameraMode == GBSViewportCameraMode::CameraAnimLock)
	{
		SetLockedActor(HostEditor.Pin()->GetSequencePreviewer()->GetTemplateSequencePreviewActor());
		SetLockedMatineeAnim(nullptr);
	}
#endif

	Invalidate();
}

void FNeAbilityEditorViewportClient::SetViewLockedBone(USkeletalMeshComponent* InSkeMeshComp, const FName& InBoneName)
{
	ViewLockedSkel = InSkeMeshComp;
	ViewLockedBoneName = InBoneName;
}

FName FNeAbilityEditorViewportClient::GetCameraLockedBone() const
{
	return ViewLockedBoneName;
}

UActorComponent* FNeAbilityEditorViewportClient::FindViewComponentForActor(AActor const* Actor)
{
	UActorComponent* PreviewComponent = nullptr;
	if (Actor)
	{
		const TSet<UActorComponent*>& Comps = Actor->GetComponents();

		for (UActorComponent* Comp : Comps)
		{
			FMinimalViewInfo DummyViewInfo;
			if (Comp && Comp->IsActive() && Comp->GetEditorPreviewInfo(/*DeltaTime =*/0.0f, DummyViewInfo))
			{
				if (Comp->IsSelected())
				{
					PreviewComponent = Comp;
					break;
				}
				else if (PreviewComponent)
				{
					UCameraComponent* AsCamComp = Cast<UCameraComponent>(Comp);
					if (AsCamComp != nullptr)
					{
						PreviewComponent = AsCamComp;
					}
					continue;
				}
				PreviewComponent = Comp;
			}
		}

	}
	return PreviewComponent;
}

//FSceneView* FNeAbilityEditorViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const int32 StereoViewIndex)
//{
//	FSceneView* View = FNeSimpleEdViewportClient::CalcSceneView(ViewFamily, StereoViewIndex);
//	return View;
//}

void FNeAbilityEditorViewportClient::OverridePostProcessSettings( FSceneView& View )
{
	FNeSimpleEdViewportClient::OverridePostProcessSettings(View);
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	// CameraAnim override
	if (CameraManager)
	{
		TArray<FPostProcessSettings> const* CameraAnimPPSettings;
		TArray<float> const* CameraAnimPPBlendWeights;
		CameraManager->GetCachedPostProcessBlends(CameraAnimPPSettings, CameraAnimPPBlendWeights);

		for (int32 PPIdx = 0; PPIdx < CameraAnimPPBlendWeights->Num(); ++PPIdx)
		{
			View.OverridePostProcessSettings((*CameraAnimPPSettings)[PPIdx], (*CameraAnimPPBlendWeights)[PPIdx]);
		}
	}
}

void FNeAbilityEditorViewportClient::SetPlaybackSpeedMode(ENeAbilityPlaybackSpeeds::Type InMode)
{
	PlaybackSpeedMode = InMode;
	CustomDilation = FMath::Clamp(ENeAbilityPlaybackSpeeds::Values[PlaybackSpeedMode], 0.0001, 20.0);
}

ENeAbilityPlaybackSpeeds::Type FNeAbilityEditorViewportClient::GetPlaybackSpeedMode() const
{
	return PlaybackSpeedMode;
}