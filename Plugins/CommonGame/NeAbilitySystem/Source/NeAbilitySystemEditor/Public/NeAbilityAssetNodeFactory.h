// Copyright NetEase Games, Inc. All Rights Reserved.
// 本文件用于处理拖拽资源生成技能节点

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "NeAbilityAssetNodeFactory.generated.h"

struct FNeAbilitySegment;
struct FNeAbilityTrack;
class FNeAbilityTimelineMode;
/**
 * UNeAbilityNodeFactory
 *
 * 处理资源拖拽到技能时间轴上生成技能节点
 */
UCLASS(abstract)
class NEABILITYSYSTEMEDITOR_API UNeAbilityNodeFactory : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	static const UNeAbilityNodeFactory* FindFactory(const FAssetData& AssetData);

	virtual bool CanCreateNodeFrom( const FAssetData& AssetData) const;

	virtual FNeAbilityTrack* CreateNewTrack(UObject* InAsset, FNeAbilityTimelineMode& TimelineMode, const FName& OnGroup) const;
	virtual void PostCreateNewTrack(UObject* InAsset, FNeAbilityTrack& NewTrack, FNeAbilitySegment& NewSegment) const;

public:
	/** 用于创建技能节点类*/
	UPROPERTY(BlueprintReadWrite, Category=Misc)
	UClass*  ActionClass;

private:
	static TArray<TWeakObjectPtr<UClass>> FactoryTypeMap;
};

/**
 * UNeAbilityNodeFactory_Animation
 */
UCLASS()
class NEABILITYSYSTEMEDITOR_API UNeAbilityNodeFactory_Animation : public UNeAbilityNodeFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual bool CanCreateNodeFrom( const FAssetData& AssetData) const override;
	virtual void PostCreateNewTrack(UObject* InAsset, FNeAbilityTrack& NewTrack, FNeAbilitySegment& NewSegment) const override;
};

/**
 * UNeAbilityNodeFactory_Niagara
 */
UCLASS()
class NEABILITYSYSTEMEDITOR_API UNeAbilityNodeFactory_Niagara : public UNeAbilityNodeFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual bool CanCreateNodeFrom( const FAssetData& AssetData) const override;
	virtual void PostCreateNewTrack(UObject* InAsset, FNeAbilityTrack& NewTrack, FNeAbilitySegment& NewSegment) const override;
};
