// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Misc/NeRootMotionSource.h"

#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#if ROOT_MOTION_DEBUG

float GetCVarDebugRootMotionSourcesLifetime()
{
	static auto* CVarDebugRootMotionSourcesLifetime = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("p.RootMotion.DebugSourceLifeTime"));
	return CVarDebugRootMotionSourcesLifetime->GetValueOnGameThread();
}

#endif

static float EvaluateFloatCurveAtFraction(const UCurveFloat& Curve, const float Fraction)
{
	float MinCurveTime(0.f);
	float MaxCurveTime(1.f);

	Curve.GetTimeRange(MinCurveTime, MaxCurveTime);
	return Curve.GetFloatValue(FMath::GetRangeValue(FVector2f(MinCurveTime, MaxCurveTime), Fraction));
}


/**
 * 主体代码Copy from FRootMotionSource_MoveToForce
 * 增加了部分修改
 */
void FNeRootMotionSource_MoveToForce::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (Duration > UE_SMALL_NUMBER && MovementTickTime > UE_SMALL_NUMBER)
	{
		float MoveFraction = (GetTime() + SimulationTime) / Duration;
		if (TimeMappingCurve)
		{
			MoveFraction = EvaluateFloatCurveAtFraction(*TimeMappingCurve, MoveFraction);
		}

		FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, MoveFraction);
		CurrentTargetLocation += GetPathOffsetInWorldSpace(MoveFraction);

		const FVector CurrentLocation = Character.GetActorLocation();

		FVector Force = (CurrentTargetLocation - CurrentLocation) / MovementTickTime;

		if (bRestrictSpeedToExpected && !Force.IsNearlyZero(UE_KINDA_SMALL_NUMBER))
		{
			// Calculate expected current location (if we didn't have collision and moved exactly where our velocity should have taken us)
			const float PreviousMoveFraction = GetTime() / Duration;
			FVector CurrentExpectedLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, PreviousMoveFraction);
			CurrentExpectedLocation += GetPathOffsetInWorldSpace(PreviousMoveFraction);

			// Restrict speed to the expected speed, allowing some small amount of error
			const FVector ExpectedForce = (CurrentTargetLocation - CurrentExpectedLocation) / MovementTickTime;
			const float ExpectedSpeed = ExpectedForce.Size();
			const float CurrentSpeedSqr = Force.SizeSquared();

			const float ErrorAllowance = 0.5f; // in cm/s
			if (CurrentSpeedSqr > FMath::Square(ExpectedSpeed + ErrorAllowance))
			{
				Force.Normalize();
				Force *= ExpectedSpeed;
			}
		}

		// Debug
#if ROOT_MOTION_DEBUG
		if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnGameThread() != 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = GetCVarDebugRootMotionSourcesLifetime();

			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(), Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Red, true, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Green, true, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), TargetLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Blue, true, DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation+Force, FColor::Blue, true, DebugLifetime);
		}
#endif

		FTransform NewTransform(Force);
		RootMotionParams.Set(NewTransform);
	}
	else
	{
		checkf(Duration > UE_SMALL_NUMBER, TEXT("FRootMotionSource_MoveToForce prepared with invalid duration."));
	}

	SetTime(GetTime() + SimulationTime);
}

bool FNeRootMotionSource_MoveToForce::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource_MoveToForce::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << TimeMappingCurve;
	bOutSuccess = true;
	return true;
}

UScriptStruct* FNeRootMotionSource_MoveToForce::GetScriptStruct() const
{
	return FNeRootMotionSource_MoveToForce::StaticStruct();
}

FString FNeRootMotionSource_MoveToForce::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FNeRootMotionSource_MoveToForce %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FNeRootMotionSource_MoveToForce::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(TimeMappingCurve);
	FRootMotionSource_MoveToForce::AddReferencedObjects(Collector);
}

// *********************************************************
// FNeRootMotionSource_MoveToDynamicForce

void FNeRootMotionSource_MoveToDynamicForce::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	FRootMotionSource_MoveToDynamicForce::PrepareRootMotion(SimulationTime, MovementTickTime, Character, MoveComponent);
}

bool FNeRootMotionSource_MoveToDynamicForce::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return FRootMotionSource_MoveToDynamicForce::NetSerialize(Ar, Map, bOutSuccess);
}

