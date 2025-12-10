// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/NeAbilityTask_PostProcess.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "Misc/NeCameraModifier_PostProcess.h"

#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UNeAbilityTask_PostProcess::UNeAbilityTask_PostProcess(const FObjectInitializer& Initializer)
	: Super(Initializer)
	, CameraManager(nullptr)
	, bPendingBlendOut(false)
{
}

void UNeAbilityTask_PostProcess::Activate()
{
	Super::Activate();
	CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	PPModifier = nullptr;
	bPendingBlendOut = false;

	CreateCameraModifier();
	if (PPModifier == nullptr)
	{
		EndTask();
		return ;
	}

	PPModifier->EnableModifier();
	PPModifier->SetAlphaInTime(BlendInTime);
	PPModifier->SetAlphaOutTime(BlendOutTime);
}

void UNeAbilityTask_PostProcess::SamplePosition(const float Position, const float PreviousPosition)
{
	if (PPModifier)
	{
		const float DeltaTime = Position > PreviousPosition ? Position - PreviousPosition : Position;
		FMinimalViewInfo POV = CameraManager->GetCameraCacheView();
		PPModifier->ModifyCamera(DeltaTime, POV);
		CameraManager->SetCameraCachePOV(POV);
	}}

void UNeAbilityTask_PostProcess::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	bool bHasBlendOut = bPendingBlendOut || PPModifier->IsDisabled();
	if (!bHasBlendOut && BlendOutTime > 0.f)
	{
		const float Duration = GetRequiredDuration();
		if (Duration > 0.f)
		{
			float RemainTime = Duration - GetRunningTime();
			if (RemainTime <= BlendOutTime)
			{
				PPModifier->DisableModifier();
				bPendingBlendOut = true;
			}
		}
	}
}

void UNeAbilityTask_PostProcess::OnEndTask()
{
	Super::OnEndTask();
	if (IsValid(PPModifier) && IsValid(CameraManager))
	{
		if (PPModifier->IsDisabled() || BlendOutTime <= 0.f)
		{
			CameraManager->RemoveCameraModifier(PPModifier);
		}
		else
		{
			PPModifier->DisableModifier();
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Modifier=PPModifier, PlayerCameraManager=CameraManager] ()
			{
				if (IsValid(PlayerCameraManager) && IsValid(Modifier))
				{
					PlayerCameraManager->RemoveCameraModifier(Modifier);
				}
			}, BlendOutTime, false);
		}
		PPModifier = nullptr;
	}
}

FString UNeAbilityTask_PostProcess::GetDebugString() const
{
	return FString::Printf(TEXT("Change post process. "));
}

void UNeAbilityTask_PostProcess::CreateCameraModifier()
{
	check(CameraManager);
	if (PPModifierClass)
	{
		PPModifier = Cast<UNeCameraModifier_PostProcess>(CameraManager->AddNewCameraModifier(PPModifierClass));
	}
}

//=============================================================================
/**
 * UNeAbilityTask_ChangePPSettings
 */
UNeAbilityTask_ChangePPSettings::UNeAbilityTask_ChangePPSettings(const FObjectInitializer& Initializer) : Super(Initializer)
{
	PPModifierClass = UNeCameraModifier_ChangePPSettings::StaticClass();
}

void UNeAbilityTask_ChangePPSettings::Activate()
{
	Super::Activate();
	if (UNeCameraModifier_ChangePPSettings* PPSettingModifier = Cast<UNeCameraModifier_ChangePPSettings>(PPModifier))
	{
		PPSettingModifier->PPSettings = PPSettings;
	}
}

//=============================================================================
/**
 * UNeAbilityTask_PPMaterial
 */
UNeAbilityTask_PPMaterial::UNeAbilityTask_PPMaterial(const FObjectInitializer& Initializer) : Super(Initializer)
{
	PPModifierClass = UNeCameraModifier_PPMaterial::StaticClass();
}

void UNeAbilityTask_PPMaterial::Activate()
{
	Super::Activate();
	PPMatInstDynamic = nullptr;
	if (UNeCameraModifier_PPMaterial* PPMaterialModifier = Cast<UNeCameraModifier_PPMaterial>(PPModifier))
	{
		PPMaterialModifier->Blendable.Weight = Blendable.Weight;
		PPMaterialModifier->Blendable.Object = Blendable.Object;

		UMaterialInterface* MaterialInstance = Cast<UMaterialInterface>(Blendable.Object);
		PPMatInstDynamic = MaterialInstance != nullptr ? UMaterialInstanceDynamic::Create(MaterialInstance, this) : nullptr;
		if (PPMatInstDynamic != nullptr)
		{
			UpdateMaterialParameters(0.f);
			ReceiveUpdateMaterialParameters(0.f);
			PPMaterialModifier->Blendable.Object = PPMatInstDynamic;
		}
	}
}

void UNeAbilityTask_PPMaterial::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	UpdateMaterialParameters(DeltaTime);
	ReceiveUpdateMaterialParameters(DeltaTime);
}

void UNeAbilityTask_PPMaterial::UpdateMaterialParameters(float DeltaSeconds)
{
}

//=============================================================================
/**
 * UNeAbilityTask_EnableDof
 */

UNeAbilityTask_EnableDof::UNeAbilityTask_EnableDof(const FObjectInitializer& Initializer) : Super(Initializer)
{
	PPModifierClass = UNeCameraModifier_ChangePPSettings::StaticClass();
}

void UNeAbilityTask_EnableDof::Activate()
{
	Super::Activate();
}