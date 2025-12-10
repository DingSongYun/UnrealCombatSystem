// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "NeAbilityWeakPtr.h"
#include "NeAbilitySegmentEditorObject.generated.h"

UCLASS(MinimalAPI)
class UNeAbilitySegmentEditorObject : public UObject
{
	GENERATED_BODY()
public:
	virtual void Initialize(const FNeAbilitySegmentPtr& InSegment)
	{
		SegmentPtr = InSegment;
	}

	FNeAbilitySegmentPtr SegmentPtr;
};
