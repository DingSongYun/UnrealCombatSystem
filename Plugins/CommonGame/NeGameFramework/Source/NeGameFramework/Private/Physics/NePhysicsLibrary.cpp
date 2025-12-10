// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Physics/NePhysicsLibrary.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/PhysicsVolume.h"
#include "DrawDebugHelpers.h"
#include "KismetTraceUtils.h"
#include "Components/PrimitiveComponent.h"

FNeCollisionResponseParams		FNeCollisionResponseParams::DefaultResponseParam;
FNeCollisionQueryParams			FNeCollisionQueryParams::DefaultQueryParam("DefaultQueryParam", true);

#if ENABLE_DRAW_DEBUG
void DrawDebugBoxOverlap(const UWorld* World, const FVector& Start, const FVector HalfSize, const FQuat& Orientation, EDrawDebugTrace::Type DrawDebugType, bool bHit, const TArray<FOverlapResult>& OverlapResults, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	if (DrawDebugType == EDrawDebugTrace::None || World == nullptr)
	{
		return;
	}

	const bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
	const float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;
	const uint8 DepthPriority = 0;

	if (bHit)
	{
		::DrawDebugBox(World, Start, HalfSize, Orientation, TraceHitColor.ToFColor(true), bPersistent, LifeTime, DepthPriority);
	}
	else
	{
		::DrawDebugBox(World, Start, HalfSize, Orientation, TraceColor.ToFColor(true), bPersistent, LifeTime, DepthPriority);
	}

	// draw hits
	for (int32 HitIdx = 0; HitIdx < OverlapResults.Num(); ++HitIdx)
	{
		FOverlapResult const& Hit = OverlapResults[HitIdx];
		::DrawDebugPoint(World, Hit.Component->GetComponentLocation(), 16.f, (Hit.bBlockingHit ? TraceColor.ToFColor(true) : TraceHitColor.ToFColor(true)), bPersistent, LifeTime);
	}
}
#endif



APhysicsVolume* UNePhysicsLibrary::GetPhysicsVolume(AActor* InActor)
{
	check(InActor);
	if (InActor)
	{
		// 这里不适应原始Actor实现, 默认实现对于 bShouldUpdatePhysicsVolume的物体存在歧义
		// APhysicsVolume* Volume = InActor->GetPhysicsVolume();
		if (const USceneComponent* ActorRootComponent = InActor->GetRootComponent())
		{
			if (APhysicsVolume* Volume = ActorRootComponent->GetPhysicsVolume())
			{
				return Volume;
			}
			else if (ActorRootComponent->GetShouldUpdatePhysicsVolume() == false)
			{
				return GetPhysicsVolumeAtLocation(InActor, InActor->GetActorLocation());
			}
		}

		return InActor->GetWorld()->GetDefaultPhysicsVolume();
	}
	return nullptr;
}

APhysicsVolume* UNePhysicsLibrary::GetPhysicsVolumeAtLocation(UObject* WorldContextObject, const FVector& AtLocation)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	check(World);

	for (auto VolumeIter = World->GetNonDefaultPhysicsVolumeIterator(); VolumeIter; ++VolumeIter)
	{
		APhysicsVolume* Volume = VolumeIter->Get();
		if (Volume != nullptr)
		{
			const USceneComponent* VolumeRoot = Volume->GetRootComponent();
			if (VolumeRoot)
			{
				if (VolumeRoot->Bounds.GetSphere().IsInside(AtLocation))
				{
					if (VolumeRoot->Bounds.GetBox().IsInside(AtLocation))
					{
						return Volume;
					}
				}
			}
		}
	}

	return World->GetDefaultPhysicsVolume();
}

APhysicsVolume* UNePhysicsLibrary::GetDefaultPhysicsVolume(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		return World->GetDefaultPhysicsVolume();
	}

	return nullptr;
}

float UNePhysicsLibrary::GetGravityZInVolume(APhysicsVolume* InVolume)
{
	check(InVolume);
	return InVolume ? InVolume->GetGravityZ() : 0.f;
}

FCollisionQueryParams ConfigureCollisionParams(const FName& TraceTag, const FNeCollisionQueryParams& FromCollisionQueryParams)
{
	FCollisionQueryParams Params = FromCollisionQueryParams.ConvertToCollisionQueryParams();
	Params.StatId =  SCENE_QUERY_STAT_ONLY(KismetTraceUtils);

	if (Params.TraceTag.IsNone())
	{
		Params.TraceTag = TraceTag;
	}

	return Params;
}

