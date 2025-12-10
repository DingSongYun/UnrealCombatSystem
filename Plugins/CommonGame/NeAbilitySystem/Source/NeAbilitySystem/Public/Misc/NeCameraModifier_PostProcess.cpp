// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCameraModifier_PostProcess.h"

//=============================================================================
/**
 * UNeCameraModifier_ChangePPSettings
 */
void UNeCameraModifier_ChangePPSettings::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
{
	PostProcessBlendWeight = 1.f * Alpha;
	PostProcessSettings = PPSettings;
}

//=============================================================================
/**
 * UNeCameraModifier_PPMaterial
 */
void UNeCameraModifier_PPMaterial::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
{
	PostProcessBlendWeight = 1.f * Alpha;
	PostProcessSettings.WeightedBlendables.Array.Add(Blendable);
}

void UNeCameraModifier_PPMaterial::DisableModifier(bool bImmediate)
{
	Super::DisableModifier(bImmediate);
}

//=============================================================================
/**
 * UCameraModifier_ChromaticAberration
 */
UCameraModifier_ChromaticAberration::UCameraModifier_ChromaticAberration(const FObjectInitializer& Initializer)
	: Super(Initializer)
	, SceneFringeIntensity(0.f)
	, ChromaticAberrationStartOffset(0.f)
{}

void UCameraModifier_ChromaticAberration::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
{
	PostProcessBlendWeight = 1.f * Alpha;

	PostProcessSettings.bOverride_SceneFringeIntensity = true;
	PostProcessSettings.SceneFringeIntensity = SceneFringeIntensity;

	PostProcessSettings.bOverride_ChromaticAberrationStartOffset = true;
	PostProcessSettings.ChromaticAberrationStartOffset = ChromaticAberrationStartOffset;

	FWeightedBlendable Blendable = PostProcessSettings.WeightedBlendables.Array.AddDefaulted_GetRef();
}

//=============================================================================
/**
 * UCameraModifier_DepthOfField
 */
void UCameraModifier_DepthOfField::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
{
	PostProcessBlendWeight = 1.f * Alpha;
	PostProcessSettings.bOverride_MobileHQGaussian = true;
	PostProcessSettings.bMobileHQGaussian = bMobileHQGaussian;

	PostProcessSettings.bOverride_DepthOfFieldFocalRegion = true;
	PostProcessSettings.DepthOfFieldFocalRegion = FocalRegion;

	PostProcessSettings.bOverride_DepthOfFieldNearTransitionRegion = true;
	PostProcessSettings.DepthOfFieldNearTransitionRegion = NearTransitionRegion;

	PostProcessSettings.bOverride_DepthOfFieldFarTransitionRegion = true;
	PostProcessSettings.DepthOfFieldFarTransitionRegion = FarTransitionRegion;

	PostProcessSettings.bOverride_DepthOfFieldScale = true;
	PostProcessSettings.DepthOfFieldScale = Scale;

	PostProcessSettings.bOverride_DepthOfFieldNearBlurSize = true;
	PostProcessSettings.DepthOfFieldNearBlurSize = NearBlurSize;

	PostProcessSettings.bOverride_DepthOfFieldFarBlurSize = true;
	PostProcessSettings.DepthOfFieldFarBlurSize = FarBlurSize;
	//UE_LOG(LogTemp, Warning, TEXT("UCameraModifier_DepthOfField::ModifyPostProcess: %f"), Alpha);
}

//=============================================================================
/**
 * UCameraModifier_Toon
 */
//void UCameraModifier_Toon::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
//{
//	PostProcessBlendWeight = 1.f * Alpha;
//	PostProcessSettings.bOverride_SceneColorWeight = true;
//	PostProcessSettings.SceneColorWeight = SceneColorWeight;
//
//	PostProcessSettings.bOverride_BrightColorWeight = true;
//	PostProcessSettings.BrightColorWeight = BrightColorWeight;
//
//	PostProcessSettings.bOverride_GIScale = true;
//	PostProcessSettings.GIScale = GIScale;
//
//	PostProcessSettings.bOverride_LightScale = true;
//	PostProcessSettings.LightScale = LightScale;
//
//	PostProcessSettings.bOverride_SkyLightScale = true;
//	PostProcessSettings.SkyLightScale = SkyLightScale;
//	UE_LOG(LogTemp, Warning, TEXT("UCameraModifier_Toon::ModifyPostProcess: %f"), Alpha);
//}

