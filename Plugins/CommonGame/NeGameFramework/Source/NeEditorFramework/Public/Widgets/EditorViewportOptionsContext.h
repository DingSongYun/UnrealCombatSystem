// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportOptionsContext.generated.h"

UCLASS()
class NEEDITORFRAMEWORK_API UEditorViewportOptionsMenuContext : public UObject
{
	GENERATED_BODY()
public:

	TWeakPtr<const class SEditorViewportOptionsMenu> EditorViewportOptionsMenu;
};
