// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityTargetData.h"
#include "NeAbilityLocatingData.generated.h"

class UNeAbilityBeam;

/** ENeAbilityLocatingOrigin: 定位参考的基准点 */
UENUM(BlueprintType)
enum class ENeAbilityLocatingOrigin : uint8
{
	ALT_Target					= 0				UMETA(DisplayName = "技能/Beam的目标的坐标系"),
	ALT_Owner									UMETA(DisplayName = "技能拥有者的坐标系"),
	ALT_Instigator								UMETA(DisplayName = "技能始作俑者的坐标系"),
	ALT_World									UMETA(DisplayName = "世界坐标系"),
	ALT_Socket									UMETA(DisplayName = "Socket坐标系"),
	ALT_Weapon									UMETA(DisplayName = "武器坐标系"),
	ALT_Camera									UMETA(DisplayName = "镜头坐标系"),
	ALT_Custom									UMETA(DisplayName = "自定义")
};

UENUM(BlueprintType)
enum class ENeAbilityLocatingDirection : uint8
{
	ALD_Default					= 0				UMETA(DisplayName = "使用Origin默认朝向"),
	ALD_Owner2Target							UMETA(DisplayName = "使用所属者到目标点的朝向"),
	ALD_Owner2TargetActor						UMETA(DisplayName = "使用所属者到目标Actor的朝向"),
	ALD_Owner2Camera							UMETA(DisplayName = "使用所属者到镜头的朝向")
};

/****************************************************************/
/** FNeAbilityLocatingContext */
USTRUCT(Blueprintable)
struct NEABILITYSYSTEM_API FNeAbilityLocatingContext
{
	GENERATED_USTRUCT_BODY()

	FORCEINLINE bool IsValid() const { return Owner != nullptr; }

public:
	UPROPERTY()
	AActor* Owner = nullptr;

	UPROPERTY()
	AActor* Instigator = nullptr;

	TOptional<FNeAbilityTargetingInfo> Target;
};

/****************************************************************/
/** FLocatingContextBuilder */
struct NEABILITYSYSTEM_API FLocatingContextBuilder
{
	FNeAbilityLocatingContext& Context;
public:
	FLocatingContextBuilder(FNeAbilityLocatingContext& InContext) : Context(InContext) {}
	FLocatingContextBuilder& BuildFromBeam(const UNeAbilityBeam* InBeam);
	FLocatingContextBuilder& UpdateTarget(const FNeAbilityTargetingInfo& InTarget);
};

/****************************************************************/
/**
 * FNeAbilityLocatingData
 * 内置部分逻辑的定位数据
 * TODO: 这部分之后还是要移动到Python侧
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityLocatingData
{
	GENERATED_USTRUCT_BODY()

public:
	FNeAbilityLocatingData() {}

	/**
	 * 计算Transform
	 */
	FTransform GetWorldTransform() const;

	/**
	 * 获取坐标系关联的SceneComponent
	 */
	USceneComponent* GetBaselineComponent() const;

	/** 获取Locating 上下文构建器 */
	FLocatingContextBuilder GetLocatingContextBuilder() const;

	/**
	 * 由一个给定的世界坐标系位置构建TransformCreater
	 *
	 * @param WorldTransform			世界空间下的坐标信息
	 */
	void FromWorldTransform(const FTransform& WorldTransform);

	FORCEINLINE void SetOffsetTransform(const FTransform& InTransform)
	{
		TransformAdd = InTransform;
	}

protected:
	/**
	 * 位置计算
	 *
	 * @param OutTransform			世界坐标系下的Transform
	 * @param OutComponent			挂接的Component
	 * @return 计算成功与否
	 */
	bool TryGetWorldTransform(FTransform& OutTransform, USceneComponent*& OutComponent) const;

	/**
	 * 获取坐标计算的基准SceneComponent
	 */
	bool TryGetBaselineComponent(USceneComponent*& OutComponent) const;

	/**
	 * 获取坐标计算的基准SceneComponent
	 *
	 * @param InLocatingType		定位模式
	 * @param InSocketName			挂接Socket
	 * @param OutComponent			找到的SceneComponent
	 * @return 查找是否成功
	 */
	bool TryGetBaselineComponent(const ENeAbilityLocatingOrigin& InLocatingType, const FName& InSocketName,  USceneComponent*& OutComponent) const;

public:
	/** 定位原点 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENeAbilityLocatingOrigin LocatingType = ENeAbilityLocatingOrigin::ALT_Owner;

	/** 定位朝向 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="LocatingType != ENeAbilityLocatingOrigin::ALT_Custom"))
	ENeAbilityLocatingDirection DirectionType = ENeAbilityLocatingDirection::ALD_Default;

	/** Socket Name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="LocatingType != ENeAbilityLocatingOrigin::ALT_Custom"))
	FName Socket = NAME_None;

	/** 原点坐标偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(PropBinding=True))
	FTransform TransformAdd;

	/** 额外朝向 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(PropBinding=True))
	FRotator ExtraDirection = FRotator::ZeroRotator;

	/** 从Actor身上取坐标时，优先考虑其上Mesh组件的坐标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="LocatingType != ENeAbilityLocatingOrigin::ALT_Custom"))
	bool bMeshPreferred = false;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	mutable FNeAbilityLocatingContext LocatingContext;
};

