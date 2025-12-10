// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "NeInputComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FNeInputActionHandlerValueDDG, const FInputActionValue&, ActionValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FNeInputActionHandlerInstanceDDG, const FInputActionInstance&, ActionInstance);
DECLARE_DYNAMIC_DELEGATE_FourParams(FNeInputActionHandlerValueExDDG, const FInputActionValue&, ActionValue, float, ElapsedTime, float, TriggeredTime, const UInputAction*, SourceAction);

/**
 * FNeInputActionBindingStub
 *
 * InputAction绑定的存根，主要是提供给脚本层用
 */
USTRUCT(BlueprintType)
struct FNeInputActionBindingStub
{
	GENERATED_BODY()
public:
	bool IsValid() const { return Handler > 0 && BindingObject; }

public:
	static FNeInputActionBindingStub INVALID;

	UPROPERTY()
	TObjectPtr<UObject> BindingObject;

	UPROPERTY()
	uint32 Handler = -1;
};

/**
 * FNeInputActionBindingHandlerWrapper
 * 封装一层ActionBindingHandler，保存BindingObject & ActionBindingHandler的联系
 * 主要是给脚本层使用
 */
struct FNeInputActionBindingHandlerWrapper
{
public:
	FNeInputActionBindingHandlerWrapper(UObject* InBindingObject, FEnhancedInputActionEventBinding& InBindingHandler)
		: BindingObject(InBindingObject), BindingHandler(InBindingHandler)
	{}

	FORCEINLINE uint32 GetHandler() const { return BindingHandler.GetHandle(); }

public:
	TWeakObjectPtr<UObject> BindingObject;
	FEnhancedInputActionEventBinding& BindingHandler;
};

/**
 * UNeInputComponent
 *
 * 扩展 UEnhancedInputComponent
 *	1. 扩展支持Tag绑定的InputAction
 *	2. 提供python支持接口
 */
UCLASS(ClassGroup=(NeGame), Blueprintable, config=Game)
class NEGAMEFRAMEWORK_API UNeInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UNeInputComponent(const FObjectInitializer& Initializer = FObjectInitializer::Get());

	template<typename FuncType>
	void BindActionWithTag(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UObject* BindObject, FuncType Func);

	/** 寻找跟特定标签绑定的InputAction*/
	UFUNCTION(BlueprintPure)
	const UInputAction* FindActionOfTag(const FGameplayTag& InputTag) const;

	/**
	 * @brief 监听指定输入，回调参数 FInputActionValue
	 * @param InputTag				输入对应的Tag
	 * @param TriggerEvent			触发类型
	 * @param BindingObject			绑定的对象
	 * @param ActionHandler			回调
	 */
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "ActionHandler"))
	FNeInputActionBindingStub BindActionValueWithTag(const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UObject* BindingObject,
		FNeInputActionHandlerValueDDG ActionHandler);

	/**
	 * @brief 监听指定输入，带有完整输入信息 FInputActionInstance
	 * @param InputTag				输入对应的Tag
	 * @param TriggerEvent			触发类型
	 * @param BindingObject			绑定的对象
	 * @param ActionHandler			回调
	 */
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "ActionHandler"))
	FNeInputActionBindingStub BindActionInstanceWithTag(const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UObject* BindingObject,
		FNeInputActionHandlerInstanceDDG ActionHandler);

	/**
	 * @brief 监听指定输入，回调是部分简化的输入信息
	 * @param InputTag				输入对应的Tag
	 * @param TriggerEvent			触发类型
	 * @param BindingObject			绑定的对象
	 * @param ActionHandler			回调
	 */
	UFUNCTION(BlueprintCallable)
	FNeInputActionBindingStub BindActionValueExWithTag(const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UObject* BindingObject,
		FNeInputActionHandlerValueExDDG ActionHandler);

	/**
	 * @brief 监听指定输入，回调是部分简化的输入信息
	 * @param UInputAction			输入Action
	 * @param TriggerEvent			触发类型
	 * @param BindingObject			绑定的对象
	 * @param ActionHandler			回调
	 */
	UFUNCTION(BlueprintCallable)
	FNeInputActionBindingStub BindActionValueEx(const UInputAction* InAction,
		ETriggerEvent TriggerEvent,
		UObject* BindingObject,
		FNeInputActionHandlerValueExDDG ActionHandler);

	/**
	 * @brief 清楚绑定对象注册的输入监听
	 * @param BindingObject			绑定对象
	 */
	UFUNCTION(BlueprintCallable)
	void ClearBindings(UObject* BindingObject);

	/** 清楚所有输入监听 */
	UFUNCTION(BlueprintCallable)
	void ClearAllBindings();

	/** 解除定输入监听 */
	UFUNCTION(BlueprintCallable)
	void RemoveActionBinding(const FNeInputActionBindingStub& BindingStub);

	/** 解除指定输入监听 */
	UFUNCTION(BlueprintCallable)
	void RemoveActionBindingByHandler(int32 Handler);

	//~==================================================
	// 暴露给蓝图/脚本使用的结构体的方法
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IsValid", DefaultToSelf = InBindingStub))
	static bool InputActionBindingStub_IsValid(const FNeInputActionBindingStub& InBindingStub)
	{
		return InBindingStub.IsValid();
	}

private:
	/** 输入绑定的句柄 */
	TArray<FNeInputActionBindingHandlerWrapper> ActionBindingHandlers;
};

template<typename FuncType>
void UNeInputComponent::BindActionWithTag(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UObject* BindObject, FuncType Func)
{
	check(InputTag.IsValid());
	if (const UInputAction* InputAction = FindActionOfTag(InputTag))
	{
		BindAction(InputAction, TriggerEvent, BindObject, Func);
	}
}
