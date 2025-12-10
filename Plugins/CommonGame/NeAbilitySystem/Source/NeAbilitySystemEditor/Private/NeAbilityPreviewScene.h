// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "AdvancedPreviewScene.h"
#include "NeAbilityWeakPtr.h"
#include "Widgets/SNeAbilityEditorViewport.h"

struct FNeAbilitySegment;
class UNeAbility;
class ULevelStreaming;
class UAssetViewerSettings;
class FNeAbilityBlueprintEditor;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCreatePlayerDelegate, AActor* /** Player*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCreateTargetsDelegate, TArray<TWeakObjectPtr<AActor>> /** Targets */);

/**
 * FNeAbilityPreviewScene
 * 预览环境场景管理
 */
class FNeAbilityPreviewScene final : public FAdvancedPreviewScene
{
public:
	FNeAbilityPreviewScene(ConstructionValues CVS, TSharedPtr<FNeAbilityBlueprintEditor> InHostEditor);
	virtual ~FNeAbilityPreviewScene();

	//~BEGIN: FPreviewScene interface
	virtual void Tick(float DeltaTime) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~END: FPreviewScene interface

	void OnCreateViewport(SNeAbilityEditorViewport* InEditorViewport, TSharedPtr<class FSceneViewport> InSceneViewport) const;

	void InitPreviewWorld();
	void ResetPreviewWorld();
	void PostInitPreviewWorld();

	void SpawnControllers();
	void SpawnPreviewActors(const UNeAbility* Ability);
	void DestroyPreviewActors();

	/** Spawn gizmo actor for segment */
	void SpawnGizmoForSegment(int32 SectionIndex, int32 SegmentID);
	class ANeAbilityGizmoActor* SpawnGizmoForSegment(const FNeAbilitySegment& Segment);

	/** 非PIE模式下使 Actor在场景内不可见 */
	void SetActorVisibilityInViewport(AActor* InActor, bool bShow);

	/** 预览环境中的角色 */
	FORCEINLINE AActor* GetPlayerActor() const { return PlayerActor.Get(); }

	/** 预览环境中的目标角色 */
	FORCEINLINE AActor* GetTargetActor() const { return TargetActors[0].Get(); }

	/** 预览环境中的目标角色 */
	FORCEINLINE TArray<TWeakObjectPtr<AActor>> GetTargetActors() const { return TargetActors; }

	/** Find the gizmo-actor of segment */
	class ANeAbilityGizmoActor* FindGizmoActor(const FWeakAbilitySegmentPtr& SegmentPtr);

protected:
	/** Called when add new segment */
	void OnAddNewSegment(const FWeakAbilitySegmentPtr& SegmentPtr);

	/** called after delete a segment */
	void OnDeleteSegment(const FWeakAbilitySegmentPtr& SegmentPtr);

	/** On select segment */
	void OnSegmentSelectionChanged(const FWeakAbilitySegmentPtr& SegmentPtr);

	/** Clear the gizmo selection */
	void ClearGizmoSelection();

public:
	FOnCreatePlayerDelegate OnCreatePlayerDelegate;
	FOnCreateTargetsDelegate OnCreateTargetsDelegate;

private:
	TWeakPtr<FNeAbilityBlueprintEditor> MyAbilityEditor;

	TWeakObjectPtr<class UGameViewportClient> GameViewportClient;

	/** Player controller for preview */
	TWeakObjectPtr<APlayerController> PlayerController;

	/** Actors created for previewing */
	TWeakObjectPtr<AActor> PlayerActor;
	TArray<TWeakObjectPtr<AActor>> TargetActors;
	TArray<TWeakObjectPtr<AActor>> PreviewActors;

	/** Gizmo Actors */
	TArray<TWeakObjectPtr<ANeAbilityGizmoActor>> GizmoActors;

	/** Currently selecting gizmo actor */
	TWeakObjectPtr<ANeAbilityGizmoActor> GizmoSelecting;

	/** 存储那些需要在reset world时保留下来的Actor */
	TArray<AActor*> InherentActorList;

	/** 标记： 需要更新Viewport */
	bool bInvalidateViewport = false;
};