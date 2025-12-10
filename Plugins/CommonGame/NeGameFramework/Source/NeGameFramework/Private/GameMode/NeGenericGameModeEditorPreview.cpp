// Copyright NetEase Games, Inc. All Rights Reserved.

#include "GameMode/NeGenericGameModeEditorPreview.h"

#include "SceneManagement/NeSceneManageSystem.h"
#include "UI/NeUIManagerSubSystem.h"

AGenericGameModeEditorPreview::AGenericGameModeEditorPreview(const FObjectInitializer& Initializer) : Super(Initializer)
{}

bool AGenericGameModeEditorPreview::SupportSubsystem(TSubclassOf<UWorldSubsystem> SubsystemClass) const
{
	if (SubsystemClass->IsChildOf<UNeSceneManageSystem>())
	{
		return SupportSceneManagement();
	}
	else if (SubsystemClass->IsChildOf<UNeUIManagerSubsystem>())
	{
		return SupportUIManagement();
	}

	return false;
}

bool AGenericGameModeEditorPreview::SupportSceneManagement() const
{
	return true;
}

bool AGenericGameModeEditorPreview::SupportUIManagement() const
{
	return false;
}
