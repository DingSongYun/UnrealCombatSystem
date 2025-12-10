// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/StructOnScope.h"
#include "NeAbilityDataBoardValueType.generated.h"

/**
 * FNeAbilityDataBoardValueType
 * 技能数据板支持类型
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	virtual ~FNeAbilityDataBoardValueType() {}

	/**
	 * 用于序列化时获取类型，子类必须实现
	 */
	virtual UScriptStruct* GetScriptStruct() const
	{
		return FNeAbilityDataBoardValueType::StaticStruct();
	}

};

USTRUCT(BlueprintType, meta=(Hidden))
struct NEABILITYSYSTEM_API FNeADBValueType_Numeric : public FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	virtual double GetValue() const { return 0.f; }

	virtual UScriptStruct* GetScriptStruct() const
	{
		return FNeADBValueType_Numeric::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="Int"))
struct NEABILITYSYSTEM_API FNeADBValueType_Int : public FNeADBValueType_Numeric
{
	GENERATED_USTRUCT_BODY()

	typedef int64 FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	int64 Value = 0;

public:
	virtual double GetValue() const override { return Value; }
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_Int::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="Float"))
struct NEABILITYSYSTEM_API FNeADBValueType_Float : public FNeADBValueType_Numeric
{
	GENERATED_USTRUCT_BODY()

	typedef double FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	double Value = 0.f;

public:
	virtual double GetValue() const override { return Value; }

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_Float::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="Bool"))
struct NEABILITYSYSTEM_API FNeADBValueType_Bool : public FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	typedef bool FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	bool Value = false;

public:
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_Bool::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="Vector"))
struct NEABILITYSYSTEM_API FNeADBValueType_Vector : public FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	typedef FVector FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	FVector Value = FVector::ZeroVector;

public:
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_Vector::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="LinearColor"))
struct NEABILITYSYSTEM_API FNeADBValueType_LinearColor : public FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	typedef FLinearColor FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	FLinearColor Value = FLinearColor::White;

public:
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_LinearColor::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="Rotator"))
struct NEABILITYSYSTEM_API FNeADBValueType_Rotator : public FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	typedef FRotator FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	FRotator Value = FRotator::ZeroRotator;

public:
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_Rotator::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="Struct"))
struct NEABILITYSYSTEM_API FNeADBValueType_Struct : public FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	typedef FInstancedStruct FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	FInstancedStruct Value;

public:

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_Struct::StaticStruct();
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="Object"))
struct NEABILITYSYSTEM_API FNeADBValueType_Object : public FNeAbilityDataBoardValueType
{
	GENERATED_USTRUCT_BODY()

	typedef UObject* FDataType;
	static const FDataType DEFAULT_VALUE;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UObject> Value = nullptr;

public:
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNeADBValueType_Object::StaticStruct();
	}
};