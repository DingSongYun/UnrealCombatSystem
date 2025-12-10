// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SceneManagement/NeGameplayActorComponent.h"
#include "SceneManagement/NeSceneManageSystem.h"

UNeGameplayActorComponent::UNeGameplayActorComponent()
{
	bNeedRegisterToSceneManagement = true;
	bRegisteredToSceneManagement = false;
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
	SetIsReplicated(false);
}

void UNeGameplayActorComponent::OnRegister()
{
	Super::OnRegister();
	RegisterToSceneManagement();
}

void UNeGameplayActorComponent::OnUnregister()
{
	UnregisterFromSceneManagement();
	Super::OnUnregister();
}

void UNeGameplayActorComponent::BeginPlay()
{
	Super::BeginPlay();
	RegisterToSceneManagement();
}

void UNeGameplayActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterFromSceneManagement();
	Super::EndPlay(EndPlayReason);
}

void UNeGameplayActorComponent::RegisterToSceneManagement()
{
	if (bNeedRegisterToSceneManagement == false) return;

	if (bRegisteredToSceneManagement) return;
	if (UNeSceneManageSystem* SceneManager = UNeSceneManageSystem::GetInstance(this))
	{
		SceneManager->RegisterActor(GetOwner());
		bRegisteredToSceneManagement = true;
	}
}

void UNeGameplayActorComponent::UnregisterFromSceneManagement()
{
	if (bRegisteredToSceneManagement == false) return;
	if (UNeSceneManageSystem* SceneManager = UNeSceneManageSystem::GetInstance(this))
	{
		SceneManager->UnregisterActor(GetOwner());
	}
	bRegisteredToSceneManagement = false;
}
