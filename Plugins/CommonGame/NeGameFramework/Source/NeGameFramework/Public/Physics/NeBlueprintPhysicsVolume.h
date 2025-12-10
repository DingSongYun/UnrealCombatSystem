// Copyright NetEase Games, Inc. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPhysicsVolume.h"
#include "GameFramework/PhysicsVolume.h"
#include "UObject/Object.h"
#include "NeBlueprintPhysicsVolume.generated.h"

/**
 * UNeBlueprintPhysicsVolume
 *
 * 提供一个方便蓝图或者脚本继承的PhysicsVolume
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class NEGAMEFRAMEWORK_API ANeBlueprintPhysicsVolume : public APhysicsVolume
{
	GENERATED_BODY()

public:
	//~BEGIN: APhysicsVolume interface
	virtual float GetGravityZ() const override;
	virtual void ActorEnteredVolume(class AActor* Other) override;
	virtual void ActorLeavingVolume(class AActor* Other) override;
	virtual bool IsOverlapInVolume(const class USceneComponent& TestComponent) const override;
	//~END: APhysicsVolume interface

	UFUNCTION(BlueprintPure)
	FORCEINLINE float K2_GetGravityZ() const
	{
		return GetGravityZ();
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool K2_IsOverlapInVolume(USceneComponent* TestComponent) const
	{
		if (!IsValid(TestComponent))
		{
			check(0);
			return false;
		}

		return IsOverlapInVolume(*TestComponent);
	}

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveActorEnteredVolume(class AActor* Other);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveActorLeavingVolume(class AActor* Other);
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class NEGAMEFRAMEWORK_API ANeBlueprintDefaultPhysicsVolume : public ADefaultPhysicsVolume
{
	GENERATED_BODY()

public:
	//~BEGIN: APhysicsVolume interface
	virtual void ActorEnteredVolume(class AActor* Other) override;
	virtual void ActorLeavingVolume(class AActor* Other) override;
	//~END: APhysicsVolume interface

	UFUNCTION(BlueprintPure)
	FORCEINLINE float K2_GetGravityZ() const
	{
		return GetGravityZ();
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool K2_IsOverlapInVolume(USceneComponent* TestComponent) const
	{
		if (!IsValid(TestComponent))
		{
			check(0);
			return false;
		}

		return IsOverlapInVolume(*TestComponent);
	}

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveActorEnteredVolume(class AActor* Other);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveActorLeavingVolume(class AActor* Other);
};