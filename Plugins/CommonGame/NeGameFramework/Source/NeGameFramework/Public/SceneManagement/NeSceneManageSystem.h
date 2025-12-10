// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NeSceneManageSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNeOnSceneReadySignature);

/**
 * UNeSceneManageSystem
 *
 * 场景内物体管理
 */
UCLASS(config=Game, Abstract)
class NEGAMEFRAMEWORK_API UNeSceneManageSystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Gameplay Scene Management"))
	static UNeSceneManageSystem* GetInstance(const UObject* WorldContextObject);

	//~BEGIN: UWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void UpdateStreamingState() override;
	virtual void OnWorldComponentsUpdated(UWorld& World) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual TStatId GetStatId() const override;
	//~END: UWorldSubsystem interface

	UFUNCTION(BlueprintCallable)
	virtual FVector GetActorGravityDirection(AActor* InActor) const;

	UFUNCTION(BlueprintCallable)
	virtual FVector GetGravityDirection(const FVector& InLocation) const;

	/** Actor 注册到场景管理 */
	UFUNCTION(BlueprintCallable)
	virtual void RegisterActor(AActor* InActor);

	/** Actor 从场景管理删除 */
	UFUNCTION(BlueprintCallable)
	virtual void UnregisterActor(AActor* InActor);

	/** 查找注册过的指定类型的Actor */
	UFUNCTION(BlueprintCallable)
	bool FindSceneActorOfClass(UClass* ActorClass, TArray<AActor*>& OutActors) const;

	/**
	 * @return true 场景准备好
	 */
	UFUNCTION(BlueprintPure)
	virtual bool IsSceneReady() const { return bSceneReady; }
	virtual void CheckSceneReady();

protected:
	/** 提供给脚本层 */
	UFUNCTION()
	FORCEINLINE UWorld* K2_GetWorld() const { return GetWorld(); }

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveInitialize();
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveDeinitialize();
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveOnWorldComponentsUpdated(UWorld* World);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveOnWorldBeginPlay();
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveUpdateStreamingState();
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveOnActorSpawned(AActor* SpawnedActor);

	UFUNCTION(BlueprintImplementableEvent)
	FVector ReceiveGetActorGravityDirection(AActor* InActor) const;

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveCheckSceneReady() const;

	UFUNCTION(BlueprintImplementableEvent)
	FVector ReceiveGetGravityDirection(const FVector& InLocation) const;

	//~=============================================================================
	// World Delegate callbacks
	virtual void OnActorSpawned(AActor* SpawnedActor);

public:
	UPROPERTY(BlueprintAssignable)
	FNeOnSceneReadySignature OnSceneReady;

protected:
	UPROPERTY()
	uint8 bSceneReady : 1;

	/** 场景Actor管理容器 */
	UPROPERTY()
	TObjectPtr<class UNeGameplayActorCollection> ActorCollection;

	FDelegateHandle ActorSpawnedDelegateHandle;
};

inline TStatId UNeSceneManageSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNeSceneManageSystem, STATGROUP_Tickables);
}
