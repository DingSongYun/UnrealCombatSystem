// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityPreviewScene.h"

#include "EngineUtils.h"
#include "GameMapsSettings.h"
#include "NeAbility.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityEditorDelegates.h"
#include "NeAbilityEditorPlayer.h"
#include "NeAbilitySystemSettings.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Misc/NeAbilityGizmoActor.h"
#include "GameMode/NeGenericGameModeEditorPreview.h"
#include "Slate/SceneViewport.h"

FNeAbilityPreviewScene::FNeAbilityPreviewScene(ConstructionValues CVS, TSharedPtr<FNeAbilityBlueprintEditor> InHostEditor)
	: FAdvancedPreviewScene(CVS.SetDefaultGameMode(AGenericGameModeEditorPreview::StaticClass())), MyAbilityEditor(InHostEditor)
{
	FNeAbilityEditorDelegates::AddNewSegmentDelegate.AddRaw(this, &FNeAbilityPreviewScene::OnAddNewSegment);
	FNeAbilityEditorDelegates::PreDeleteSegmentDelegate.AddRaw(this, &FNeAbilityPreviewScene::OnDeleteSegment);
	FNeAbilityEditorDelegates::SegmentSelectionChangedDelegate.AddRaw(this, &FNeAbilityPreviewScene::OnSegmentSelectionChanged);
}

FNeAbilityPreviewScene::~FNeAbilityPreviewScene()
{
	FNeAbilityEditorDelegates::AddNewSegmentDelegate.RemoveAll(this);
	FNeAbilityEditorDelegates::PreDeleteSegmentDelegate.RemoveAll(this);
	FNeAbilityEditorDelegates::SegmentSelectionChangedDelegate.RemoveAll(this);
}

void FNeAbilityPreviewScene::Tick(float DeltaTime)
{
	FAdvancedPreviewScene::Tick(DeltaTime);
	if (bInvalidateViewport)
	{
		TSharedPtr<FNeAbilityEditorViewportClient> ViewportClient = MyAbilityEditor.Pin()->GetPreviewViewportClient();
		if (ViewportClient)
		{
			GEditor->RedrawAllViewports();
			//ViewportClient->Invalidate();
		}

		bInvalidateViewport = false;
	}

	// TODO: Tick Gizmo
}

void FNeAbilityPreviewScene::AddReferencedObjects(FReferenceCollector& Collector)
{
	FAdvancedPreviewScene::AddReferencedObjects(Collector);
	if (const auto PreviewPlayer = MyAbilityEditor.Pin()->GetAbilityPreviewPlayer(); PreviewPlayer.IsValid())
	{
		PreviewPlayer->AddReferencedObjects(Collector);
	}
}

void FNeAbilityPreviewScene::OnCreateViewport(SNeAbilityEditorViewport* InEditorViewport, TSharedPtr<class FSceneViewport> InSceneViewport) const
{
	if (GetWorld()->GetGameViewport() != nullptr)
	{
		GameViewportClient->SetViewportFrame(InSceneViewport.Get());
	}
}

void FNeAbilityPreviewScene::InitPreviewWorld()
{
	//TODO: 定制预览场景

	GetWorld()->bForceUseMovementComponentInNonGameWorld = true;

	UNeAbility* Ability = MyAbilityEditor.Pin()->GetEditingAbility();

	// Create Controller if need
	SpawnControllers();

	// Create preview actors
	SpawnPreviewActors(Ability);

	// Create task Gizmo Actor
	const int32 SectionNum = Ability->GetSectionNums();
	for( int32 SectionIndex = 0; SectionIndex < SectionNum; ++ SectionIndex)
	{
		FNeAbilitySection& Section = Ability->GetSection(SectionIndex);
		for (const FNeAbilitySegment& Segment : Section.Segments)
		{
			if (Segment.ShouldCreateGizmo())
			{
				SpawnGizmoForSegment(SectionIndex, Segment.GetID());
			}
		}
	}

	// Dispatch Begin play
	UWorld* World = GetWorld();
	if (!World->HasBegunPlay())
	{
		for (FActorIterator It(World); It; ++It)
		{
			const bool bFromLevelLoad = true;
			It->DispatchBeginPlay(bFromLevelLoad);
		}

		World->bBegunPlay = true;
	}
}

void FNeAbilityPreviewScene::ResetPreviewWorld()
{
	UWorld* World = GetWorld();
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!InherentActorList.Contains(Actor))
		{
			World->DestroyActor(Actor);
		}
	}

	if (PlayerController.IsValid()) World->DestroyActor(PlayerController.Get());
	DestroyPreviewActors();
	GetWorld()->bAllowAudioPlayback = false;
	SpawnControllers();
	SpawnPreviewActors(MyAbilityEditor.Pin()->GetEditingAbility());

	// 取消Gizmo的选取
	ClearGizmoSelection();
}

void FNeAbilityPreviewScene::PostInitPreviewWorld()
{
	InherentActorList.Empty();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		InherentActorList.Add(*It);
	}
}

