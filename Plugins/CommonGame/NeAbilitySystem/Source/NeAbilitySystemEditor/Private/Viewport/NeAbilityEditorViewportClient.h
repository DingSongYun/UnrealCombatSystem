// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "LevelEditorViewport.h"
#include "NeAbilityPreviewScene.h"
#include "NeAbilityEditorViewportToolbar.h"
#include "Viewport/NeSimpleEdViewportClient.h"

namespace NeAbilityEditorViewportCameraMode
{
	extern const FName Free;
	extern const FName RealGame;
	extern const FName CameraAttach;
};

//=============================================================================
/**
 * FNeAbilityEditorViewportClient
 */
class FNeAbilityEditorViewportClient : public FNeSimpleEdViewportClient
{
public:
	FNeAbilityEditorViewportClient(TSharedPtr<FNeAbilityBlueprintEditor> InHostEditor, FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InViewportWidget);
	virtual ~FNeAbilityEditorViewportClient();

	//~BEGIN: FEditorViewportClient interface
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;
	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	//~END: FEditorViewportClient interface

	void SetAblViewMode(const FName& InCameraMode);
	FName GetAblViewMode() const { return ViewportCameraMode; }
	void SetViewLockedBone(class USkeletalMeshComponent* InSkeMeshComp, const FName& InBoneName);
	FName GetCameraLockedBone() const;

	void CaptureThumbnail();
	void TickWorld(float DeltaSeconds);

	UActorComponent* FindViewComponentForActor(AActor const* Actor);

	// 重载CalcSceneView，使其支持后效
	//virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const int32 StereoViewIndex = INDEX_NONE) override;
	virtual void OverridePostProcessSettings( FSceneView& View ) override;

	/*设置播放速率*/
	void SetPlaybackSpeedMode(ENeAbilityPlaybackSpeeds::Type InMode);
	ENeAbilityPlaybackSpeeds::Type GetPlaybackSpeedMode() const;

private:
	void HandlerPreviewScenePreTick();
	void HandlerPreviewScenePostTick();

private:
	bool m_CaptureThumbnail;
	TWeakPtr<FNeAbilityBlueprintEditor> HostEditor;
	FName ViewportCameraMode;
	class USkeletalMeshComponent* ViewLockedSkel;
	FName ViewLockedBoneName;

	TWeakObjectPtr<AActor> ActorLocked;

	FViewport* CachedViewPort = NULL;

	float CustomDilation;
	ENeAbilityPlaybackSpeeds::Type PlaybackSpeedMode;
};