// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"

class NEEDITORFRAMEWORK_API FNeSimpleEdViewportClient : public FEditorViewportClient
{
public:
	FNeSimpleEdViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene = nullptr, const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr);

	//~BEGIN: FEditorViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual void SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View) override;
	//virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass = eSSP_FULL) override;
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const int32 StereoViewIndex = INDEX_NONE) override;
	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	virtual FVector GetWidgetLocation() const override;
	virtual FMatrix GetWidgetCoordSystem() const override;
	virtual UE::Widget::EWidgetMode GetWidgetMode() const override { return WidgetMode; }
	virtual void SetWidgetMode(UE::Widget::EWidgetMode NewMode) override { WidgetMode = NewMode; }
	//virtual void SetWidgetCoordSystemSpace(ECoordSystem NewCoordSystem) override { WidgetCoordSystem = NewCoordSystem; }
	virtual void SetWidgetCoordSystemSpace(ECoordSystem NewCoordSystem) override;
	//virtual ECoordSystem GetWidgetCoordSystemSpace() const override { return WidgetCoordSystem; }
	virtual ECoordSystem GetWidgetCoordSystemSpace() const override;
	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;
	virtual void PerspectiveCameraMoved() override;
	virtual void TrackingStopped() override;
	//~END: FEditorViewportClient interface

	void GetSelectedActorsAndComponentsForMove(TArray<AActor*>& OutActorsToMove, TArray<USceneComponent*>& OutComponentsToMove) const;
	bool CanMoveActorInViewport(const AActor* InActor) const;

	////////////////////////////////////////////////////////////////
	// Actor Lock
public:
	void SetLockedActor(AActor* InActor) { LockActorInternal(InActor); }
	void OnActorLockToggleFromMenu(AActor* InActor);
	bool IsActorLocked(const TWeakObjectPtr<AActor> TestActor) const;
	bool HasAnyActorLocked() const { return ActorLocked.IsValid(); }
	AActor* GetLockedActor() const;
private:
	void LockActorInternal(AActor* InActor);
	void MoveViewportToLockedActor();
	void MoveLockedActorToViewport();
	void OnActorMoved(AActor* InActor);

protected:
	AActor* GetSelectedActor() const;

	FEditorModeTools* ModelTools;

public:
	FColor					FadeColor;
	float					FadeAmount;
	bool					bEnableFading;

private:
	UE::Widget::EWidgetMode WidgetMode = UE::Widget::EWidgetMode::WM_Translate;
	ECoordSystem WidgetCoordSystem = ECoordSystem::COORD_World;

	TWeakObjectPtr<AActor> ActorLocked;
};