void FNeAbilityPreviewScene::SpawnControllers()
{
	UWorld* World = GetWorld();

	const UNeAbilitySystemSettings* AbilitySystemSettings = GetDefault<UNeAbilitySystemSettings>();
	const UGameMapsSettings* GameMapSettings = GetDefault<UGameMapsSettings>();

	FSoftClassPath GameModeClassPath = GameMapSettings->GetGlobalDefaultGameMode();
	const UClass* GameModeClass = GameModeClassPath.TryLoadClass<AGameModeBase>();
	AGameModeBase* DefaultGameMode = GameModeClass ? Cast<AGameModeBase>(GameModeClass->GetDefaultObject()) : nullptr;

	// Create Preview PlayerController
	UClass* PlayerControllerClass = AbilitySystemSettings->PlayerControllerClass.LoadSynchronous()
		                                ? AbilitySystemSettings->PlayerControllerClass.Get()
		                                : (DefaultGameMode ? DefaultGameMode->PlayerControllerClass.Get() : nullptr);
	if (PlayerControllerClass == nullptr) PlayerControllerClass = APlayerController::StaticClass();

	PlayerController = World->SpawnActorDeferred<APlayerController>(PlayerControllerClass, FTransform::Identity);
	if (!AbilitySystemSettings->CameraManagerClass.IsNull())
	{
		PlayerController->PlayerCameraManagerClass = AbilitySystemSettings->CameraManagerClass.LoadSynchronous();
	}
	PlayerController->bAutoManageActiveCameraTarget = false;
	PlayerController->FinishSpawning(FTransform::Identity);

	// TODO: Check if need dispatch beginplay
	// PlayerController->PlayerCameraManager->DispatchBeginPlay();
	PlayerController->PlayerCameraManager->bDebugClientSideCamera = true;

	// AWSCameraManager* WSCameraManagerBase = Cast<AWSCameraManager>(PlayerController->PlayerCameraManager);
	// if (WSCameraManagerBase)
	// {
	// 	WSCameraManagerBase->HideCameraMesh(true);
	// }
}

void FNeAbilityPreviewScene::SpawnPreviewActors(const UNeAbility* Ability)
{
	check(Ability);
	UWorld* World = GetWorld();
	PlayerController->UnPossess();

	const FNeAbilityPreviewActors& PreviewActorInfos = Ability->AbilityPreviewActors;
	check(PreviewActors.IsEmpty() && TargetActors.IsEmpty());
	UNeAbilityPreviewActorType* PreviewPlayerInfo = IsValid(PreviewActorInfos.PlayerInfo)
		                                          ? PreviewActorInfos.PlayerInfo
		                                          : GetMutableDefault<UNeAbilityPreviewActorCommon>();
	if (PreviewPlayerInfo)
	{
		PlayerActor = PreviewPlayerInfo->CreateActor(World, PreviewActorInfos.PlayerTransform);
		if (PlayerActor.IsValid())
		{
			//PreviewPlayer->FinishSpawning(FTransform::Identity);
			// PreviewPlayer->DispatchBeginPlay();
			PreviewActors.Add(PlayerActor);
			PlayerController->Possess(Cast<APawn>(PlayerActor.Get()));
			PreviewPlayerInfo->PostCreateActor(PlayerActor.Get());
		}

		OnCreatePlayerDelegate.Broadcast(PlayerActor.Get());
	}

	if (PreviewActorInfos.TargetInfo)
	{
		AActor* TargetActor = PreviewActorInfos.TargetInfo->CreateActor(World, PreviewActorInfos.TargetTransform);
		if (TargetActor != nullptr)
		{
			// TargetActor->DispatchBeginPlay();
			PreviewActors.Add(TargetActor);
			TargetActors.Add(TargetActor);
			PreviewActorInfos.TargetInfo->PostCreateActor(TargetActor);
		}
		OnCreateTargetsDelegate.Broadcast(TargetActors);
	}
}

void FNeAbilityPreviewScene::DestroyPreviewActors()
{
	if (PlayerController.IsValid()) PlayerController->UnPossess();
	UWorld* World = GetWorld();
	for (TWeakObjectPtr<AActor> ActorPtr : PreviewActors)
	{
		if (ActorPtr.IsValid())
		{
			TArray<class AActor*> AttachedActors;
			ActorPtr->GetAttachedActors(AttachedActors, true);
			for (AActor* AttachedActor : AttachedActors)
			{
				World->EditorDestroyActor(AttachedActor, false);
			}

			World->EditorDestroyActor(ActorPtr.Get(), false);
		}
	}
	PlayerActor = nullptr;
	PreviewActors.Empty();
	TargetActors.Empty();
}

