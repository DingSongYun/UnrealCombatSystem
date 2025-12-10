// Copyright NetEase Games, Inc. All Rights Reserved.

#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "PreviewScene.h"
#include "SceneView.h"
#include "UnrealEdGlobals.h"
#include "Camera/CameraComponent.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/Selection.h"
#include "Viewport/NeSimpleEdViewportClient.h"

FNeSimpleEdViewportClient::FNeSimpleEdViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget)
	: FEditorViewportClient(InModeTools, InPreviewScene, InEditorViewportWidget)
	, FadeAmount(0.f)
	, bEnableFading(false)
{
	// DrawHelper set up
	DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;
	DrawHelper.AxesLineThickness = 0.0f;
	DrawHelper.bDrawGrid = true;

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(0);
	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSeparateTranslucency(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetParticles(true);
	EngineShowFlags.ModeWidgets = true;

	SetRealtime(true);
	SetShowStats(true);

	UWorld* PreviewWorld = InPreviewScene->GetWorld();
	PreviewWorld->bAllowAudioPlayback = true;

	ModelTools = InModeTools;

	GEngine->OnActorMoved().AddRaw(this, &FNeSimpleEdViewportClient::OnActorMoved);
}

void FNeSimpleEdViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
	MoveViewportToLockedActor();
}

void FNeSimpleEdViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
}

void FNeSimpleEdViewportClient::SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View)
{
	FEditorViewportClient::SetupViewForRendering( ViewFamily, View );
	if (!ViewFamily.EngineShowFlags.LightComplexity)
	{
		if (bEnableFading)
		{
			View.OverlayColor = FadeColor;
			View.OverlayColor.A = FMath::Clamp(FadeAmount, 0.f, 1.f);
		}
	}
}

FSceneView* FNeSimpleEdViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const int32 StereoViewIndex)
//FSceneView* FNeSimpleEdViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass)
{
	return FEditorViewportClient::CalcSceneView(ViewFamily, StereoViewIndex);
}

void FNeSimpleEdViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);

	if (HitProxy && HitProxy->IsA(HActor::StaticGetType()))
	{
		HActor* ActorHitProxy = (HActor*)HitProxy;
		AActor* ConsideredActor = ActorHitProxy->Actor;
		if (ConsideredActor)
		{
			while (ConsideredActor->IsChildActor())
			{
				ConsideredActor = ConsideredActor->GetParentActor();
			}

			GEditor->GetSelectedActors()->Modify();
			GEditor->SelectNone(false, true, false);
			GEditor->SelectActor(ConsideredActor, true, true, true);
		}
	}

	GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click);
}

FVector FNeSimpleEdViewportClient::GetWidgetLocation() const
{
	//FVector ComponentVisWidgetLocation;
	//if (GUnrealEd->ComponentVisManager.IsVisualizingArchetype()
	//	&& GUnrealEd->ComponentVisManager.GetWidgetLocation(this, ComponentVisWidgetLocation))
	//{
	//	return ComponentVisWidgetLocation;
	//}

	//// 获得Actor的位置
	//AActor* SelectedActor = GetSelectedActor();

	//if (SelectedActor)
	//{
	//	return SelectedActor->GetActorLocation();
	//}

	//return FVector::ZeroVector;
	return ModeTools->GetWidgetLocation();
}

FMatrix FNeSimpleEdViewportClient::GetWidgetCoordSystem() const
{
	return ModeTools->GetCustomInputCoordinateSystem();
}

void FNeSimpleEdViewportClient::SetWidgetCoordSystemSpace(ECoordSystem NewCoordSystem)
{
	ModeTools->SetCoordSystem(NewCoordSystem);
	RedrawAllViewportsIntoThisScene();
}

ECoordSystem FNeSimpleEdViewportClient::GetWidgetCoordSystemSpace() const
{
	return ModeTools->GetCoordSystem();
}

