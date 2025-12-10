// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/Object.h"
#include "NeGameLoopPhaseSystem.generated.h"

class UNeGameLoopPhase;
class UNeGameLoopPhaseSet;

NEGAMEFRAMEWORK_API DECLARE_LOG_CATEGORY_EXTERN(LogGameLoopPhase, Log, All);

DECLARE_DYNAMIC_DELEGATE_OneParam(FNeGamePhaseDDG, const UNeGameLoopPhase*, Phase);
DECLARE_DELEGATE_OneParam(FNeGamePhaseDG, const UNeGameLoopPhase* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FNeGamePhaseTagDDG, const FGameplayTag&, PhaseTag);
DECLARE_DELEGATE_OneParam(FNeGamePhaseTagDG, const FGameplayTag& PhaseTag);

/**
 * UNeGameLoopPhaseSystem
 *
 * 用来管理游戏循环阶段 PreLogin -> Login -> Playing -> ...
 */
UCLASS(config=Game)
class NEGAMEFRAMEWORK_API UNeGameLoopPhaseSystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~BEGIN: UWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual TStatId GetStatId() const override;
	//~END: UWorldSubsystem interface

	void BeginPhase(const FGameplayTag& InPhase);

	/**
	 * 注册GameLoopPhase信息
	 * 并不希望GameLoopPhase过于散列，希望能统一注册
	 */
	UFUNCTION(BlueprintCallable)
	void RegisterGameLoopPhase(const TSubclassOf<UNeGameLoopPhase>& NewPhaseClass);

	/** Phase开始时通知回调事件 */
	void NotifyPhaseBegin(const FGameplayTag& PhaseTag) const;

	/** Phase结束时通知回调事件 */
	void NotifyPhaseEnd(const FGameplayTag& PhaseTag) const;

	/** 是否处于某个游戏阶段 */
	UFUNCTION(BlueprintCallable)
	bool IsInPhase(const FGameplayTag& PhaseTag) const;

	/**
	 * @brief 注册Phase开始/结束相关回调事件
	 * @param PhaseTag			指定Phase
	 * @param Instigator		注册发起对象
	 * @param WhenPhaseBegin	回调事件
	 */
	void AddPhaseBeginObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator, FNeGamePhaseTagDG WhenPhaseBegin);
	void AddPhaseEndObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator, FNeGamePhaseTagDG WhenPhaseBegin);

	/**
	 * @brief 取消注册的回调事件
	 * @param PhaseTag			指定Phase
	 * @param Instigator		注册发起对象
	 */
	void RemovePhaseObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator);
	void RemovePhaseBeginObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator);
	void RemovePhaseEndObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "When Phase Begin", AutoCreateRefTerm = "WhenPhaseBegin"))
	void K2_AddPhaseBeginObserver(const FGameplayTag& InPhaseTag, FNeGamePhaseTagDDG WhenPhaseBegin);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "When Phase End", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_AddPhaseEndObserver(const FGameplayTag& InPhaseTag, FNeGamePhaseTagDDG WhenPhaseEnd);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Phase Observer"))
	FORCEINLINE void K2_RemovePhaseObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator) { RemovePhaseObserver(InPhaseTag, InInstigator); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Phase Begin Observer"))
	void K2_RemovePhaseBeginObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator) { RemovePhaseBeginObserver(InPhaseTag, InInstigator); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Phase End Observer"))
	void K2_RemovePhaseEndObserver(const FGameplayTag& InPhaseTag, UObject* InInstigator) { RemovePhaseEndObserver(InPhaseTag, InInstigator); }

protected:
	FName MakeGameLoopPhaseObjectName(const FGameplayTag& InPhase, UClass* PhaseClass) const;

	void EndPhase(UNeGameLoopPhase* InPhase);

	void ForEach(const TArray<UNeGameLoopPhase*>& PhaseArray, const TFunction<void(UNeGameLoopPhase*)>& ExeFunc);

public:
	UPROPERTY()
	TArray<UNeGameLoopPhase*> ActivateGameLoopPhases;

	/** 游戏默认的GameLoopPhase配置，可在DefaultGame.ini中配置 */
	UPROPERTY(config, EditDefaultsOnly)
	TSoftObjectPtr<UNeGameLoopPhaseSet> DefaultGameLoopPhaseSet;

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, UClass*> GameLoopPhaseRegistration;

	struct FGameLoopPhaseObserver
	{
	public:
		FGameLoopPhaseObserver() : Instigator(),  BeginPhaseDelegate() {}
		~FGameLoopPhaseObserver() {}

		TWeakObjectPtr<UObject> Instigator;
		union
		{
			FNeGamePhaseTagDG BeginPhaseDelegate;
			FNeGamePhaseTagDG EndPhaseDelegate;
		};
	};
	TMap<FGameplayTag, TArray<FGameLoopPhaseObserver>> PhaserObserverMap;
};
