// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeam.h"
#include "NeAbilitySegmentEvalQueue.h"
#include "NeAbilityBeam_BlueprintBase.generated.h"

/**
 * UNeAbilityBeam_BlueprintBase
 * 蓝图/脚本 实现基类
 */
UCLASS(abstract, Blueprintable, BlueprintType)
class NEABILITYSYSTEM_API UNeAbilityBeam_BlueprintBase : public UNeAbilityBeam
{
	GENERATED_BODY()

protected:
	//~BEGIN: UNeAbilityBeam interface
	virtual void InitInstanceFor(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnActive(FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnUpdate(float DeltaTime, FNeAbilitySegmentEvalContext& EvalContext) override;
	virtual void OnEnd(FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason) override;
	virtual void SamplePosition(const float Position, const float PreviousPosition) override;
#if WITH_EDITOR
	virtual FText GetDisplayText() const override;
#endif
	//~END: UNeAbilityBeam interface

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnInstanceInitialize"))
	void ReceiveOnInstanceInitialize() const;

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnActive"))
	void ReceiveActive(const FNeAbilitySegmentEvalContext& EvalContext) const;

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnUpdate"))
	void ReceiveUpdate(float DeltaTime, const FNeAbilitySegmentEvalContext& EvalContext) const;

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnEnd"))
	void ReceiveEnd(const FNeAbilitySegmentEvalContext& EvalContext, EAbilityBeamEndReason EndReason) const;

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "SamplePosition"))
	void ReceiveSamplePosition(const float Position, const float PreviousPosition) const;

	/** 用来给脚本自定义显示字串 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "GetDisplayText"))
	void MakeDisplayText(const FText& RawText, FText& Text) const;
};
