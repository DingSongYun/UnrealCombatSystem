// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NeInteractAction.generated.h"

/**
 * UNeInteractAction
 */
UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class NEGAMEFRAMEWORK_API UNeInteractAction : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UObject* InContext, class AActor* InOwnerActor);

	/** 出发交互 */
	UFUNCTION()
	void TriggerInteraction(AActor* Instigator, AActor* Target);

	/** 完成交互 */
	UFUNCTION()
	void CompleteInteraction();

	UFUNCTION()
	FORCEINLINE bool IsValid() { return ReceiveIsValid();}

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveInitialize(UObject* InContext, class AActor* InOwnerActor);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveTriggerInteraction(AActor* Instigator, AActor* Target);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveCompleteInteraction();
	UFUNCTION(BlueprintImplementableEvent)
	bool ReceiveIsValid();

public:
	/** 交互显示文字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	/** 交互组件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<class UUserWidget> WidgetClass;

protected:
	/** 交互的上下文对象，一般是配置该交互的组件 */
	UPROPERTY(Transient)
	TObjectPtr<UObject> ContextObject;

	/** 该交互所属对象，一般而言是所挂在的Actor */
	UPROPERTY(Transient)
	TObjectPtr<AActor> OwnerActor;
};
