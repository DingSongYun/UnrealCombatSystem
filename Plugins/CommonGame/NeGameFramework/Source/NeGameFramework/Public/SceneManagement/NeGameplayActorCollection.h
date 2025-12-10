// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeGameplayActorCollection.generated.h"

class AActor;

USTRUCT(Blueprintable)
struct FGameplayActorArray
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

/**
 * UNeGameplayActorCollection
 *
 * 场景物件容器
 */
UCLASS(Abstract)
class NEGAMEFRAMEWORK_API UNeGameplayActorCollection : public UObject
{
	GENERATED_BODY()

public:
	virtual void Add(AActor* InActor) PURE_VIRTUAL(UNeGameplayActorCollection::Add, );
	virtual void Remove(AActor* InActor) PURE_VIRTUAL(UNeGameplayActorCollection::Remove, );
	virtual bool FindSceneActorOfClass(UClass* ActorClass, TArray<AActor*>& OutActors) const { return false; }
};

/**
 * UNeGameplayActorCollectionGeneric
 *
 * 简单的场景Actor管理
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeGameplayActorCollectionGeneric : public UNeGameplayActorCollection
{
	GENERATED_BODY()
public:
	virtual void Add(AActor* InActor) override;
	virtual void Remove(AActor* InActor) override;
	virtual bool FindSceneActorOfClass(UClass* ActorClass, TArray<AActor*>& OutActors) const override;

private:
	UPROPERTY()
	TMap<uint32, FGameplayActorArray> ClassifiedActors;
};

/**
 * UNeGameplayActorSpacePartition
 *
 * 基于空间分割的场景物件容器
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeGameplayActorSpacePartition : public UNeGameplayActorCollection
{
	GENERATED_BODY()

public:
};

/**
 * UNeGameplayActorOctree
 *
 * 基于八叉树的空间管理
 * TODO: 之后补充完整
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeGameplayActorOctree : public UNeGameplayActorSpacePartition
{
	GENERATED_BODY()

public:
};

/**
 * UNeGameplayActorHashGrid
 *
 * 基于HashGrid的空间管理
 * TODO: 之后补充完整
 */
UCLASS(Abstract)
class NEGAMEFRAMEWORK_API UNeGameplayActorHashGrid : public UNeGameplayActorSpacePartition
{
	GENERATED_BODY()

public:
};