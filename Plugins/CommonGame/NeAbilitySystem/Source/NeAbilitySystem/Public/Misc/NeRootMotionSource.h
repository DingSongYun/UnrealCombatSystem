// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RootMotionSource.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "NeRootMotionSource.generated.h"

/**
 * FNeRootMotionSource_MoveToForce
 *
 * 朝着一个给定的目标点移动
 */
USTRUCT()
struct NEABILITYSYSTEM_API FNeRootMotionSource_MoveToForce : public FRootMotionSource_MoveToForce
{
	GENERATED_USTRUCT_BODY()

public:
	FNeRootMotionSource_MoveToForce() {}

	virtual ~FNeRootMotionSource_MoveToForce() override {}

	/** 移动完成度的控制曲线 */
	UPROPERTY()
	TObjectPtr<UCurveFloat> TimeMappingCurve = nullptr;

	virtual void PrepareRootMotion
	(
		float SimulationTime,
		float MovementTickTime,
		const ACharacter& Character,
		const UCharacterMovementComponent& MoveComponent
	) override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;
	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
};

template<>
struct TStructOpsTypeTraits< FNeRootMotionSource_MoveToForce > : public TStructOpsTypeTraitsBase2< FNeRootMotionSource_MoveToForce >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

/**
 * FNeRootMotionSource_MoveToDynamicForce
 *
 * 朝着一个可变的目标点移动
 */
USTRUCT()
struct NEABILITYSYSTEM_API FNeRootMotionSource_MoveToDynamicForce : public FRootMotionSource_MoveToDynamicForce
{
	GENERATED_USTRUCT_BODY()

public:
	FNeRootMotionSource_MoveToDynamicForce() {}

	virtual ~FNeRootMotionSource_MoveToDynamicForce() override {}

	/** TODO: */
	UPROPERTY()
	bool ForcePathOffsetCurve = false;

	virtual void PrepareRootMotion
	(
		float SimulationTime,
		float MovementTickTime,
		const ACharacter& Character,
		const UCharacterMovementComponent& MoveComponent
	) override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;
};

template<>
struct TStructOpsTypeTraits< FNeRootMotionSource_MoveToDynamicForce > : public TStructOpsTypeTraitsBase2< FNeRootMotionSource_MoveToDynamicForce >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

/**
 * FNeRootMotionSource_MoveAlongCurve
 *
 * 沿着给定曲线位移
 */
USTRUCT()
struct NEABILITYSYSTEM_API FNeRootMotionSource_MoveAlongCurve : public FRootMotionSource
{
	GENERATED_USTRUCT_BODY()

public:
	FNeRootMotionSource_MoveAlongCurve() {}

	virtual ~FNeRootMotionSource_MoveAlongCurve() {}

	/** 位移曲线 */
	UPROPERTY()
	UCurveVector* TranslationCurve = nullptr;

	/** 旋转曲线 */
	UPROPERTY()
	UCurveVector* RotationCurve = nullptr;

	UPROPERTY(EditAnywhere, Category="RootMotion")
	float TimeCofficient = 1;

	FVector LastTranslation = FVector::ZeroVector;

	virtual void PrepareRootMotion
	(
		float SimulationTime,
		float MovementTickTime,
		const ACharacter& Character,
		const UCharacterMovementComponent& MoveComponent
	) override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;
	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
};

template<>
struct TStructOpsTypeTraits< FNeRootMotionSource_MoveAlongCurve > : public TStructOpsTypeTraitsBase2< FNeRootMotionSource_MoveAlongCurve >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
