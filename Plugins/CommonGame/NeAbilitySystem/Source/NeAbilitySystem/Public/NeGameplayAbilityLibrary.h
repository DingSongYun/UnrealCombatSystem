// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataBoard/NeAbilityDataBoard.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NeGameplayAbilityLibrary.generated.h"

struct FNeAbilityTargetData_Generic;
struct FGameplayAbilitySpec;

/**
 * UNeGameplayAbilityLibrary
 *
 * 开放给蓝图/python的方法库
 */
UCLASS()
class NEABILITYSYSTEM_API UNeGameplayAbilityLibrary : public UBlueprintFunctionLibrary
// class NEABILITYSYSTEM_API UNeGameplayAbilityLibrary : public UAbilitySystemBlueprintLibrary
{
	GENERATED_BODY()

public:
	/** 从GameplayTask中检索Factory Method */
	UFUNCTION(BlueprintCallable)
	static bool IsTaskFactoryMethod(const UFunction* Function, const UClass* InTargetType);

	/** 从Actor身上搜索指定名字的Component */
	UFUNCTION(BlueprintCallable)
	static USceneComponent* GetComponentByNameOrTag(const FName& NameOrTag, AActor* InActor);

	/**
	 * 从Actor身上搜索定义了Socket的Component, 这个过程会遍历所有的Component
	 *
	 * @param SocketName	SocketName
	 * @param InActor		Actor
	 * @return				含有Socket的Component或者nullptr
	 */
	static USceneComponent* GetComponentOfSocket(const FName& SocketName, AActor* InActor);

	/** Make gameplay ability spec*/
	UFUNCTION(BlueprintCallable)
	static FGameplayAbilitySpec Make_GameplayAbilitySpec(TSubclassOf<UGameplayAbility> InAbilityClass, int32 InLevel = 1);

	//~=============================================================================
	// FNeAbilityLocatingData
	// 这里提供关于FNeAbilityLocatingData脚本使用的接口
	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetWorldTransform", DefaultToSelf = InData))
	static FTransform AbilityLocatingData_GetWorldTransform(const FNeAbilityLocatingData& InData);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "BuildFromBeam", DefaultToSelf = InData))
	static void AbilityLocatingData_BuildFromBeam(const FNeAbilityLocatingData& InData, const UNeAbilityBeam* InBeam);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "UpdateTarget", DefaultToSelf = InData))
	static void AbilityLocatingData_UpdateTarget(const FNeAbilityLocatingData& InData, const FNeAbilityTargetingInfo& InTarget);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "UpdateTargetActor", DefaultToSelf = InData))
	static const FNeAbilityLocatingData& AbilityLocatingData_UpdateTargetActor(const FNeAbilityLocatingData& InData, AActor* InTargetActor);

	//~=============================================================================
	// Data Board 相关接口

	UFUNCTION(BlueprintPure, Category="Data Board", meta = (DisplayName = "IsValid", DefaultToSelf = InData))
	static bool AbilityDataBoardKey_IsValid(const FNeAbilityDataBoardKey& InKey) { return InKey.IsValid(); }

	//~=============================================================================
	// TargetData 相关接口

	UFUNCTION(BlueprintPure, CustomThunk, Category = "Ability|TargetData", meta=(CustomStructureParam = "Value"))
    static FGameplayAbilityTargetDataHandle	AbilityTargetDataFromStruct(const int32& Value);

	UFUNCTION(BlueprintPure, CustomThunk, Category = "Ability|TargetData", meta = (CustomStructureParam = "Value"))
	static void GetStructFromTargetData(UPARAM(Ref) const FGameplayAbilityTargetDataHandle& TargetDataHandle, int32& Value);

	static FNeAbilityTargetData_Generic* MakeGenericStructTargetData(const UScriptStruct* InScriptStruct, const uint8* InStructMemory);
	static void GetGenericStructFromTargetData(const FNeAbilityTargetData_Generic& InTargetData, void*& OutStructMemory);

	UFUNCTION(BlueprintPure, Category = "Ability|Attribute")
	static FGameplayAttribute MakeGameplayAttribute(TSubclassOf<UAttributeSet> AttributeClass, const FName& AttributeName);

private:
	DECLARE_FUNCTION(execAbilityTargetDataFromStruct);
	DECLARE_FUNCTION(execGetStructFromTargetData);
};
