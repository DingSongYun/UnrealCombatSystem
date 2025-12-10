// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NeUIManagerSubSystem.generated.h"

struct FGameplayTag;
class UNeGameUIPolicy;
class ULocalPlayer;
enum class ESlateVisibility : uint8;

/**
 * UNeUIManagerSubsystem
 *
 * 通用UI管理器
 */
UCLASS(Abstract, config = Game)
class NEGAMEFRAMEWORK_API UNeUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~BEGIN: subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~END: subsystem interface

	UFUNCTION(BlueprintCallable)
	void SetGameUIVisibility(bool bVisible, ULocalPlayer* Player = nullptr);

	void SetGameUIVisibility(ULocalPlayer* Player, ESlateVisibility Visibility) const;

	FORCEINLINE const UNeGameUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	FORCEINLINE UNeGameUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }
	void SetUIPolicy(UNeGameUIPolicy* InPolicy);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	class UCommonActivatableWidget* AddWidgetToLayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	void RemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveInitialize();

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveDeinitialize();

protected:
	void OnPlayerAdded(ULocalPlayer* LocalPlayer);
	void OnPlayerRemoved(ULocalPlayer* LocalPlayer);

	//~==============================================================================================
	// Library functions

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);

	static FName SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);

	static void ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken);

private:
	static int32 InputSuspensionsCount;

private:
	UPROPERTY(Transient)
	TObjectPtr<UNeGameUIPolicy> CurrentPolicy = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UNeGameUIPolicy> DefaultUIPolicyClass;
};
