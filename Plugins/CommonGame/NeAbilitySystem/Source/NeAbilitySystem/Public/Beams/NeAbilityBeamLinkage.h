// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityBeam.h"
#include "NeAbilityBeamLinkage.generated.h"

/**
 * UNeAbilityBeamLinkage
 *
 * 桥接GAS部件
 */
UCLASS(abstract, HideDropdown)
class NEABILITYSYSTEM_API UNeAbilityBeamLinkage : public UNeAbilityBeam
{
	GENERATED_UCLASS_BODY()
public:
	/** 获取链接的类型 */
	virtual UClass* GetSupportClass() const { return nullptr; }
	virtual void PostCDOContruct() override;
	virtual void InitialLink(UClass* LinkClass);

#if WITH_EDITOR
public:
	struct FBeamLinkageDesc
	{
		UClass* LinkageBeamClass = nullptr;

		FBeamLinkageDesc() : LinkageBeamClass(nullptr) {}

		FBeamLinkageDesc(UClass* InBeamClass) : LinkageBeamClass(InBeamClass)
		{}
	};

	static TArray<UClass*> GetRegisteredLinkTypes();
	static bool IsBeamLinkType(UClass* LinkType);
	static const FBeamLinkageDesc* GetBeamLinkageDesc(UClass* LinkType);
protected:
	static void RegisterLinkType(UNeAbilityBeamLinkage* BeamLinkage);
#endif

public:
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category="Beam|Linkage")
	FSoftClassPath LinkedClass;
};
