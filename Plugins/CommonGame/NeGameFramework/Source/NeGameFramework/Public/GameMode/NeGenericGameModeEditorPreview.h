// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IWorldSubsystemAssistInterface.h"
#include "GameFramework/GameMode.h"
#include "NeGenericGameModeEditorPreview.generated.h"

/**
 * AGenericGameModeEditorPreview
 *
 * 一个通用的给游戏编辑器中预览场景使用的GameMode
 * 提供一些功能
 *	1. 更精细的控制那些Subsystem在预览场景中启用
 */
UCLASS()
class NEGAMEFRAMEWORK_API AGenericGameModeEditorPreview : public AGameMode, public IWorldSubsystemAssistInterface
{
	GENERATED_UCLASS_BODY()

public:
	//~BEGIN: IWorldSubsystemAssistInterface interface
	virtual bool SupportSubsystem(TSubclassOf<UWorldSubsystem> SubsystemClass) const override;
	//~END: IWorldSubsystemAssistInterface interface

protected:
	/** EditorPreview下是否启用场景管理 */
	virtual bool SupportSceneManagement() const;

	/** EditorPreview下是否启用UI管理 */
	virtual bool SupportUIManagement() const;
};
