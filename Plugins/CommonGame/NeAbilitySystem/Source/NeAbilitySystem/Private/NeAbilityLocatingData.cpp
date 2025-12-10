// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityLocatingData.h"

#include "NeGameplayAbilityLibrary.h"
#include "Beams/NeAbilityBeam.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

FLocatingContextBuilder& FLocatingContextBuilder::BuildFromBeam(const UNeAbilityBeam* InBeam)
{
	if (InBeam)
	{
		Context.Owner = InBeam->GetOwnerActor();
		Context.Instigator = InBeam->GetOwnerActor();
	}
	return *this;
}

FLocatingContextBuilder& FLocatingContextBuilder::UpdateTarget(const FNeAbilityTargetingInfo& InTarget)
{
	Context.Target = InTarget;
	return *this;
}

FTransform FNeAbilityLocatingData::GetWorldTransform() const
{
	FTransform OutTransform;
	if (USceneComponent* AttachedComponent = nullptr; TryGetWorldTransform(OutTransform, AttachedComponent))
	{
		return OutTransform;
	}

	return FTransform::Identity;}

USceneComponent* FNeAbilityLocatingData::GetBaselineComponent() const
{
	if (USceneComponent* OutComponent = nullptr; TryGetBaselineComponent(OutComponent))
	{
		return OutComponent;
	}
	return nullptr;
}

FLocatingContextBuilder FNeAbilityLocatingData::GetLocatingContextBuilder() const
{
	return FLocatingContextBuilder(LocatingContext);
}

void FNeAbilityLocatingData::FromWorldTransform(const FTransform& WorldTransform)
{
	FTransform CoordTransform = FTransform::Identity;

	// This should only occurred when edit task in preview editor
	if (!LocatingContext.IsValid())
	{
		TransformAdd = WorldTransform * CoordTransform.Inverse();
		return ;
	}

	// Base CoordSystem
	USceneComponent* BaselineComponent = nullptr;
	if (TryGetBaselineComponent(BaselineComponent))
	{
		CoordTransform = BaselineComponent->GetComponentTransform();
		if (!Socket.IsNone() && BaselineComponent->DoesSocketExist(Socket))
		{
			CoordTransform = BaselineComponent->GetSocketTransform(Socket);
		}
	}

	// Extra Direction
	if (DirectionType != ENeAbilityLocatingDirection::ALD_Default)
	{
		USceneComponent* DirectionBaselineComponent = nullptr;

		FTransform DirectionTransform = FTransform::Identity;
		if (DirectionType == ENeAbilityLocatingDirection::ALD_Owner2Target)
		{
			if (IsValid(LocatingContext.Owner) && LocatingContext.Target.IsSet())
			{
				FVector XAxis= LocatingContext.Target->GetLocation() - LocatingContext.Owner->GetActorLocation();
				XAxis = XAxis.IsNearlyZero() ? BaselineComponent->GetForwardVector() : XAxis.GetSafeNormal();
				// 将Z轴投影到XZ平面
				FVector ZAxis = BaselineComponent->GetUpVector();
				ZAxis = ZAxis.IsNearlyZero() ? BaselineComponent->GetUpVector() : ZAxis.GetSafeNormal();
				ZAxis = ZAxis.VectorPlaneProject(ZAxis, XAxis);


				DirectionTransform.SetRotation(FRotationMatrix::MakeFromZX(ZAxis, XAxis).ToQuat());
			}
		}
		else if (DirectionType == ENeAbilityLocatingDirection::ALD_Owner2TargetActor)
		{
			if (IsValid(LocatingContext.Owner) && LocatingContext.Target.IsSet() && IsValid(LocatingContext.Target->SourceActor))
			{
				FVector XAxis= LocatingContext.Target->SourceActor->GetActorLocation() - LocatingContext.Owner->GetActorLocation();
				XAxis = XAxis.IsNearlyZero() ? BaselineComponent->GetForwardVector() : XAxis.GetSafeNormal();
				// 将Z轴投影到XZ平面
				FVector ZAxis = BaselineComponent->GetUpVector();
				ZAxis = ZAxis.IsNearlyZero() ? BaselineComponent->GetUpVector() : ZAxis.GetSafeNormal();
				ZAxis = ZAxis.VectorPlaneProject(ZAxis, XAxis);


				DirectionTransform.SetRotation(FRotationMatrix::MakeFromZX(ZAxis, XAxis).ToQuat());
			}
		}
		
		CoordTransform.SetRotation(DirectionTransform.GetRotation());
	}

	if (!ExtraDirection.IsZero())
	{
		CoordTransform.SetRotation(CoordTransform.TransformRotation(ExtraDirection.Quaternion()));
	}

	TransformAdd = WorldTransform * CoordTransform.Inverse();
}