bool FNeSimpleEdViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	if (GUnrealEd->ComponentVisManager.HandleInputDelta(this, InViewport, InDrag, InRot, InScale))
	{
		return true;
	}

	bool bHandled = false;

	AActor* SelectedActor = GetSelectedActor();

	if (SelectedActor && InCurrentAxis != EAxisList::None)
	{
		GEditor->ApplyDeltaToActor(SelectedActor, true,
			&InDrag, &InRot, &InScale,
			false, false, false);

		if (!InRot.IsZero())
		{
			FRotator TranslateRotateWidgetRotation(0, ModeTools->TranslateRotateXAxisAngle, 0);
			TranslateRotateWidgetRotation += InRot;
			ModeTools->TranslateRotateXAxisAngle = TranslateRotateWidgetRotation.Yaw;

			FRotator Widget2DRotation(ModeTools->TranslateRotate2DAngle, 0, 0);
			Widget2DRotation += InRot;
			ModeTools->TranslateRotate2DAngle = Widget2DRotation.Pitch;
		}

		ModeTools->PivotLocation += InDrag;
		ModeTools->SnappedLocation += InDrag;

		bHandled = true;
	}

	return bHandled;
}

void FNeSimpleEdViewportClient::PerspectiveCameraMoved()
{
	FEditorViewportClient::PerspectiveCameraMoved();
	MoveLockedActorToViewport();
	MoveViewportToLockedActor();
}

void FNeSimpleEdViewportClient::TrackingStopped()
{
	TArray<USceneComponent*> ComponentsToMove;
	TArray<AActor*> ActorsToMove;
	GetSelectedActorsAndComponentsForMove(ActorsToMove, ComponentsToMove);

	for (USceneComponent* Component : ComponentsToMove)
	{
		//FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(Component, PropertyChangedEvent);
		
		Component->PostEditComponentMove(true);
		GEditor->BroadcastEndObjectMovement(*Component);
	}

	for (AActor* Actor : ActorsToMove)
	{
		//FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(Actor, PropertyChangedEvent);
		Actor->PostEditMove(true);
		GEditor->BroadcastEndObjectMovement(*Actor);
	}

	GEditor->BroadcastActorsMoved(ActorsToMove);
}

void FNeSimpleEdViewportClient::OnActorLockToggleFromMenu(AActor* InActor)
{
	LockActorInternal(InActor);
}

bool FNeSimpleEdViewportClient::IsActorLocked(const TWeakObjectPtr<AActor> TestActor) const
{
	return TestActor.IsValid() && TestActor == GetLockedActor();
}

AActor* FNeSimpleEdViewportClient::GetLockedActor() const
{
	return ActorLocked.Get();
}

void FNeSimpleEdViewportClient::LockActorInternal(AActor* InActor)
{
	ActorLocked = InActor;
	MoveViewportToLockedActor();
}

void FNeSimpleEdViewportClient::MoveViewportToLockedActor()
{
	if( AActor* LockedActor = GetLockedActor() )
	{
		SetViewLocation( LockedActor->GetActorLocation() );
		SetViewRotation( LockedActor->GetActorRotation() );
		if (UCameraComponent* CameraComp = Cast<UCameraComponent>(LockedActor->GetComponentByClass(UCameraComponent::StaticClass())))
		{
			ViewFOV = CameraComp->FieldOfView;
		}
		Invalidate();
	}
}

void FNeSimpleEdViewportClient::MoveLockedActorToViewport()
{
	if (AActor* LockedActor = GetLockedActor())
	{
		if (!LockedActor->IsLockLocation())
		{
			LockedActor->SetActorLocation(GetViewLocation(), false);
			LockedActor->SetActorRotation(GetViewRotation());

			USceneComponent* LockedActorComponent = LockedActor->GetRootComponent();
			TOptional<FRotator> PreviousRotator;
			if (LockedActorComponent)
			{
				PreviousRotator = LockedActorComponent->GetRelativeRotation();
			}

			if (LockedActorComponent)
			{
				const FRotator Rot = PreviousRotator.GetValue();
				FRotator ActorRotWind, ActorRotRem;
				Rot.GetWindingAndRemainder(ActorRotWind, ActorRotRem);
				const FQuat ActorQ = ActorRotRem.Quaternion();
				const FQuat ResultQ = LockedActorComponent->GetRelativeRotation().Quaternion();
				FRotator NewActorRotRem = FRotator(ResultQ);
				ActorRotRem.SetClosestToMe(NewActorRotRem);
				FRotator DeltaRot = NewActorRotRem - ActorRotRem;
				DeltaRot.Normalize();
				LockedActorComponent->SetRelativeRotationExact(Rot + DeltaRot);
			}
		}
	}
}

