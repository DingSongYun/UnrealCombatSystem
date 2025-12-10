// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedPlayerInput.h"
#include "NeInputRegistration.h"
#include "NeEnhancedPlayerInput.generated.h"

/**
 *
 */
UCLASS(config=Game)
class NEGAMEFRAMEWORK_API UNeEnhancedPlayerInput : public UEnhancedPlayerInput
{
	GENERATED_BODY()
public:
	UNeEnhancedPlayerInput();
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

public:
	static void UpdateActionTagMapping(const UInputAction* Action, const struct FGameplayTag& Tag, bool bTagUpdate = false);

	UFUNCTION(BlueprintCallable)
	static UInputAction* FindActionOfTag(const FGameplayTag& InputTag);

	//~==================================================
	// 暴露给蓝图/脚本使用的结构体的方法
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IsValid", DefaultToSelf = InAction))
	static bool TaggedInputAction_IsValid(const FNeTaggedInputAction& InAction)
	{
		return InAction.IsValid();
	}

private:
	void BuildExternalInputRegistration();

public:
	/** 输入配置数据 */
	UPROPERTY(config, EditDefaultsOnly)
	TSoftObjectPtr<class UNeInputRegistration> ExternalInputRegistration;
};
