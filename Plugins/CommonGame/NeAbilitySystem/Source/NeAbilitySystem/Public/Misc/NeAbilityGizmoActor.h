// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityWeakPtr.h"
#include "Misc/Attribute.h"
#include "GameFramework/Actor.h"
#include "NeAbilityGizmoActor.generated.h"

struct FGizmoArgs
{
	TAttribute<FNeAbilitySegmentEvalContext*> EvalContext;

	TAttribute<AActor*> PreviewPlayer;

	TAttribute<AActor*> PreviewTarget;
};

/**
 * ANeAbilityGizmoActor
 * TaskGizmo 用来提供一些Task需要基于场景空间的属性编辑
 */
UCLASS(Abstract, HideCategories=(Replication, Collision, Actor, Input, HLOD, Physics, WorldPartition, Event, Cooking, DataLayers))
class NEABILITYSYSTEM_API ANeAbilityGizmoActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	/** Initialize */
	virtual void InitializeFor(const FWeakAbilitySegmentPtr& InSegmentPtr, const FGizmoArgs& Args);

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	/** Call after gizmo actor created */
	virtual void PostGizmoCreated() {}

	/** Refresh gizmo actor from task infos */
	UFUNCTION()
	void SynchronizeFromBinding();

	/** Call when request to synchronize */
	virtual void OnSynchronizeFromBinding() {}

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnSynchronizeFromBinding"))
	void ReceiveSynchronizeFromBinding();

	/** When move gizmo actor on editor viewport */
	UFUNCTION()
	void NotifyGizmoMoved();

	/** Call when gizmo moved */
	virtual void OnGizmoMoved() {}

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="NotifyGizmoMoved"))
	void ReceiveOnGizmoMoved();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="InitializeFor"))
	void ReceiveInitializeFor(const FNeAbilitySegment& InSegment);

	const FWeakAbilitySegmentPtr& GetBindingSegment() const { return BindingSegment; }

	UFUNCTION(BlueprintCallable)
	const FNeAbilitySegmentEvalContext& GetEvalContext() const;

	UFUNCTION(BlueprintCallable)
	AActor* GetPreviewPlayer() const;

	UFUNCTION(BlueprintCallable)
	AActor* GetPreviewTarget() const;

	/** Trigger task owner changed property */
	void PostTaskChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent);

	virtual void OnTransformUpdated(USceneComponent* InRootComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);

protected:
	UPROPERTY(EditDefaultsOnly)
	uint8 bSyncOnTick : 1;

	uint8 bLockGizmoMove : 1;
	uint8 bLockGizmoSync : 1;

	FWeakAbilitySegmentPtr BindingSegment;

	/** TODO: FName还是不太好做关联，之后替换成一种PropertyHandle的方式，让属性可以选 */
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> ObservedProperties;

	TAttribute<FNeAbilitySegmentEvalContext*> EvalContext;

	TAttribute<AActor*> PreviewPlayer;

	TAttribute<AActor*> PreviewTarget;
};

/**
 * ANeAbilityTransformGizmo
 * 通用Transform的Gizmo
 */
UCLASS(Abstract)
class NEABILITYSYSTEM_API ANeAbilityTransformGizmo : public ANeAbilityGizmoActor
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITORONLY_DATA
	/** Billboard used to see the trigger in the editor */
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBillboardComponent> SpriteComponent;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif

};