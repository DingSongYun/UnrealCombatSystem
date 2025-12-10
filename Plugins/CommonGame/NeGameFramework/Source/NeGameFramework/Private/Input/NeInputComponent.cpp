// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Input/NeInputComponent.h"

#include "Input/NeEnhancedPlayerInput.h"
#include "Input/NeInputRegistration.h"

FNeTaggedInputAction FNeTaggedInputAction::INVALID = FNeTaggedInputAction();
FNeInputActionBindingStub FNeInputActionBindingStub::INVALID = FNeInputActionBindingStub();

UNeInputComponent::UNeInputComponent(const FObjectInitializer& Initializer) : Super(Initializer)
{}

const UInputAction* UNeInputComponent::FindActionOfTag(const FGameplayTag& InputTag) const
{
	return UNeEnhancedPlayerInput::FindActionOfTag(InputTag);
}

FNeInputActionBindingStub UNeInputComponent::BindActionValueWithTag(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent,
	UObject* BindingObject, FNeInputActionHandlerValueDDG ActionHandler)
{
	if (const UInputAction* InputAction = FindActionOfTag(InputTag))
	{
		const FNeInputActionBindingHandlerWrapper& HandlerWrapper = ActionBindingHandlers.Add_GetRef({
			BindingObject,
			BindAction(InputAction, TriggerEvent, ActionHandler.GetUObject(), ActionHandler.GetFunctionName())
		});

		return { BindingObject, HandlerWrapper.GetHandler() };
	}

	return FNeInputActionBindingStub::INVALID;
}

FNeInputActionBindingStub UNeInputComponent::BindActionInstanceWithTag(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent,
	UObject* BindingObject, FNeInputActionHandlerInstanceDDG ActionHandler)
{
	if (const UInputAction* InputAction = FindActionOfTag(InputTag))
	{
		const FNeInputActionBindingHandlerWrapper& HandlerWrapper = ActionBindingHandlers.Add_GetRef({
			BindingObject,
			BindAction(InputAction, TriggerEvent, ActionHandler.GetUObject(), ActionHandler.GetFunctionName())
		});

		return { BindingObject, HandlerWrapper.GetHandler() };
	}
	return FNeInputActionBindingStub::INVALID;
}

FNeInputActionBindingStub UNeInputComponent::BindActionValueExWithTag(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent,
	UObject* BindingObject, FNeInputActionHandlerValueExDDG ActionHandler)
{
	if (const UInputAction* InputAction = FindActionOfTag(InputTag))
	{
		const FNeInputActionBindingHandlerWrapper& HandlerWrapper = ActionBindingHandlers.Add_GetRef({
			BindingObject,
			BindAction(InputAction, TriggerEvent, ActionHandler.GetUObject(), ActionHandler.GetFunctionName())
		});

		BindActionValue(InputAction);

		return { BindingObject, HandlerWrapper.GetHandler() };
	}
	return FNeInputActionBindingStub::INVALID;
}

FNeInputActionBindingStub UNeInputComponent::BindActionValueEx(const UInputAction* InAction, ETriggerEvent TriggerEvent, UObject* BindingObject, FNeInputActionHandlerValueExDDG ActionHandler)
{
	if (InAction)
	{
		const FNeInputActionBindingHandlerWrapper& HandlerWrapper = ActionBindingHandlers.Add_GetRef({
				BindingObject,
				BindAction(InAction, TriggerEvent, ActionHandler.GetUObject(), ActionHandler.GetFunctionName())
			});

		return { BindingObject, HandlerWrapper.GetHandler() };
	}
	return FNeInputActionBindingStub::INVALID;
}

void UNeInputComponent::ClearBindings(UObject* InBindingObject)
{
	for (auto It = ActionBindingHandlers.CreateIterator(); It; ++ It)
	{
		if (It->BindingObject == InBindingObject)
		{
			RemoveBindingByHandle(It->GetHandler());
			It.RemoveCurrent();
		}
	}
}

void UNeInputComponent::ClearAllBindings()
{
	for (const FNeInputActionBindingHandlerWrapper& HandlerWrapper : ActionBindingHandlers)
	{
		RemoveBindingByHandle(HandlerWrapper.GetHandler());
	}
	ActionBindingHandlers.Empty();
}

void UNeInputComponent::RemoveActionBinding(const FNeInputActionBindingStub& BindingStub)
{
	const int32 FoundIndex = ActionBindingHandlers.IndexOfByPredicate([&](const FNeInputActionBindingHandlerWrapper& Ele)
	{
		return Ele.GetHandler() == BindingStub.Handler;
	});

	if (FoundIndex != INDEX_NONE)
	{
		ActionBindingHandlers.RemoveAt(FoundIndex);
		RemoveBindingByHandle(BindingStub.Handler);
	}
}

void UNeInputComponent::RemoveActionBindingByHandler(int32 Handler)
{
	RemoveBindingByHandle(Handler);
}