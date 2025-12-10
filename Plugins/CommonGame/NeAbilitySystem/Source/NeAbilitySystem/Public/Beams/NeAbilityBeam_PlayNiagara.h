// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeam.h"
#include "NeAbilityLocatingData.h"
#include "Curves/CurveVector.h"
#include "FX/NeNiagaraTypes.h"
#include "NeAbilityBeam_PlayNiagara.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/** EFxScaleType */
UENUM(BlueprintType)
enum class EFxScaleType: uint8
{
	None			= 0								UMETA(DisplayName = "不做Scale"),
	Fill											UMETA(DisplayName = "铺满"),
	FitX											UMETA(DisplayName = "宽度适配"),
	FitY											UMETA(DisplayName = "高度适配"),
};

/** FSpawnedNiagaraActorInfo */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FSpawnedNiagaraInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	UNiagaraComponent* NiagaraComponent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	/**
	 * 记录Niagara播放起始点坐标轴
	 * 用来做Offset的基于Curve的变化
	 */
	UPROPERTY(BlueprintReadWrite)
	FTransform OriginTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite)
	float TimeStamp = 0.0f;
};

/**
 * UNeAbilityBeam_PlayNiagara
 */
UCLASS(Category="FX", DisplayName="播放特效")
class NEABILITYSYSTEM_API UNeAbilityBeam_PlayNiagara : public UNeAbilityBeam
{
	GENERATED_UCLASS_BODY()
public:
	//~BEGIN: UNeAbilityBeam interface
	virtual void OnActive(FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason) override;
	virtual void SamplePosition(const float Position, const float PreviousPosition) override;
	virtual FString GetDebugString() const override;
	//~END: UNeAbilityBeam interface

	/** Get all spawned niagara infos*/
	const TArray<FSpawnedNiagaraInfo>& GetSpawnedNiagaraInfos() const { return SpawnedNiagaraInfos; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/** 获取显示文本 */
	virtual FText GetDisplayText() const override;
#endif

protected:
	/** 设置变量 */
	void ApplyNiagaraUserVariable(const FSpawnedNiagaraInfo& SpawnedNiagara) const;

public:
	/** 粒子资源 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	TSoftObjectPtr<UNiagaraSystem> NiagaraAsset;

	/** 粒子播放速率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	float PlayRate = 1.0f;

	/** 是否含有循环粒子 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Niagara")
	bool bHasLoopEmitter = false;

	/** 生成的Niagara是否自动删除，而不是交由Beam控制 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	bool bAutoDestroy = true;

	/** 生成特效是否与生成者同步Slomo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	bool bSlomoWithOwner = true;

	/** 删除延时 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	float FinishDelay = 0.1f;

	/** Niagara暂停时间点 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "Niagara")
	float NiagaraPauseTime = 0.0f;

	// 坐标计算信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform | Offset", meta=(ShowOnlyInnerProperties))
	FNeAbilityLocatingData LocatingData;

	/** 是否进行绑定 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform | Offset")
	bool bNeedAttach = false;

	/** 跟随策略 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform | Offset", meta = (EditCondition = "bNeedAttach"))
	EFxAttachPolicy AttachPolicy = EFxAttachPolicy::LocationAndRotation;

	/** 镜头坐标系下的铺满方式 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform | Offset")
	EFxScaleType ScaleType = EFxScaleType::None;

	/** 是否使用位置更新曲线 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Transform | Curve")
	uint8 bNeedUseCurve : 1;

	/** 是否应用特效贴地 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CheckGround")
	uint8 bNeedCheckGround : 1;

	/** 是否应用贴地位置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bNeedCheckGround"), Category = "CheckGround")
	uint8 bAdjustGroundPosition : 1;

	/** 是否应用贴地法线 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bNeedCheckGround"), Category = "CheckGround")
	uint8 bAdjustRotationByGroungNormal : 1;

	/** 是否应用贴地偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bNeedCheckGround"), Category = "CheckGround")
	FTransform GroundOffset;

	/** 地面检测类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bNeedCheckGround"), Category = "CheckGround")
	TArray<TEnumAsByte<EObjectTypeQuery> > GroundCheckObjectTypes;

	/** 位置曲线 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bNeedUseCurve"), Category = "Transform | Curve")
	FRuntimeVectorCurve LocationCurve;

	/** 旋转曲线 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bNeedUseCurve"), Category = "Transform | Curve")
	FRuntimeVectorCurve RotationCurve;

	/** Scale曲线 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bNeedUseCurve"), Category = "Transform | Curve")
	FRuntimeVectorCurve ScaleCurve;

	/** 设置Niagara StaticMesh变量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Variables")
	TMap<FString, UStaticMesh*> OverrideStaticMesh;

	/** 设置Niagara SkeletalMeshComponent 变量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Variables")
	TMap<FString, FName> OverrideSkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "Parameters")
	EPSCPoolMethod PoolingMethod = EPSCPoolMethod::None;

	//~=============================================================================
	// 运行时变量
private:
	UPROPERTY(Transient)
	TArray<FSpawnedNiagaraInfo> SpawnedNiagaraInfos;

	/** Cache Loaded Asset */
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> NiagaraToPlay = nullptr;
};
