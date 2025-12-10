// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Camera/CameraModifier.h"
#include "NeCameraModifier_PostProcess.generated.h"

//=============================================================================
/**
 * UNeCameraModifier_PostProcess
 * Base class for post-process camera modifier
 */
UCLASS(abstract, config = Camera)
class UNeCameraModifier_PostProcess : public UCameraModifier
{
	GENERATED_BODY()

public:
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override {}
	UFUNCTION(BlueprintCallable)
	virtual void SetAlphaInTime(float InTime) { AlphaInTime = InTime; }
	UFUNCTION(BlueprintCallable)
	virtual void SetAlphaOutTime(float OutTime) { AlphaOutTime = OutTime; }
};

//=============================================================================
/**
 * UNeCameraModifier_ChangePPSettings
 */
UCLASS(config = Camera)
class UNeCameraModifier_ChangePPSettings : public UNeCameraModifier_PostProcess
{
	GENERATED_BODY()
public:
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPostProcessSettings PPSettings;
};

//=============================================================================
/**
 * UNeCameraModifier_PPMaterial
 */
UCLASS(config=Camera)
class UNeCameraModifier_PPMaterial : public UNeCameraModifier_PostProcess
{
	GENERATED_BODY()
public:
	//~BEGIN: UCameraModifer Interface
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;
	virtual void DisableModifier(bool bImmediate = false) override;
	//~END: UCameraModifer Interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeightedBlendable Blendable;
};

//=============================================================================
/**
 * UCameraModifier_ChromaticAberration
 */
UCLASS(config=Camera)
class UCameraModifier_ChromaticAberration : public UNeCameraModifier_PostProcess
{
	GENERATED_UCLASS_BODY()

public:
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;

public:
	UPROPERTY()
	float SceneFringeIntensity;

	UPROPERTY()
	float ChromaticAberrationStartOffset;
};

//=============================================================================
/**
 * UCameraModifier_DepthOfField
 */
UCLASS(config=Camera)
class UCameraModifier_DepthOfField : public UNeCameraModifier_PostProcess
{
	GENERATED_BODY()
public:
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMobileHQGaussian = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FocalRegion = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearTransitionRegion = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FarTransitionRegion = 8000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearBlurSize = 15.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FarBlurSize = 15.f;
};

//=============================================================================
/**
* UCameraModifier_Toon 用于修改PostProcessVolume的Toon相关属性
*/
//UCLASS(config = Camera)
//class UCameraModifier_Toon : public UNeCameraModifier_PostProcess
//{
//	GENERATED_BODY()
//public:
//	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;
//
//public:
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		float SceneColorWeight = 0.f;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		float BrightColorWeight = 0.f;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		float GIScale = 0.f;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		float LightScale = 0.f;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		float SkyLightScale = 0.f;
//};

//=============================================================================
/**
 * UCameraModifier_Bloom
 */
UCLASS(config = Camera)
class UCameraModifier_Bloom : public UNeCameraModifier_PostProcess
{
	GENERATED_BODY()

public:
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;

public:
	UPROPERTY()
	uint8 bOverride_BloomMethod : 1;

	UPROPERTY()
	uint8 bOverride_BloomIntensity : 1;

	UPROPERTY()
	uint8 bOverride_BloomThreshold : 1;

	UPROPERTY()
	uint8 bOverride_Bloom1Tint : 1;

	UPROPERTY()
	uint8 bOverride_Bloom1Size : 1;

	UPROPERTY()
	uint8 bOverride_Bloom2Size : 1;

	UPROPERTY()
	uint8 bOverride_Bloom2Tint : 1;

	UPROPERTY()
	uint8 bOverride_Bloom3Tint : 1;

	UPROPERTY()
	uint8 bOverride_Bloom3Size : 1;

	UPROPERTY()
	uint8 bOverride_Bloom4Tint : 1;

	UPROPERTY()
	uint8 bOverride_Bloom4Size : 1;

