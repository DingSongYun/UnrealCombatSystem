// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NeAbilityCondition.generated.h"

class UNeAbilityBeam;

UENUM(BlueprintType)
enum class ECondLogic : uint8
{
	AND,
	OR,
};

/** Boolean Operators */
UENUM(BlueprintType)
enum class ECondCheckOp : uint8
{
	EQ	= 0	UMETA(DisplayName="=="),
	LT		UMETA(DisplayName="<"),
	LE		UMETA(DisplayName="<="),
	GE		UMETA(DisplayName=">="),
	GT		UMETA(DisplayName=">"),
	NE		UMETA(DisplayName="!="),
};

//=============================================================================
/**
 * UNeAbilityCondition
 * 通用条件
 */
UCLASS(EditInlineNew, Abstract, BlueprintType)
class NEABILITYSYSTEM_API UNeAbilityCondition : public UObject
{
	GENERATED_BODY()

public:
	class UWorld* GetWorld() const override;
	UObject* GetContextObject() const { return ContextObject; }

	UFUNCTION(BlueprintCallable)
	virtual void Initialize(UObject* InContextObject)
	{
		ContextObject = InContextObject;
	}

	UFUNCTION(BlueprintCallable)
	virtual void Terminate() {}

	UFUNCTION(BlueprintCallable)
	bool CheckCondition() const;

	UFUNCTION(BlueprintCallable)
	virtual void ResetState() {}

	UFUNCTION(BlueprintPure, meta=(DisplayName="Outer Character"))
	class ACharacter* GetCharacter() const;

	UFUNCTION(BlueprintPure, meta=(DisplayName="Outer Actor"))
	AActor* GetActor() const;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Task Instance"))
	UNeAbilityBeam* GetOutterBeam() const;

	UFUNCTION(BlueprintPure)
	virtual bool IsOnlyControlPlayerActiveTask() const { return false; }

protected:
	virtual bool PerformCheck() const { return false; }

public:
	// 取 "非"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Condition)
	bool bIsNot = false;

private:
	UPROPERTY(Transient)
	UObject* ContextObject = nullptr;
};

USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityCondGroupItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ECondLogic Logic = ECondLogic::AND;

	UPROPERTY(EditAnywhere, Instanced)
	UNeAbilityCondition* Condition = nullptr;
};

//=============================================================================
/**
 * UNeAbilityConditionGroup
 * 条件组
 */
UCLASS(BlueprintType, DisplayName="条件组")
class NEABILITYSYSTEM_API UNeAbilityConditionGroup : public UNeAbilityCondition
{
	GENERATED_BODY()
public:
	//~BEGIN: UGBSCondition interface
	virtual void Initialize(UObject* WorldContext) override;
	virtual bool PerformCheck() const override;
	virtual void ResetState() override;
	virtual void Terminate() override;
	//~BEGIN: UGBSCondition interface

	void AddCondition(UNeAbilityCondition* NewCond);
	bool RemoveCondition(UNeAbilityCondition* CondToRemove);
	void GetConditions(TArray<FNeAbilityCondGroupItem>& OutConds);
	void ClearConditions();

protected:
	UPROPERTY(EditInstanceOnly, Category=Condition, meta=(DisplayName="条件集", ShowOnlyInnerProperties))
	TArray<FNeAbilityCondGroupItem> CondItems;
};

//=============================================================================
/**
 * UNeAbilityConditionBP
 * Condition for blueprint implement
 * 条件组
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NEABILITYSYSTEM_API UNeAbilityConditionBP : public UNeAbilityCondition
{
	GENERATED_BODY()
public:
	virtual void Initialize(UObject* WorldContext) override
	{
		Super::Initialize(WorldContext);
		ReceiveInitialize();
	}

	virtual bool PerformCheck() const override { return ReceivePerformCheck(); }
	virtual void ResetState() override { ReceiveResetState(); }
	virtual void Terminate() override { ReceiveTerminate(); }

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Initialize"))
	void ReceiveInitialize();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "PerformCheck"))
	bool ReceivePerformCheck() const;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ResetState"))
	void ReceiveResetState();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Terminate"))
	void ReceiveTerminate();
};