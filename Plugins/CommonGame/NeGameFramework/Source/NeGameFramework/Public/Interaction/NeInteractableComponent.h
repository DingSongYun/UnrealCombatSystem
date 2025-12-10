// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "INeInteractableInterface.h"
#include "Components/ActorComponent.h"
#include "NeInteractableComponent.generated.h"


/**
 * UNeInteractableComponent
 * 一个通用的可交互组件，用来配置交互行为
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEGAMEFRAMEWORK_API UNeInteractableComponent : public UActorComponent, public INeInteractableInterface
{
	GENERATED_BODY()

public:
	UNeInteractableComponent(const FObjectInitializer& Initializer);
	virtual void BeginPlay() override;

	/** 开始交互 */
	void StartInteraction(AActor* Instigator);

	/** 结束交互 */
	void EndInteraction();

	//~BEGIN: INeInteractableInterface
	virtual void GatherInteractionOptions(const FNeInteractionQuery& InteractQuery, FInteractionOptionBuilder& Options) override;
	//~END: INeInteractableInterface

protected:
	UPROPERTY(EditAnywhere)
	TArray<FNeInteractionOption> InteractionOptions;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> InteractionWith;
};