bool FNeAbilityLocatingData::TryGetWorldTransform(FTransform& OutTransform, USceneComponent*& OutComponent) const
{
	OutTransform = FTransform::Identity;
	OutComponent = nullptr;

	// This should only occurred when edit task in preview editor
	if (!LocatingContext.IsValid())
	{
		OutTransform =  TransformAdd * OutTransform;
		return true; // Always not mark false
	}

	// Base CoordSystem
	USceneComponent* BaselineComponent = nullptr;
	if (!TryGetBaselineComponent(BaselineComponent))
	{
		return false;
	}

	if (BaselineComponent)
	{
		OutTransform = BaselineComponent->GetComponentTransform();
		if (!Socket.IsNone() && BaselineComponent->DoesSocketExist(Socket))
		{
			if (LocatingType == ENeAbilityLocatingOrigin::ALT_Socket)
			{
				OutTransform = BaselineComponent->GetSocketTransform(Socket);
			}
			else
			{
				OutTransform.SetLocation(BaselineComponent->GetSocketTransform(Socket).GetLocation());
			}
		}
		OutComponent = BaselineComponent;
	}

	// 在ALT_Camera模式下，去Baseline的可能不准
	if (LocatingType == ENeAbilityLocatingOrigin::ALT_Camera)
	{
		if (const APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(LocatingContext.Owner, 0))
		{
			OutTransform = { PlayerCameraManager->GetCameraRotation(), PlayerCameraManager->GetCameraLocation() };
		}
	}

	// Extra Direction
	if (DirectionType != ENeAbilityLocatingDirection::ALD_Default)
	{
		FTransform DirectionTransform = FTransform::Identity;
		switch (DirectionType)
		{
		case ENeAbilityLocatingDirection::ALD_Owner2Target:
			if (IsValid(LocatingContext.Owner) && LocatingContext.Target.IsSet())
			{
				FVector XAxis= LocatingContext.Target->GetLocation() - LocatingContext.Owner->GetActorLocation();
				XAxis = XAxis.IsNearlyZero() ? BaselineComponent->GetForwardVector() : XAxis.GetSafeNormal();
				// 将Z轴投影到XZ平面
				FVector ZAxis = BaselineComponent->GetUpVector();
				ZAxis = ZAxis.IsNearlyZero() ? BaselineComponent->GetUpVector() : ZAxis.GetSafeNormal();
				ZAxis = ZAxis.VectorPlaneProject(ZAxis, XAxis);
				
				DirectionTransform.SetRotation(FRotationMatrix::MakeFromZX(ZAxis, XAxis).ToQuat());
			}
			break;
		case ENeAbilityLocatingDirection::ALD_Owner2TargetActor:
			if (IsValid(LocatingContext.Owner) && LocatingContext.Target.IsSet() && IsValid(LocatingContext.Target->SourceActor))
			{
				FVector XAxis= LocatingContext.Target->SourceActor->GetActorLocation() - LocatingContext.Owner->GetActorLocation();
				XAxis = XAxis.IsNearlyZero() ? BaselineComponent->GetForwardVector() : XAxis.GetSafeNormal();
				// 将Z轴投影到XZ平面
				FVector ZAxis = BaselineComponent->GetUpVector();
				ZAxis = ZAxis.IsNearlyZero() ? BaselineComponent->GetUpVector() : ZAxis.GetSafeNormal();
				ZAxis = ZAxis.VectorPlaneProject(ZAxis, XAxis);


				DirectionTransform.SetRotation(FRotationMatrix::MakeFromZX(ZAxis, XAxis).ToQuat());
			}
			break;
		case ENeAbilityLocatingDirection::ALD_Owner2Camera:
			break;
		default: check(0);
		}
		OutTransform.SetRotation(DirectionTransform.GetRotation());
	}

	// Apply direction reverse
	if (!ExtraDirection.IsZero())
	{
		OutTransform.SetRotation(OutTransform.TransformRotation(ExtraDirection.Quaternion()));
	}

	/*
	* TODO:其他处理
	*/

	OutTransform =  TransformAdd * OutTransform;

	return true;
}

bool FNeAbilityLocatingData::TryGetBaselineComponent(USceneComponent*& OutComponent) const
{
	return TryGetBaselineComponent(LocatingType, Socket, OutComponent);
}

bool FNeAbilityLocatingData::TryGetBaselineComponent(const ENeAbilityLocatingOrigin& InLocatingType, const FName& InSocketName, USceneComponent*& OutComponent) const
{
	OutComponent = nullptr;
	check(LocatingContext.IsValid());

	if (LocatingType == ENeAbilityLocatingOrigin::ALT_World)
	{
		return true;
	}

	AActor* BaselineActor = nullptr;
	switch ( LocatingType )
	{
		case ENeAbilityLocatingOrigin::ALT_Target:
		{
			BaselineActor = LocatingContext.Target.IsSet() ? LocatingContext.Target->SourceActor : nullptr;
			break;
		}
		case ENeAbilityLocatingOrigin::ALT_Owner:
		case ENeAbilityLocatingOrigin::ALT_Socket:
		{
			BaselineActor = LocatingContext.Owner;
			break;
		}
		case ENeAbilityLocatingOrigin::ALT_Instigator:
		{
			BaselineActor = LocatingContext.Instigator;
			break;
		}
		case ENeAbilityLocatingOrigin::ALT_Camera:
		{
			if (APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(LocatingContext.Owner, 0))
			{
				BaselineActor = PlayerCameraManager->GetViewTarget();
			}
			break;
		}
		case ENeAbilityLocatingOrigin::ALT_Weapon:
		{
			// TODO: 补充基于武器坐标的逻辑
			break;
		}
		case ENeAbilityLocatingOrigin::ALT_Custom:
			// TODO: 补充自定义扩展逻辑
			break;
		default: check(0);
	}

	if (BaselineActor)
	{
		// 考虑到CharacterMesh本身有旋转调整，这里取socket所在Component的坐标系会存在潜在问题
		// 如果之后有这方面的需求，@dingsongyun 进行确认
		if (!Socket.IsNone())
		{
			OutComponent = UNeGameplayAbilityLibrary::GetComponentOfSocket(Socket, BaselineActor);
		}

		// fallback to mesh component
		if (!OutComponent && bMeshPreferred)
		{
			OutComponent = Cast<UMeshComponent>(BaselineActor->GetComponentByClass(UMeshComponent::StaticClass()));
		}

		if (!OutComponent) OutComponent = BaselineActor->GetRootComponent();
	}

	return OutComponent != nullptr;
}
