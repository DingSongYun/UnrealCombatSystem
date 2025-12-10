// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportCameraContext.generated.h"

UCLASS()
class NEEDITORFRAMEWORK_API UEditorViewportCameraMenuContext : public UObject
{
	GENERATED_BODY()
public:

	TWeakPtr<const class SEditorViewportCameraMenu> EditorViewporCameraMenu;
};
