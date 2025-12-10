// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "DataBoard/NeAbilityDataBoard.h"

#include "NeAbilityTargetData.generated.h"

USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityTargetingInfo : public FGameplayAbilityTargetingLocationInfo
{
	GENERATED_BODY()

	FNeAbilityTargetingInfo() : FGameplayAbilityTargetingLocationInfo() {}
	FNeAbilityTargetingInfo(AActor* InActor) : FGameplayAbilityTargetingLocationInfo()
	{
		SourceActor = InActor;
		LocationType = EGameplayAbilityTargetingLocationType::ActorTransform;
	}

	FNeAbilityTargetingInfo(const FVector& InLocation, const FRotator& InRotation) : FGameplayAbilityTargetingLocationInfo()
	{
		LiteralTransform = FTransform(InRotation, InLocation);
		LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	}

	FNeAbilityTargetingInfo(UMeshComponent* InComponent, const FName& InSocket) : FGameplayAbilityTargetingLocationInfo()
	{
		SourceComponent = InComponent;
		SourceSocketName = InSocket;
		LocationType = EGameplayAbilityTargetingLocationType::SocketTransform;
	}

	FORCEINLINE FVector GetLocation() const { return GetTargetingTransform().GetLocation(); }

	FORCEINLINE bool IsValid() const
	{
		return (LocationType == EGameplayAbilityTargetingLocationType::ActorTransform && SourceActor.Get())
			|| (LocationType == EGameplayAbilityTargetingLocationType::SocketTransform && SourceComponent.Get() && !SourceSocketName.IsNone());
	}

	~FNeAbilityTargetingInfo() {}

	friend bool operator==(const FNeAbilityTargetingInfo& LHS, const FNeAbilityTargetingInfo& RHS)
	{
		return LHS.LocationType == RHS.LocationType
			&& LHS.SourceActor == RHS.SourceActor
			&& LHS.SourceComponent == RHS.SourceComponent
			&& LHS.SourceSocketName == RHS.SourceSocketName;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		FGameplayAbilityTargetingLocationInfo::NetSerialize(Ar, Map, bOutSuccess);

		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FNeAbilityTargetingInfo> : public TStructOpsTypeTraitsBase2<FNeAbilityTargetingInfo>
{
	enum
	{
		WithNetSerializer = true,	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
		WithCopy = true
	};
};

USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityTargetData_Activation : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	/** 技能目标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNeAbilityTargetingInfo> Targets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EActivateAbilityCheckMethod ActivateCheckMethod = EActivateAbilityCheckMethod::AlwaysInterrupt;

	/** 技能的其他信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Transient)
	FNeAbilityDataBoard Data;

	virtual bool HasOrigin() const override { return false; }

	virtual bool HasEndPoint() const override { return false; }

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_LocationInfo::StaticStruct();
	}

	virtual FString ToString() const override
	{
		return TEXT("FNeAbilityTargetData_Activation");
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << ActivateCheckMethod;
		SafeNetSerializeTArray_WithNetSerialize<128>(Ar, Targets, Map);
		Data.NetSerialize(Ar, Map, bOutSuccess);

		bOutSuccess = true;
		return bOutSuccess;
	}
};

template<>
struct TStructOpsTypeTraits<FNeAbilityTargetData_Activation> : public TStructOpsTypeTraitsBase2<FNeAbilityTargetData_Activation>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

/**
 * FNeAbilityTargetData_Generic
 *
 * 通用struck的TargetData
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityTargetData_Generic : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Transient)
	FInstancedStruct Data;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeAbilityTargetData_Generic::StaticStruct();
	}

	bool IsDataValid() const { return Data.IsValid();}

	virtual const UScriptStruct* GetDataStructType() const
	{
		return Data.GetScriptStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Data.NetSerialize(Ar, Map, bOutSuccess);
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FNeAbilityTargetData_Generic> : public TStructOpsTypeTraitsBase2<FNeAbilityTargetData_Activation>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};