	UPROPERTY()
	uint8 bOverride_Bloom5Tint : 1;

	UPROPERTY()
	uint8 bOverride_Bloom5Size : 1;

	UPROPERTY()
	uint8 bOverride_Bloom6Tint : 1;

	UPROPERTY()
	uint8 bOverride_Bloom6Size : 1;

	UPROPERTY()
	uint8 bOverride_BloomSizeScale : 1;

	UPROPERTY()
	uint8 bOverride_BloomConvolutionTexture : 1;

	UPROPERTY()
	uint8 bOverride_BloomConvolutionSize : 1;

	UPROPERTY()
	uint8 bOverride_BloomConvolutionCenterUV : 1;

	UPROPERTY()
	uint8 bOverride_BloomConvolutionPreFilterMin : 1;

	UPROPERTY()
	uint8 bOverride_BloomConvolutionPreFilterMax : 1;

	UPROPERTY()
	uint8 bOverride_BloomConvolutionPreFilterMult : 1;

	UPROPERTY()
	uint8 bOverride_BloomConvolutionBufferScale : 1;

	UPROPERTY()
	TEnumAsByte<enum EBloomMethod> BloomMethod;

	UPROPERTY()
	float BloomIntensity;

	UPROPERTY()
	float BloomThreshold;

	UPROPERTY()
	float BloomSizeScale;

	UPROPERTY()
	float Bloom1Size;

	UPROPERTY()
	float Bloom2Size;

	UPROPERTY()
	float Bloom3Size;

	UPROPERTY()
	float Bloom4Size;

	UPROPERTY()
	float Bloom5Size;
	
	UPROPERTY()
	float Bloom6Size;

	UPROPERTY()
	FLinearColor Bloom1Tint;

	UPROPERTY()
	FLinearColor Bloom2Tint;

	UPROPERTY()
	FLinearColor Bloom3Tint;

	UPROPERTY()
	FLinearColor Bloom4Tint;

	UPROPERTY()
	FLinearColor Bloom5Tint;

	UPROPERTY()
	FLinearColor Bloom6Tint;

	UPROPERTY()
	float BloomConvolutionSize;

	UPROPERTY()
	class UTexture2D* BloomConvolutionTexture;

	UPROPERTY()
	FVector2D BloomConvolutionCenterUV;

	UPROPERTY()
	float BloomConvolutionPreFilterMin;

	UPROPERTY()
	float BloomConvolutionPreFilterMax;

	UPROPERTY()
	float BloomConvolutionPreFilterMult;

	UPROPERTY()
	float BloomConvolutionBufferScale;
};

//=============================================================================
/**
 * UCameraModifier_GI
 */
UCLASS(config = Camera)
class UCameraModifier_GI : public UNeCameraModifier_PostProcess
{
	GENERATED_BODY()

public:
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;

public:
	UPROPERTY()
	uint8 bOverride_IndirectLightingColor : 1;

	UPROPERTY()
	uint8 bOverride_IndirectLightingIntensity : 1;

	UPROPERTY()
	FLinearColor IndirectLightingColor;

	UPROPERTY()
	float IndirectLightingIntensity;
};

//=============================================================================
/**
 * UCameraModifier_Exposure
 */
UCLASS(config = Camera)
class UCameraModifier_Exposure : public UNeCameraModifier_PostProcess
{
	GENERATED_BODY()
public:
	virtual void ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings) override;

public:

	UPROPERTY()
	uint8 bOverride_AutoExposureMethod : 1;

	UPROPERTY()
	TEnumAsByte<enum EAutoExposureMethod> AutoExposureMethod;

	UPROPERTY()
	uint8 bOverride_AutoExposureBias : 1;
	UPROPERTY()
	float AutoExposureBias;

	UPROPERTY()
	uint8 bOverride_AutoExposureApplyPhysicalCameraExposure : 1;
	UPROPERTY()
	uint32 AutoExposureApplyPhysicalCameraExposure : 1;
};