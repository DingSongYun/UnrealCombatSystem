// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NeAbilitySegment.generated.h"

class UNeAbility;
class UNeAbilityBeam;

#if WITH_EDITOR
/**
 * FNeAbilitySegmentDef
 * 用来生成 FNeAbilitySegment的信息
 */
struct FNeAbilitySegment;
struct NEABILITYSYSTEM_API FNeAbilitySegmentDef
{
	UClass* ActionClass = nullptr;

	FNeAbilitySegment* Template = nullptr;

public:
	FNeAbilitySegmentDef(UClass* InActionClass) : ActionClass(InActionClass)
	{
		check(InActionClass);
	}

	bool IsValid() const { return ActionClass != nullptr; }
};
#endif

/**
 * FNeAbilitySegment
 *
 * 技能Track的片段
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilitySegment
{
	GENERATED_BODY()

	friend class FNeAbilitySegmentCustomization;
	friend class UNeAbility;

	static uint32 INVALID_ID;

public:
	FNeAbilitySegment() : FNeAbilitySegment(FNeAbilitySegment::INVALID_ID ) {}
	
	explicit FNeAbilitySegment(uint32 SegID);

	void OnActive();
	void OnEnd();

	uint32 GetID() const { return ID; }

	FORCEINLINE bool IsEnable() const { return bEnable; }

	FORCEINLINE void SetEnable(bool bInEnable) { bEnable = bInEnable; }

	EAbilityDurationPolicy GetDurationPolicy() const;

	bool IsInstant() const;

	float GetDuration() const;

	void SetDuration(float NewDuration) const;

	friend bool operator==(const FNeAbilitySegment& LHS, const FNeAbilitySegment& RHS)
	{
		return LHS.ID == RHS.ID;
	}

	FName GetName() const;

	FORCEINLINE float GetStartTime() const { return StartTime; }

	/** 是否有父的Segment */
	bool HasParent() const { return ParentSegment > FNeAbilitySegment::INVALID_ID; }

	/**
	 * 是否是组合式的Segment
	 */
	bool IsCompound() const;

	/** 获取父Segment的ID */
	uint32 GetParentSegment() const { return ParentSegment; }

	/** 获取所有子Segment的ID */
	TArray<uint32> GetAllChildSegments() const { return Children; }

	UNeAbilityBeam* GetAbilityBeam() const { return Beam; }

private:
	UNeAbilityBeam* CreateBeam(UObject* Outter, UClass* Class, UObject* Template) const;

#if WITH_EDITORONLY_DATA
public:
	/** Get tooltip of the segment */
	FText GetToolTipText() const;

	bool ValidateAsset() const
	{
		return Beam != nullptr;
	}

#endif

#if WITH_EDITOR

public:
	void Construct(class UNeAbility* OutterAbility, const FNeAbilitySegmentDef& SegmentDef);

	/** 获取显示文本 */
	FText GetDisplayText() const;

	/** Set Start time */
	void SetStartTime(float NewStartTime);

	void SetParentSegment(uint32 NewParent) { ParentSegment = NewParent; }

	/** 添加子Segment */
	int32 AddChild(uint32 ChildID);

	/** 移除子Segment */
	int32 RemoveChild(uint32 ChildID);

	bool ShouldCreateGizmo() const;
	/** Gizmo Actor type */
	UClass* GetGizmoActorType() const;

#endif

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Segment")
	uint8 bEnable : 1;

	UPROPERTY(EditDefaultsOnly, Category = "Segment")
	uint32 ID = FNeAbilitySegment::INVALID_ID ;

	UPROPERTY(EditDefaultsOnly, Category = "Segment")
	float StartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "Ability Segment")
	FName SegmentName_Override;

	/** 作为子Segment时的执行模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Segment|Child", meta=(Tooltip="", EditCondition="ParentSegment > 0"))
	ESegmentTriggerMode TriggerMode = ESegmentTriggerMode::Immediately;

	/** 被触发且晚于时间轴时，是否立刻触发 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Segment|Child", meta=(EditCondition="TriggerMode==ESegmentTriggerMode::FollowTimeline && ParentSegment > 0", DisplayName="始终执行", Tooltip="基于时间轴执行得模式下，即使触发晚于时间轴也立刻执行"))
	uint8 bNeverExpire : 1;

	/** Beam: 技能模块化逻辑 */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite)
	TObjectPtr<class UNeAbilityBeam> Beam;

private:
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Segment")
	uint32 ParentSegment = FNeAbilitySegment::INVALID_ID ;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Segment")
	TArray<uint32> Children;
};