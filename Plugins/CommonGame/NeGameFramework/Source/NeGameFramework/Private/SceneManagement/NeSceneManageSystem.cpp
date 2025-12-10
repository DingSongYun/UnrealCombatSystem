// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SceneManagement/NeSceneManageSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "GameMode/IWorldSubsystemAssistInterface.h"
#include "SceneManagement/NeGameplayActorCollection.h"

UNeSceneManageSystem* UNeSceneManageSystem::GetInstance(const UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr) return nullptr;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		return UWorld::GetSubsystem<UNeSceneManageSystem>(World);
	}

	return nullptr;
}

void UNeSceneManageSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UWorld* World = GetWorld();
	check(World);

	const FOnActorSpawned::FDelegate ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &UNeSceneManageSystem::OnActorSpawned);
	ActorSpawnedDelegateHandle = World->AddOnActorSpawnedHandler(ActorSpawnedDelegate);

	// Create Actor Collection
	ActorCollection = NewObject<UNeGameplayActorCollection>(this, UNeGameplayActorCollectionGeneric::StaticClass());

	ReceiveInitialize();
}

void UNeSceneManageSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	ReceiveOnWorldBeginPlay();
}

void UNeSceneManageSystem::UpdateStreamingState()
{
	ReceiveUpdateStreamingState();
}

void UNeSceneManageSystem::OnWorldComponentsUpdated(UWorld& World)
{
	Super::OnWorldComponentsUpdated(World);
	ReceiveOnWorldComponentsUpdated(&World);
}

void UNeSceneManageSystem::Deinitialize()
{
	Super::Deinitialize();
	ReceiveDeinitialize();
}

void UNeSceneManageSystem::Tick(float DeltaTime)
{
	if (bSceneReady == false)
	{
		CheckSceneReady();
	}
}

bool UNeSceneManageSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (UWorld* World = Cast<UWorld>(Outer))
	{
		if (!World->IsGameWorld())
		{
			const UClass* DefaultGameModeClass = World->GetWorldSettings()->DefaultGameMode.Get();
			if (DefaultGameModeClass)
			{
				AGameModeBase* DefaultGameMode = DefaultGameModeClass->GetDefaultObject<AGameModeBase>();
				const IWorldSubsystemAssistInterface* SubsystemAssist = Cast<IWorldSubsystemAssistInterface>(DefaultGameMode);
				if (SubsystemAssist)
				{
					return SubsystemAssist->SupportSubsystem(GetClass());
				}
			}
			return false;
		}
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UNeSceneManageSystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::EditorPreview;
}

FVector UNeSceneManageSystem::GetActorGravityDirection(AActor* InActor) const
{
	check(IsValid(InActor));
	FVector GravDir = ReceiveGetActorGravityDirection(InActor);

	if (GravDir.IsZero())
	{
		GravDir = GetGravityDirection(InActor->GetActorLocation());
	}

	return GravDir;
}

FVector UNeSceneManageSystem::GetGravityDirection(const FVector& InLocation) const
{
	FVector GravDir = ReceiveGetGravityDirection(InLocation);

	if (GravDir.IsZero())
	{
		GravDir = FVector::UpVector;
	}

	check(GravDir.IsNormalized());

	return GravDir;
}

void UNeSceneManageSystem::CheckSceneReady()
{
	bSceneReady = true;

	ReceiveCheckSceneReady();

	// Notify when scene ready
	if (bSceneReady)
	{
		OnSceneReady.Broadcast();
	}
}

void UNeSceneManageSystem::OnActorSpawned(AActor* SpawnedActor)
{
	if (!GIsReinstancing)
	{
		ReceiveOnActorSpawned(SpawnedActor);
	}
}

void UNeSceneManageSystem::RegisterActor(AActor* InActor)
{
	check(ActorCollection);
	ActorCollection->Add(InActor);
}

void UNeSceneManageSystem::UnregisterActor(AActor* InActor)
{
	check(ActorCollection);
	ActorCollection->Remove(InActor);
}

bool UNeSceneManageSystem::FindSceneActorOfClass(UClass* ActorClass, TArray<AActor*>& OutActors) const
{
	check(ActorCollection);
	return ActorCollection->FindSceneActorOfClass(ActorClass, OutActors);
}

