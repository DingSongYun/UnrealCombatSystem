// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreNetTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NeNetworkFunctionLibrary.generated.h"

/**
 * UNeNetworkFunctionLibrary
 *
 * 同步需要用的一些工具方法，大部分是为了给脚本层使用
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNeNetworkFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 将一个Actor的Sub-Object注册为可同步对象
	 *
	 * @param Actor						所属的Actor
	 * @param SubObject					需要同步的子对象
	 * @param NetCondition				同步条件
	 */
	UFUNCTION(BlueprintCallable)
	static void AddActorReplicatedSubObject(AActor* Actor, UObject* SubObject, ELifetimeCondition NetCondition = COND_None);

	/**
	 * 将一个注册过的同步Sub-Object移除
	 */
	UFUNCTION(BlueprintCallable)
	static void RemoveActorReplicatedSubObject(AActor* Actor, UObject* SubObject);

	/**
	 * 将一个Component的Sub-Object注册为可同步对象
	 *
	 * @param ActorComponent			所属的Component
	 * @param SubObject					需要同步的子对象
	 * @param NetCondition				同步条件
	 */
	UFUNCTION(BlueprintCallable)
	static void AddComponentReplicatedSubObject(UActorComponent* ActorComponent, UObject* SubObject, ELifetimeCondition NetCondition = COND_None);

	/**
	 * 将一个注册过的Component的同步Sub-Object移除
	 */
	UFUNCTION(BlueprintCallable)
	static void RemoveComponentReplicatedSubObject(UActorComponent* ActorComponent, UObject* SubObject);
};