void FNeAbilityPreviewScene::SpawnGizmoForSegment(int32 SectionIndex, int32 SegmentID)
{
	UNeAbility* Ability = MyAbilityEditor.Pin()->GetEditingAbility();
	const FNeAbilitySectionPtr SectionPtr = MakeWeakSectionPtr(Ability, SectionIndex);
	FNeAbilitySegmentPtr SegmentPtr = MakeWeakSegmentPtr(SectionPtr, SegmentID);
	if (!SegmentPtr.IsValid() || !SegmentPtr->ShouldCreateGizmo()) return;
	ANeAbilityGizmoActor* GizmoActor = SpawnGizmoForSegment(SegmentPtr.Get());
	if (GizmoActor)
	{
		FGizmoArgs GizmoArgs;
		{
			GizmoArgs.PreviewPlayer.BindRaw(this, &FNeAbilityPreviewScene::GetPlayerActor);
			GizmoArgs.PreviewTarget.BindRaw(this, &FNeAbilityPreviewScene::GetPlayerActor);
			GizmoArgs.EvalContext = MakeAttributeLambda(
				[SegmentPtr, this]()
				{
					FNeAbilitySegmentEvalContext* EvalContext = nullptr;
					TSharedPtr<FNeAbilityEditorPlayerBase> PreviewPlayer = MyAbilityEditor.IsValid() ? MyAbilityEditor.Pin()->GetAbilityPreviewPlayer() : nullptr;
					if (PreviewPlayer)
					{
						EvalContext = PreviewPlayer->GetPreviewSegmentEvalContext(SegmentPtr);
					}
					return EvalContext;
				});
		}
		GizmoActor->InitializeFor(SegmentPtr, GizmoArgs);
		GizmoActor->PostGizmoCreated();
		SetActorVisibilityInViewport(GizmoActor, false);
		
		// 将Actor加入InherentActorList
		GizmoActors.Add(GizmoActor);
		InherentActorList.AddUnique(GizmoActor);
	}
}

ANeAbilityGizmoActor* FNeAbilityPreviewScene::SpawnGizmoForSegment(const FNeAbilitySegment& Segment)
{
	UClass* GizmoType = Segment.GetGizmoActorType();
	if (GizmoType)
	{
		UWorld* World = GetWorld();
		ANeAbilityGizmoActor* GizmoActor = World->SpawnActor<ANeAbilityGizmoActor>(GizmoType);
		return GizmoActor;
	}

	return nullptr;
}

void FNeAbilityPreviewScene::SetActorVisibilityInViewport(AActor* InActor, bool bShow)
{
	if (InActor == nullptr) return;
	InActor->SetIsTemporarilyHiddenInEditor(!bShow);

	TArray<class AActor*> OutActors;
	InActor->GetAttachedActors(OutActors);
	for (auto Actor : OutActors)
	{
		Actor->SetIsTemporarilyHiddenInEditor(!bShow);
	}
	bInvalidateViewport = true;
}

void FNeAbilityPreviewScene::OnAddNewSegment(const FWeakAbilitySegmentPtr& SegmentPtr)
{
	if (SegmentPtr->ShouldCreateGizmo())
	{
		SpawnGizmoForSegment(SegmentPtr.GetSectionIndex(), SegmentPtr.GetSegmentID());
	}
}

void FNeAbilityPreviewScene::OnDeleteSegment(const FWeakAbilitySegmentPtr& SegmentPtr)
{
	check(SegmentPtr.IsValid());
	if (ANeAbilityGizmoActor* GizmoActor = FindGizmoActor(SegmentPtr))
	{
		GizmoActors.Remove(GizmoActor);
		InherentActorList.Remove(GizmoActor);
	}
}

void FNeAbilityPreviewScene::OnSegmentSelectionChanged(const FWeakAbilitySegmentPtr& SegmentPtr)
{
	if (!SegmentPtr.IsValid())
	{
		ClearGizmoSelection();
		return ;
	}

	ANeAbilityGizmoActor* NewSelectedGizmo = FindGizmoActor(SegmentPtr);
	if (NewSelectedGizmo == nullptr)
	{
		ClearGizmoSelection();
		return ;
	}

	if (GizmoSelecting != NewSelectedGizmo)
	{
		// Clear old gizmo
		ClearGizmoSelection();

		GizmoSelecting = NewSelectedGizmo;

		// Select new gizmo
		GizmoSelecting->SynchronizeFromBinding();
		GEditor->SelectActor(GizmoSelecting.Get(), true, true, true);
		SetActorVisibilityInViewport(GizmoSelecting.Get(), true);
	}
}

ANeAbilityGizmoActor* FNeAbilityPreviewScene::FindGizmoActor(const FWeakAbilitySegmentPtr& SegmentPtr)
{
	if (!SegmentPtr.IsValid()) return nullptr;

	for (const TWeakObjectPtr<ANeAbilityGizmoActor>& GizmoActor : GizmoActors)
	{
		if (GizmoActor.IsValid() && GizmoActor->GetBindingSegment() == SegmentPtr)
		{
			return GizmoActor.Get();
		}
	}
	return nullptr;
}

void FNeAbilityPreviewScene::ClearGizmoSelection()
{
	if (GizmoSelecting.IsValid())
	{
		SetActorVisibilityInViewport(GizmoSelecting.Get(), false);
	}
	GEditor->SelectNone(true, true);
	GizmoSelecting = nullptr;
}