bool UNePhysicsLibrary::LineTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, ECollisionChannel TraceChannel
	, FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName LineTraceSingleName(TEXT("LineTraceSingle"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel,
		ConfigureCollisionParams(LineTraceSingleName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugLineTraceSingle(World, Start, End, Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::LineTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, ECollisionChannel TraceChannel
	, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName LineTraceMultiName(TEXT("LineTraceMulti"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->LineTraceMultiByChannel(OutHits, Start, End, TraceChannel,
		ConfigureCollisionParams(LineTraceMultiName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugLineTraceMulti(World, Start, End, Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;

}

bool UNePhysicsLibrary::LineTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End,
	const FName& ProfileName, FHitResult& OutHit, const FNeCollisionQueryParams& Params)
{
	static const FName LineTraceSingleName(TEXT("LineTraceSingleByProfile"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->LineTraceSingleByProfile(OutHit, Start, End, ProfileName, ConfigureCollisionParams(LineTraceSingleName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugLineTraceSingle(World, Start, End, Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::LineTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End,
	const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params)
{
	static const FName LineTraceMultiName(TEXT("LineTraceMultiByProfile"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->LineTraceMultiByProfile(OutHits, Start, End, ProfileName, ConfigureCollisionParams(LineTraceMultiName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugLineTraceMulti(World, Start, End, Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::SphereTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius,
	ECollisionChannel TraceChannel, FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName SphereTraceSingleName(TEXT("SphereTraceSingle"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(Radius), ConfigureCollisionParams(SphereTraceSingleName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugSphereTraceSingle(World, Start, End, Radius, Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::SphereTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius,
	ECollisionChannel TraceChannel, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName SphereTraceMultiName(TEXT("SphereTraceMulti"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(Radius), ConfigureCollisionParams(SphereTraceMultiName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugSphereTraceMulti(World, Start, End, Radius, Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::SphereTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius,
	const FName& ProfileName, FHitResult& OutHit, const FNeCollisionQueryParams& Params)
{
	static const FName SphereTraceSingleName(TEXT("SphereTraceSingleByProfile"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepSingleByProfile(OutHit, Start, End, FQuat::Identity, ProfileName,
		FCollisionShape::MakeSphere(Radius), ConfigureCollisionParams(SphereTraceSingleName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugSphereTraceSingle(World, Start, End, Radius, Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::SphereTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius,
	const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params)
{
	static const FName SphereTraceMultiName(TEXT("SphereTraceMultiByProfile"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepMultiByProfile(OutHits, Start, End, FQuat::Identity, ProfileName,
		FCollisionShape::MakeSphere(Radius), ConfigureCollisionParams(SphereTraceMultiName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugSphereTraceMulti(World, Start, End, Radius, Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::SphereOverlapByChannel(const UObject* WorldContextObject, const FVector& Location, float Radius,
	ECollisionChannel TraceChannel, TArray<FOverlapResult>& OutOverlaps, const FNeCollisionQueryParams& Params,
	const FNeCollisionResponseParams& ResponseParam)
{
	static const FName SphereOverlapName(TEXT("BoxOverlap"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->OverlapMultiByChannel(OutOverlaps, Location, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(Radius), ConfigureCollisionParams(SphereOverlapName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	// DrawDebugSphere(World, Location, Radius, Params.DrawDebugType, bHit, OutOverlaps, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::CapsuleTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight,
													ECollisionChannel TraceChannel, FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName CapsuleTraceSingleName(TEXT("CapsuleTraceSingle"));

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeCapsule(Radius, HalfHeight), ConfigureCollisionParams(CapsuleTraceSingleName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugCapsuleTraceSingle(World, Start, End, Radius, HalfHeight, Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::CapsuleTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight,
	ECollisionChannel TraceChannel, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName CapsuleTraceMultiName(TEXT("CapsuleTraceMulti"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeCapsule(Radius, HalfHeight), ConfigureCollisionParams(CapsuleTraceMultiName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugCapsuleTraceMulti(World, Start, End, Radius, HalfHeight, Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::CapsuleTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight,
	const FName& ProfileName, FHitResult& OutHit, const FNeCollisionQueryParams& Params)
{
	static const FName CapsuleTraceSingleName(TEXT("CapsuleTraceSingleByProfile"));
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepSingleByProfile(OutHit, Start, End, FQuat::Identity, ProfileName,
		FCollisionShape::MakeCapsule(Radius, HalfHeight), ConfigureCollisionParams(CapsuleTraceSingleName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugCapsuleTraceSingle(World, Start, End, Radius, HalfHeight, Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::CapsuleTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float& Radius, float HalfHeight,
	const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params)
{
	static const FName CapsuleTraceMultiName(TEXT("CapsuleTraceMultiByProfile"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepMultiByProfile(OutHits, Start, End, FQuat::Identity, ProfileName,
		FCollisionShape::MakeCapsule(Radius, HalfHeight), ConfigureCollisionParams(CapsuleTraceMultiName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugCapsuleTraceMulti(World, Start, End, Radius, HalfHeight, Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::BoxTraceSingleByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize, const FQuat& Orientation,
	ECollisionChannel TraceChannel, FHitResult& OutHit, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName BoxTraceSingleName(TEXT("BoxTraceSingle"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepSingleByChannel(OutHit, Start, End, Orientation, TraceChannel,
		FCollisionShape::MakeBox(HalfSize), ConfigureCollisionParams(BoxTraceSingleName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugBoxTraceSingle(World, Start, End, HalfSize, Orientation.Rotator(), Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::BoxTraceMultiByChannel(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize, const FQuat& Orientation,
												ECollisionChannel TraceChannel, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName BoxTraceMultiName(TEXT("BoxTraceMulti"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepMultiByChannel(OutHits, Start, End, Orientation, TraceChannel,
		FCollisionShape::MakeBox(HalfSize), ConfigureCollisionParams(BoxTraceMultiName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugBoxTraceMulti(World, Start, End, HalfSize, Orientation.Rotator(), Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::BoxTraceSingleByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize, const FQuat& Orientation,
	const FName& ProfileName, FHitResult& OutHit, const FNeCollisionQueryParams& Params)
{
	static const FName BoxTraceSingleName(TEXT("BoxTraceSingleByProfile"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	bool const bHit = World ? World->SweepSingleByProfile(OutHit, Start, End, Orientation, ProfileName,
		FCollisionShape::MakeBox(HalfSize), ConfigureCollisionParams(BoxTraceSingleName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugBoxTraceSingle(World, Start, End, HalfSize, Orientation.Rotator(), Params.DrawDebugType, bHit, OutHit, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::BoxTraceMultiByProfile(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FVector& HalfSize,
	const FQuat& Orientation, const FName& ProfileName, TArray<FHitResult>& OutHits, const FNeCollisionQueryParams& Params)
{
	static const FName BoxTraceMultiName(TEXT("BoxTraceMultiByProfile"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepMultiByProfile(OutHits, Start, End, Orientation, ProfileName,
		FCollisionShape::MakeBox(HalfSize), ConfigureCollisionParams(BoxTraceMultiName, Params)) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugBoxTraceMulti(World, Start, End, HalfSize, Orientation.Rotator(), Params.DrawDebugType, bHit, OutHits, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

bool UNePhysicsLibrary::BoxOverlapByChannel(const UObject* WorldContextObject, const FVector& Location,
	const FVector& HalfSize, const FQuat& Orientation, ECollisionChannel TraceChannel, TArray<FOverlapResult>& OutOverlaps,
	const FNeCollisionQueryParams& Params, const FNeCollisionResponseParams& ResponseParam)
{
	static const FName BoxTraceMultiName(TEXT("BoxTraceMulti"));

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->OverlapMultiByChannel(OutOverlaps, Location, Orientation, TraceChannel,
		FCollisionShape::MakeBox(HalfSize), ConfigureCollisionParams(BoxTraceMultiName, Params), ResponseParam.Response) : false;

#if ENABLE_DRAW_DEBUG
	DrawDebugBoxOverlap(World, Location, HalfSize, Orientation, Params.DrawDebugType, bHit, OutOverlaps, Params.TraceColor, Params.TraceHitColor, Params.DrawTime);
#endif

	return bHit;
}

ECollisionChannel UNePhysicsLibrary::TraceTypeToCollisionChannel(ETraceTypeQuery TraceType)
{
	return UEngineTypes::ConvertToCollisionChannel(TraceType);
}

ECollisionChannel UNePhysicsLibrary::ObjectTypeToCollisionChannel(EObjectTypeQuery ObjectType)
{
	return UEngineTypes::ConvertToCollisionChannel(ObjectType);
}

const FCollisionResponseContainer& UNePhysicsLibrary::GetCollisionResponse(const UPrimitiveComponent* PrimitiveComponent)
{
	check(PrimitiveComponent);
	return PrimitiveComponent->BodyInstance.GetResponseToChannels();
}