void FNeSimpleEdViewportClient::OnActorMoved(AActor* InActor)
{
	if (IsActorLocked(InActor))
	{
		MoveViewportToLockedActor();
	}
}

AActor* FNeSimpleEdViewportClient::GetSelectedActor() const
{
	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection)
	{
		TArray<AActor*> SelectedActors;
		int32 Num = Selection->GetSelectedObjects(SelectedActors);
		if (Num > 0)
		{
			for (AActor* Actor : SelectedActors)
			{
				if (Actor->GetWorld() == GetWorld())
				{
					return Actor;
				}
			}
		}
	}

	return nullptr;
}

void FNeSimpleEdViewportClient::GetSelectedActorsAndComponentsForMove(TArray<AActor*>& OutActorsToMove, TArray<USceneComponent*>& OutComponentsToMove) const
{
	OutActorsToMove.Reset();
	OutComponentsToMove.Reset();

	// Get the list of parent-most component(s) that are selected
	if (GEditor->GetSelectedComponentCount() > 0)
	{
		// Otherwise, if both a parent and child are selected and the delta is applied to both, the child will actually move 2x delta
		for (FSelectedEditableComponentIterator EditableComponentIt(GEditor->GetSelectedEditableComponentIterator()); EditableComponentIt; ++EditableComponentIt)
		{
			USceneComponent* SceneComponent = Cast<USceneComponent>(*EditableComponentIt);
			if (!SceneComponent)
			{
				continue;
			}

			// Check to see if any parent is selected
			bool bParentAlsoSelected = false;
			USceneComponent* Parent = SceneComponent->GetAttachParent();
			while (Parent != nullptr)
			{
				if (Parent->IsSelected())
				{
					bParentAlsoSelected = true;
					break;
				}

				Parent = Parent->GetAttachParent();
			}

			AActor* ComponentOwner = SceneComponent->GetOwner();
			if (!CanMoveActorInViewport(ComponentOwner))
			{
				continue;
			}

			const bool bIsRootComponent = (ComponentOwner && (ComponentOwner->GetRootComponent() == SceneComponent));
			if (bIsRootComponent)
			{
				// If it is a root component, use the parent actor instead
				OutActorsToMove.Add(ComponentOwner);
			}
			else if (!bParentAlsoSelected)
			{
				// If no parent of this component is also in the selection set, move it
				OutComponentsToMove.Add(SceneComponent);
			}
		}
	}

	// Skip gathering selected actors if we had a valid component selection
	if (OutComponentsToMove.Num() || OutActorsToMove.Num())
	{
		return;
	}
	
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = CastChecked<AActor>(*It);

		// If the root component was selected, this actor is already accounted for
		USceneComponent* RootComponent = Actor->GetRootComponent();
		if (RootComponent && RootComponent->IsSelected())
		{
			continue;
		}

		if (!CanMoveActorInViewport(Actor))
		{
			continue;
		}

		OutActorsToMove.Add(Actor);
	}
}

bool FNeSimpleEdViewportClient::CanMoveActorInViewport(const AActor* InActor) const
{
	if (!GEditor || !InActor)
	{
		return false;
	}

	// The actor cannot be location locked
	if (InActor->IsLockLocation())
	{
		return false;
	}

	// The actor needs to be in the current viewport world
	if (GEditor->PlayWorld)
	{
		if (bIsSimulateInEditorViewport)
		{
			// If the Actor's outer (level) outer (world) is not the PlayWorld then it cannot be moved in this viewport.
			if (!(GEditor->PlayWorld == InActor->GetOuter()->GetOuter()))
			{
				return false;
			}
		}
		else if (!(GEditor->EditorWorld == InActor->GetOuter()->GetOuter()))
		{
			return false;
		}
	}

	return true;
}