UScriptStruct* FNeRootMotionSource_MoveToDynamicForce::GetScriptStruct() const
{
	return FNeRootMotionSource_MoveToDynamicForce::StaticStruct();
}

FString FNeRootMotionSource_MoveToDynamicForce::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FNeRootMotionSource_MoveToDynamicForce %s"), LocalID, *InstanceName.GetPlainNameString());
}

// *********************************************************
// FNeRootMotionSource_MoveAlongCurve

void FNeRootMotionSource_MoveAlongCurve::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();
	
	if (Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER)
	{
		float MoveFraction = GetTime() * TimeCofficient + SimulationTime;
		
		FVector DeltaTrans = FVector::ZeroVector;
		if (TranslationCurve)
		{
			FVector Translation = TranslationCurve->GetVectorValue(MoveFraction);
			DeltaTrans = Translation - LastTranslation;
			LastTranslation = Translation;
		}

		FRotator Rotator = FRotator::ZeroRotator;
		if (RotationCurve)
		{
			FVector RotationAsVector = RotationCurve->GetVectorValue(MoveFraction);
			Rotator.Roll = RotationAsVector.X;
			Rotator.Pitch = RotationAsVector.Y;
			Rotator.Yaw = RotationAsVector.Z;
		}
		FTransform NewTransform(Rotator.Quaternion(), DeltaTrans, FVector::OneVector);

		Character.GetMesh()->ConditionalUpdateComponentToWorld();
		Character.GetRootComponent()->ConditionalUpdateComponentToWorld();

		const FTransform ActorToWorld = Character.GetTransform();

		// const FTransform ComponentToActor = ActorToWorld.GetRelativeTransform(Character.GetMesh()->GetComponentTransform());
		// const FTransform NewComponentToWorld = NewTransform * Character.GetMesh()->GetComponentTransform();
		// const FTransform NewActorTransform = ComponentToActor * NewComponentToWorld;
		const FTransform DeltaTransform = NewTransform * ActorToWorld;

		FVector DeltaWorldTranslation = ActorToWorld.TransformVector(DeltaTrans);
		DeltaWorldTranslation = DeltaWorldTranslation / MovementTickTime;

		const FQuat NewWorldRotation = ActorToWorld.GetRotation() * NewTransform.GetRotation();
		const FQuat DeltaWorldRotation = NewWorldRotation * ActorToWorld.GetRotation().Inverse();

		FTransform DeltaWorldTransform(DeltaWorldRotation, DeltaWorldTranslation);
		//LastTranslation = Translation;

		//DeltaTranslation = /*Character.GetActorTransform().TransformPosition(*/DeltaTranslation/*)*/;

		/*UE_LOG(LogTemp, Warning, TEXT("Translation %s!"), *(Translation.ToString()));
		UE_LOG(LogTemp, Warning, TEXT("time %f!"), (MoveFraction));
		UE_LOG(LogTemp, Warning, TEXT("Velocity %s!"), *(MoveComponent.Velocity.ToString()));*/

		//FVector RotationAsVector = RotationCurve->GetVectorValue(MoveFraction);
		//FRotator Rotator;
		//Rotator.Roll = RotationAsVector.X;
		//Rotator.Pitch = RotationAsVector.Y;
		//Rotator.Yaw = RotationAsVector.Z;

		//FTransform NewTransform(Rotator.Quaternion(), DeltaTranslation, FVector::OneVector);
		RootMotionParams.Set(DeltaWorldTransform);
	}
	SetTime(GetTime() + SimulationTime);
}

bool FNeRootMotionSource_MoveAlongCurve::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << TranslationCurve;
	Ar << RotationCurve;

	bOutSuccess = true;
	return true;
}

UScriptStruct* FNeRootMotionSource_MoveAlongCurve::GetScriptStruct() const
{
	return FNeRootMotionSource_MoveAlongCurve::StaticStruct();
}

FString FNeRootMotionSource_MoveAlongCurve::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FNeRootMotionSource_MoveAlongCurve %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FNeRootMotionSource_MoveAlongCurve::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(TranslationCurve);
	Collector.AddReferencedObject(RotationCurve);

	FRootMotionSource::AddReferencedObjects(Collector);
}
