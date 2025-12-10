// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeCollisionQueryParams.h"
#include "Engine/HitResult.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NePhysicsLibrary.generated.h"

struct FCollisionQueryParams;
class APhysicsVolume;

/**
 * UNePhysicsLibrary
 *
 * 提供一些蓝图/脚本可访问的物理查询接口
 */
UCLASS()
class NEGAMEFRAMEWORK_API UNePhysicsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 获取Actor所处的PhysicsVolume */
	UFUNCTION(BlueprintPure, Category="Physics")
	static APhysicsVolume* GetPhysicsVolume(AActor* InActor);

	/** 获取指定位置的PhysicsVolume */
	UFUNCTION(BlueprintPure, Category="Physics", meta=(WorldContext="WorldContextObject"))
	static APhysicsVolume* GetPhysicsVolumeAtLocation(UObject* WorldContextObject, const FVector& AtLocation);

	/** 获取World默认的PhysicsVolume */
	UFUNCTION(BlueprintPure, Category="Physics", meta=(WorldContext="WorldContextObject"))
	static APhysicsVolume* GetDefaultPhysicsVolume(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Physics", meta=(WorldContext="WorldContextObject"))
	static float GetGravityZInVolume(APhysicsVolume* InVolume);

	/** 提供给脚本用 从HitResult中获取Actor*/
	UFUNCTION(BlueprintPure, Category="Collision")
	static AActor* GetHitResultActor(const FHitResult& HitResult) { return HitResult.GetActor(); }

	//~=============================================================================
	// Collision Trace
	// 这里重新封装适合脚本使用的碰撞检测接口

	/**
	 *  Trace a ray against the world using a specific channel and return the first blocking hit
	 *
	 *  @param  WorldContextObject	World context
	 *  @param  Start           Start location of the ray
	 *  @param  End             End location of the ray
	 *  @param  TraceChannel    The 'channel' that this ray is in, used to determine which components to hit
	 *  @param  OutHit          First blocking hit found
	 *  @param  Params          Additional parameters used for the trace
	 * 	@param 	ResponseParam	ResponseContainer to be used for this trace
	 *  @return TRUE if a blocking hit is found
	 */
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool LineTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start,const FVector& End, ECollisionChannel TraceChannel, struct FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool LineTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start,const FVector& End, ECollisionChannel TraceChannel, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool LineTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FName& ProfileName, struct FHitResult& OutHit, const FNeCollisionQueryParams& Params);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool LineTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params);

	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool SphereTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, ECollisionChannel TraceChannel, FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool SphereTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, ECollisionChannel TraceChannel, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool SphereTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, const FName& ProfileName, FHitResult& OutHit, const FNeCollisionQueryParams& Params);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool SphereTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params);
	UFUNCTION(Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool SphereOverlapByChannel(const UObject* WorldContextObject, const FVector& Location, float Radius, ECollisionChannel TraceChannel, TArray<FOverlapResult>& OutOverlaps, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);

	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool CapsuleTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight, ECollisionChannel TraceChannel, FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool CapsuleTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight, ECollisionChannel TraceChannel, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool CapsuleTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight, const FName& ProfileName, FHitResult& OutHit, const FNeCollisionQueryParams& Params);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool CapsuleTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight, const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params);

	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool BoxTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize, const FQuat& Orientation, ECollisionChannel TraceChannel, FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool BoxTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize, const FQuat& Orientation, ECollisionChannel TraceChannel, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool BoxTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize, const FQuat& Orientation, const FName& ProfileName, FHitResult& OutHit, const FNeCollisionQueryParams& Params);
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool BoxTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize, const FQuat& Orientation, const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params);
	UFUNCTION(Category="Collision", meta=(WorldContext="WorldContextObject"))
	static bool BoxOverlapByChannel(const UObject* WorldContextObject, const FVector& Location, const FVector& HalfSize, const FQuat& Orientation, ECollisionChannel TraceChannel, TArray<FOverlapResult>& OutOverlaps, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam);

	/** FNeCollisionQueryParams 转换到 FCollisionQueryParams */
	static void ConvertCollisionQueryParams(const FNeCollisionQueryParams& RawCollisionQueryParams, FCollisionQueryParams& OutQueryParams);

	/** 从TraceType转换到CollisionChannel */
	UFUNCTION(BlueprintCallable, Category="Collision")
	static ECollisionChannel TraceTypeToCollisionChannel(ETraceTypeQuery TraceType);

	/** EObjectTypeQuery -> ECollisionChannel */
	UFUNCTION(BlueprintCallable, Category="Collision")
	static ECollisionChannel ObjectTypeToCollisionChannel(EObjectTypeQuery ObjectType);

	/** Get Collision Response */
	UFUNCTION(Category="Collision")
	static const FCollisionResponseContainer& GetCollisionResponse(const UPrimitiveComponent* PrimitiveComponent);
};

inline void UNePhysicsLibrary::ConvertCollisionQueryParams(const FNeCollisionQueryParams& RawCollisionQueryParams, FCollisionQueryParams& OutQueryParams)
{
	RawCollisionQueryParams.ConvertToCollisionQueryParams(OutQueryParams);
}

