// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "NeAbilityWeakPtr.h"
#include "Abilities/GameplayAbilityTypes.h"

class UNeAbility;
class UNeAbilityPreviewInstance;

/**
 * INeAbilityPreviewProxy
 *
 * 技能的预览播放器
 */
class INeAbilityEditorPlayer
{
public:
	virtual ~INeAbilityEditorPlayer() {}
	virtual void SetPosition(int32 InSectionIndex, float NewPosition) = 0;
	virtual float GetPosition() const = 0;
	virtual int32 GetCurrentSection() const = 0;
	virtual double GetFrameRate() { return 30.f; }

	/*
	=========================================================
	Preview control
	*/
	virtual bool IsPlaying() const = 0;
	virtual bool IsPaused() const = 0;
	virtual bool IsStopped() const = 0;

	//virtual bool IsLooping() const = 0;
	virtual void Play() = 0;
	virtual void Resume() = 0;
	virtual void Pause() = 0;
	virtual void Stop() = 0;

	//virtual void SetLooping(bool bIsLooping) = 0;

	virtual void PlaySection(int32 Index) = 0;
	virtual bool IsPlayingSection(int32 Index) const = 0;
};

//=============================================================================
/**
 * FNeAbilityEditorPlayer
 */
class FNeAbilityEditorPlayerBase : public INeAbilityEditorPlayer
{
public:
	static TSharedPtr<FNeAbilityEditorPlayerBase> Create(UNeAbility* InAsset, const TSharedPtr<class FNeAbilityBlueprintEditor>& InHostEditor);

public:
	FNeAbilityEditorPlayerBase(UNeAbility* InAsset, const TSharedPtr<class FNeAbilityBlueprintEditor>& InHostEditor);
	virtual ~FNeAbilityEditorPlayerBase() override;

	void InitializeFor(class UNeAbilitySystemComponent* AbilitySystemComponent);

	virtual UNeAbility* GetPreviewAsset() const { return AbilityAsset.Get(); }
	void ResetAsset(UNeAbility* InAsset);

	void OnResetPreviewWorld();

	virtual void Play() override;
	virtual void Resume() override;
	virtual void Pause() override;
	virtual void Stop() override;
	virtual bool IsPlaying() const override;
	virtual bool IsPaused() const override;
	virtual bool IsStopped() const override;
	virtual void SetPosition(int32 InSectionIndex, float NewPosition) override;
	virtual float GetPosition() const override;
	virtual int32 GetCurrentSection() const override;
	virtual float GetCurrentSectionPosition() const;
	virtual void PlaySection(int32 Index) override;
	virtual bool IsPlayingSection(int32 Index) const override;
	virtual float GetPlaySectionTime() const { return 0; }

	void Tick(float DeltaTime);

	/** 预览播放的Instance */
	virtual UNeAbility* GetPlayingInstance() const;

	/** 非播放时的Instance */
	virtual UNeAbility* GetPreviewInstance() const;

	/** get eval context of preview segment */
	struct FNeAbilitySegmentEvalContext* GetPreviewSegmentEvalContext(const FWeakAbilitySegmentPtr& InSegmentPtr);

	virtual void AddReferencedObjects(FReferenceCollector& Collector);

protected:
	virtual void InternalPlay(UNeAbility* InAsset);
	virtual void InternalPlaySection(int32 Index);
	virtual bool IsPlayingSection(UNeAbility* InAsset, int32 Index) const;

	void CreatePreviewInstance(UNeAbility* InAsset);
	void DestroyPreviewInstance();

	/** 获取技能组件 */
	class UNeAbilitySystemComponent* GetAbilityComponent() const;

	/** 获取预览环境中的Player */
	AActor* GetPlayerActor() const;

	void CleanPreviewTask();

	/** 播放结束 */
	void OnPreviewReachEnd(const FAbilityEndedData& AbilityEndedData);
	void OnCreatePreviewPlayer(AActor* PlayerActor);

protected:
	bool bPlaying = false;
	bool bPause = false;
	//bool bLooping = false;

	float ScrubPosition;
	int32 CurrentSection = INDEX_NONE;

	/** Asset to preview */
	TWeakObjectPtr<UNeAbility> AbilityAsset;

	TWeakObjectPtr<UNeAbilityPreviewInstance> PreviewInstance;

	/** Ability runtime instance */
	FGameplayAbilitySpecHandle PlayingAbilitySpecHandle;

	TWeakPtr<class FNeAbilityBlueprintEditor> HostEditor;

	/** Preview scene*/
	TWeakPtr<class FNeAbilityPreviewScene> PreviewScene;

	FTimerHandle ResetTimerHandler;
};