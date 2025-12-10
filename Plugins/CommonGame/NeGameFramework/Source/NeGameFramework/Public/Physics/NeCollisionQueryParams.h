// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NeCollisionQueryParams.generated.h"

UENUM(BlueprintType)
enum class ENeQueryMobilityType : uint8
{
	Any,
	Static,	//Any shape that is considered static by physx (static mobility)
	Dynamic	//Any shape that is considered dynamic by physx (movable/stationary mobility)
};

/**
 * FNeCollisionQueryParams
 *  引擎默认的 FCollisionQueryParams 在蓝图跟脚本侧并不可用
 *  KismetSystemLibrary中的接口参数过长不利于逻辑编写,
 *  这里额外提供一个可用给脚本使用的版本
 *  并且在NePhysicsLibrary中提供相应的使用接口
 */
USTRUCT(BlueprintType)
struct FNeCollisionQueryParams
{
	GENERATED_USTRUCT_BODY()

	static FNeCollisionQueryParams DefaultQueryParam;

	/** Tag used to provide extra information or filtering for debugging of the trace (e.g. Collision Analyzer) */
	UPROPERTY()
	FName TraceTag = NAME_None;

	/** Tag used to indicate an owner for this trace */
	UPROPERTY()
	FName OwnerTag = NAME_None;

	/** Whether we should trace against complex collision */
	UPROPERTY()
	bool bTraceComplex;

	/** Whether we want to find out initial overlap or not. If true, it will return if this was initial overlap. */
	UPROPERTY()
	bool bFindInitialOverlaps;

	/** Whether we want to return the triangle face index for complex static mesh traces */
	UPROPERTY()
	bool bReturnFaceIndex;

	/** Whether we want to include the physical material in the results. */
	UPROPERTY()
	bool bReturnPhysicalMaterial;

	/** Whether to ignore blocking results. */
	UPROPERTY()
	bool bIgnoreBlocks;

	/** Whether to ignore touch/overlap results. */
	UPROPERTY()
	bool bIgnoreTouches;

	/** Whether to skip narrow phase checks (only for overlaps). */
	UPROPERTY()
	bool bSkipNarrowPhase;

	/** Whether to ignore traces to the cluster union and trace against its children instead. */
	UPROPERTY()
	bool bTraceIntoSubComponents;

	/** Filters query by mobility types (static vs stationary/movable)*/
	UPROPERTY()
	ENeQueryMobilityType MobilityType;

	/** TArray typedef of components to ignore. */
	typedef TArray<uint32, TInlineAllocator<8>> IgnoreComponentsArrayType;

	/** TArray typedef of actors to ignore. */
	typedef TArray<uint32, TInlineAllocator<4>> IgnoreActorsArrayType;

	/** Tracks whether the IgnoreComponents list is verified unique. */
	UPROPERTY()
	mutable bool bComponentListUnique;

	/** Set of actors to ignore during the trace */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> IgnoreActors;

	/** Set of components to ignore during the trace */
	UPROPERTY()
	TArray<TObjectPtr<UPrimitiveComponent>> IgnoreComponents;

	UPROPERTY()
	bool bDebugQuery;

	UPROPERTY(EditAnywhere, Category=DrawDebug)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, Category=DrawDebug)
	float DrawTime = 5.0f;

	UPROPERTY(EditAnywhere, Category=DrawDebug)
	FLinearColor TraceColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, Category=DrawDebug)
	FLinearColor TraceHitColor = FLinearColor::Green;

public:
	FNeCollisionQueryParams()
	{
		bTraceComplex = false;
		MobilityType = ENeQueryMobilityType::Any;
		TraceTag = NAME_None;
		bFindInitialOverlaps = true;
		bReturnFaceIndex = false;
		bReturnPhysicalMaterial = false;
		bComponentListUnique = true;
		bIgnoreBlocks = false;
		bIgnoreTouches = false;
		bSkipNarrowPhase = false;
		bDebugQuery = false;
		bTraceIntoSubComponents = true;
	}

	FNeCollisionQueryParams(FName InTraceTag, bool bInTraceComplex=false, AActor* InIgnoreActor = nullptr)
	: FNeCollisionQueryParams()
	{
		TraceTag = InTraceTag;
		bTraceComplex = bInTraceComplex;
		if (InIgnoreActor != nullptr)
		{
			IgnoreActors.Add(InIgnoreActor);
			OwnerTag = InIgnoreActor->GetFName();
		}
	}

	void ConvertToCollisionQueryParams(FCollisionQueryParams& OutQueryParams) const
	{
		OutQueryParams.TraceTag = TraceTag;
		OutQueryParams.OwnerTag = OwnerTag;
		OutQueryParams.bTraceComplex = bTraceComplex;
		OutQueryParams.bFindInitialOverlaps = bFindInitialOverlaps;
		OutQueryParams.bReturnFaceIndex = bReturnFaceIndex;
		OutQueryParams.bReturnPhysicalMaterial = bReturnPhysicalMaterial;
		OutQueryParams.bIgnoreBlocks = bIgnoreBlocks;
		OutQueryParams.bIgnoreTouches = bIgnoreTouches;
		OutQueryParams.bSkipNarrowPhase = bSkipNarrowPhase;
		OutQueryParams.bTraceIntoSubComponents = bTraceIntoSubComponents;
		OutQueryParams.MobilityType = static_cast<EQueryMobilityType>(MobilityType);
		OutQueryParams.AddIgnoredActors(IgnoreActors);
		OutQueryParams.AddIgnoredComponents(IgnoreComponents);
		OutQueryParams.bDebugQuery = bDebugQuery;
	}

	FCollisionQueryParams ConvertToCollisionQueryParams() const
	{
		FCollisionQueryParams Params;
		ConvertToCollisionQueryParams(Params);
		return MoveTemp(Params);
	}
};

USTRUCT(BlueprintType)
struct FNeCollisionResponseParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FCollisionResponseContainer Response;

	static FNeCollisionResponseParams DefaultResponseParam;
};