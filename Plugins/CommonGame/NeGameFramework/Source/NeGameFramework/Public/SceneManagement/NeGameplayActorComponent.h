// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NeGameplayActorComponent.generated.h"

/**
 * UNeGameplayActorComponent
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Gameplay Actor")
class NEGAMEFRAMEWORK_API UNeGameplayActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNeGameplayActorComponent();
	//~BEGIN: UActorComponent interface
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~END: UActorComponent interface

protected:
	void RegisterToSceneManagement();
	void UnregisterFromSceneManagement();

protected:
	UPROPERTY(EditAnywhere, Category="Gameplay Actor")
	uint8 bNeedRegisterToSceneManagement : 1;

private:
	uint8 bRegisteredToSceneManagement : 1;
};