void UCameraModifier_Bloom::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
{
	PostProcessBlendWeight = 1.f * Alpha;
	PostProcessSettings.bOverride_BloomMethod = bOverride_BloomMethod;
	PostProcessSettings.BloomMethod = BloomMethod;

	PostProcessSettings.bOverride_BloomIntensity = bOverride_BloomIntensity;
	PostProcessSettings.BloomIntensity = BloomIntensity;

	PostProcessSettings.bOverride_BloomThreshold = bOverride_BloomThreshold;
	PostProcessSettings.BloomThreshold = BloomThreshold;

	PostProcessSettings.bOverride_BloomSizeScale = bOverride_BloomSizeScale;
	PostProcessSettings.BloomSizeScale = BloomSizeScale;

	PostProcessSettings.bOverride_Bloom1Size = bOverride_Bloom1Size;
	PostProcessSettings.Bloom1Size = Bloom1Size;

	PostProcessSettings.bOverride_Bloom2Size = bOverride_Bloom2Size;
	PostProcessSettings.Bloom2Size = Bloom2Size;

	PostProcessSettings.bOverride_Bloom3Size = bOverride_Bloom3Size;
	PostProcessSettings.Bloom3Size = Bloom3Size;

	PostProcessSettings.bOverride_Bloom4Size = bOverride_Bloom4Size;
	PostProcessSettings.Bloom4Size = Bloom4Size;

	PostProcessSettings.bOverride_Bloom5Size = bOverride_Bloom5Size;
	PostProcessSettings.Bloom5Size = Bloom5Size;

	PostProcessSettings.bOverride_Bloom6Size = bOverride_Bloom6Size;
	PostProcessSettings.Bloom6Size = Bloom6Size;

	PostProcessSettings.bOverride_Bloom1Tint = bOverride_Bloom1Tint;
	PostProcessSettings.Bloom1Tint = Bloom1Tint;

	PostProcessSettings.bOverride_Bloom2Tint = bOverride_Bloom2Tint;
	PostProcessSettings.Bloom2Tint = Bloom2Tint;

	PostProcessSettings.bOverride_Bloom3Tint = bOverride_Bloom3Tint;
	PostProcessSettings.Bloom3Tint = Bloom3Tint;

	PostProcessSettings.bOverride_Bloom4Tint = bOverride_Bloom4Tint;
	PostProcessSettings.Bloom4Tint = Bloom4Tint;

	PostProcessSettings.bOverride_Bloom5Tint = bOverride_Bloom5Tint;
	PostProcessSettings.Bloom5Tint = Bloom5Tint;

	PostProcessSettings.bOverride_Bloom6Tint = bOverride_Bloom6Tint;
	PostProcessSettings.Bloom6Tint = Bloom6Tint;

	PostProcessSettings.bOverride_BloomConvolutionSize = bOverride_BloomConvolutionSize;
	PostProcessSettings.BloomConvolutionSize = BloomConvolutionSize;

	PostProcessSettings.bOverride_BloomConvolutionTexture = bOverride_BloomConvolutionTexture;
	PostProcessSettings.BloomConvolutionTexture = BloomConvolutionTexture;

	PostProcessSettings.bOverride_BloomConvolutionCenterUV = bOverride_BloomConvolutionCenterUV;
	PostProcessSettings.BloomConvolutionCenterUV = BloomConvolutionCenterUV;

	PostProcessSettings.bOverride_BloomConvolutionPreFilterMin = bOverride_BloomConvolutionPreFilterMin;
	PostProcessSettings.BloomConvolutionPreFilterMin = BloomConvolutionPreFilterMin;

	PostProcessSettings.bOverride_BloomConvolutionPreFilterMax = bOverride_BloomConvolutionPreFilterMax;
	PostProcessSettings.BloomConvolutionPreFilterMax = BloomConvolutionPreFilterMax;

	PostProcessSettings.bOverride_BloomConvolutionPreFilterMult = bOverride_BloomConvolutionPreFilterMult;
	PostProcessSettings.BloomConvolutionPreFilterMult = BloomConvolutionPreFilterMult;

	PostProcessSettings.bOverride_BloomConvolutionBufferScale = bOverride_BloomConvolutionBufferScale;
	PostProcessSettings.BloomConvolutionBufferScale = BloomConvolutionBufferScale;

	//UE_LOG(LogTemp, Warning, TEXT("UCameraModifier_Bloom::ModifyPostProcess: %f, %f"), PostProcessSettings.bOverride_BloomIntensity, Alpha);
	//UE_LOG(LogTemp, Warning, TEXT("UCameraModifier_Bloom::ModifyPostProcess: %f"), PostProcessSettings.BloomThreshold);
}

void UCameraModifier_GI::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
{
	PostProcessBlendWeight = 1.f * Alpha;
	PostProcessSettings.bOverride_IndirectLightingColor = bOverride_IndirectLightingColor;
	PostProcessSettings.IndirectLightingColor = IndirectLightingColor;

	PostProcessSettings.bOverride_IndirectLightingIntensity = bOverride_IndirectLightingIntensity;
	PostProcessSettings.IndirectLightingIntensity = IndirectLightingIntensity;

	//UE_LOG(LogTemp, Warning, TEXT("UCameraModifier_GI::ModifyPostProcess: %f"), PostProcessSettings.IndirectLightingIntensity);
}

void UCameraModifier_Exposure::ModifyPostProcess(float DeltaTime, float& PostProcessBlendWeight, FPostProcessSettings& PostProcessSettings)
{
	PostProcessBlendWeight = 1.f * Alpha;
	PostProcessSettings.bOverride_AutoExposureMethod = bOverride_AutoExposureMethod;
	PostProcessSettings.AutoExposureMethod = AutoExposureMethod;

	PostProcessSettings.bOverride_AutoExposureBias = bOverride_AutoExposureBias;
	PostProcessSettings.AutoExposureBias = AutoExposureBias;

	PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = bOverride_AutoExposureApplyPhysicalCameraExposure;
	PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = AutoExposureApplyPhysicalCameraExposure;
}