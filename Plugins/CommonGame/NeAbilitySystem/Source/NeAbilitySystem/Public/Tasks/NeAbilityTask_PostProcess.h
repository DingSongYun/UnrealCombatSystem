// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityTask.h"
#include "Engine/Scene.h"
#include "NeAbilityTask_PostProcess.generated.h"

class UNeCameraModifier_PostProcess;

/**
 * UNeAbilityTask_PostProcess
 * 后效Task基类
 */
UCLASS(Abstract, Category="屏幕后处理")
class NEABILITYSYSTEM_API UNeAbilityTask_PostProcess : public UNeAbilityTask
{
	GENERATED_BODY()

public:
	UNeAbilityTask_PostProcess(const FObjectInitializer& Initializer);

	//~BEGIN: UNeAbilityTask interface
	virtual void Activate() override;
	virtual void SamplePosition(const float Position, const float PreviousPosition) override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnEndTask() override;
	virtual FString GetDebugString() const override;
	//~END: UNeAbilityTask interface

protected:
	UFUNCTION(BlueprintCallable)
	void CreateCameraModifier();

	//~=============================================================================
	// Settings
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Blend)
	float BlendInTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Blend)
	float BlendOutTime = 0.2f;

	TSubclassOf<class UNeCameraModifier_PostProcess> PPModifierClass;

	//~=============================================================================
	// Runtime
protected:
	UPROPERTY(Transient, BlueprintReadWrite)
	class APlayerCameraManager* CameraManager = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	class UNeCameraModifier_PostProcess* PPModifier = nullptr;

	/** Is during blending out*/
	UPROPERTY(Transient)
	uint8 bPendingBlendOut : 1;
};

//=============================================================================
/**
 * UNeAbilityTask_ChangePPSettings
 * 修改后效参数
 */
UCLASS(Category="屏幕后处理", DisplayName="后处理参数")
class NEABILITYSYSTEM_API UNeAbilityTask_ChangePPSettings : public UNeAbilityTask_PostProcess
{
	GENERATED_BODY()

public:
	UNeAbilityTask_ChangePPSettings(const FObjectInitializer& Initializer);
	virtual void Activate() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPostProcessSettings PPSettings;
};

//=============================================================================
/**
 * UNeAbilityTask_PPMaterial
 * 后处理材质
 */
UCLASS(Category="屏幕后处理", DisplayName="后处理材质")
class NEABILITYSYSTEM_API UNeAbilityTask_PPMaterial  : public UNeAbilityTask_PostProcess
{
	GENERATED_BODY()

public:
	UNeAbilityTask_PPMaterial(const FObjectInitializer& Initializer);
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

protected:
	void UpdateMaterialParameters(float DeltaSeconds);

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveUpdateMaterialParameters(float DeltaSeconds);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PP Materials")
	FWeightedBlendable Blendable;

protected:
	UPROPERTY(Transient, BlueprintReadWrite)
	class UMaterialInstanceDynamic* PPMatInstDynamic = nullptr;
};

//=============================================================================
/**
 * UNeAbilityTask_EnableDof
 * 后处理材质
 */
UCLASS(Category="屏幕后处理", DisplayName="景深")
class NEABILITYSYSTEM_API UNeAbilityTask_EnableDof : public UNeAbilityTask_PostProcess
{
	GENERATED_BODY()
public:
	UNeAbilityTask_EnableDof(const FObjectInitializer& Initializer);
	virtual void Activate() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Overrides, meta=(PinHiddenByDefault, InlineEditConditionToggle))
	uint8 bOverride_DepthOfFieldFocalDistance:1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Overrides, meta=(PinHiddenByDefault, InlineEditConditionToggle))
	uint8 bOverride_DepthOfFieldDepthBlurAmount:1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Overrides, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	uint8 bOverride_DepthOfFieldDepthBlurRadius : 1;

	/** Distance in which the Depth of Field effect should be sharp, in unreal units (cm) */
	UPROPERTY(interp, BlueprintReadWrite, Category="Depth of Field", meta=(ClampMin = "0.0", UIMin = "1.0", UIMax = "10000.0", editcondition = "bOverride_DepthOfFieldFocalDistance", DisplayName = "Focal Distance"))
	float DepthOfFieldFocalDistance;

	/** CircleDOF only: Depth blur km for 50% */
	UPROPERTY(interp, BlueprintReadWrite, Category="Depth of Field", meta=(ClampMin = "0.000001", ClampMax = "100.0", editcondition = "bOverride_DepthOfFieldDepthBlurAmount", DisplayName = "Depth Blur km for 50%"))
	float DepthOfFieldDepthBlurAmount;

	/** CircleDOF only: Depth blur radius in pixels at 1920x */
	UPROPERTY(interp, BlueprintReadWrite, Category="Depth of Field", meta=(ClampMin = "0.0", UIMax = "4.0", editcondition = "bOverride_DepthOfFieldDepthBlurRadius", DisplayName = "Depth Blur Radius"))
	float DepthOfFieldDepthBlurRadius;
};