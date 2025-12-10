// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Object.h"
#include "NeAbilityPreviewActor.generated.h"

class UWorld;

//=============================================================================
/**
 * UNeAbilityPreviewActor
 */
UCLASS(Abstract, EditInlineNew)
class NEABILITYSYSTEM_API UNeAbilityPreviewActorType : public UObject
{
	GENERATED_BODY()

public:
	virtual AActor* CreateActor(class UWorld* InWorld, const FTransform& InTransform) { return nullptr; }
	virtual void PostCreateActor(AActor* InActor) {}
};

//=============================================================================
/**
 * UNeAbilityPreviewActorCommon
 */
UCLASS(Blueprintable)
class NEABILITYSYSTEM_API UNeAbilityPreviewActorCommon : public UNeAbilityPreviewActorType
{
	GENERATED_UCLASS_BODY()
public:
	virtual AActor* CreateActor(class UWorld* InWorld, const FTransform& InTransform) override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ActorClass = nullptr;
};

UCLASS(Abstract, Blueprintable, EditInlineNew)
class NEABILITYSYSTEM_API UNeAbilityPreviewActorBlueprintType : public UNeAbilityPreviewActorType
{
	GENERATED_BODY()

public:
	virtual AActor* CreateActor(class UWorld* InWorld, const FTransform& InTransform) override
	{
		CachedWorld = InWorld;

		return Recv_CreateActor(InWorld, InTransform);
	}

	virtual void PostCreateActor(AActor* InActor) override
	{
		Recv_PostCreateActor(InActor);
	}

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="CreateActor"))
	AActor* Recv_CreateActor(class UWorld* InWorld, const FTransform& InTransform);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="PostCreateActor"))
	void Recv_PostCreateActor(AActor* InActor);


	UFUNCTION(BlueprintCallable)
	AActor* SpawnActorDeferred(UClass* ActorClass, const FTransform& InTransform);

	UFUNCTION(BlueprintCallable)
	void FinishSpawning(AActor* Actor, const FTransform & InTransform)
	{
		Actor->FinishSpawning(InTransform);
	}

	class UWorld* GetWorld() const override { return CachedWorld; }

private:
	UPROPERTY()
	UWorld* CachedWorld = nullptr;
};

USTRUCT(BlueprintType)
struct FNeAbilityPreviewActors
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Preview Player")
	UNeAbilityPreviewActorType* PlayerInfo = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Preview Player")
	FTransform PlayerTransform = FTransform(FRotator::ZeroRotator, FVector(-200.0f, 0.0f, 100.0f));

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Preview Target")
	UNeAbilityPreviewActorType* TargetInfo = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Preview Target")
	FTransform TargetTransform = FTransform(FRotator(0.0f, 180.0f, 0.0f), FVector(200.0f, 0.0f, 100.0f));
};