// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NeAbilityDataBoardValueType.h"
#include "NeAbilityDataBoard.generated.h"

struct FInstancedStruct;

UENUM(BlueprintType)
enum class EAbilityDataAccessResult : uint8
{
	/** Successfully set the got the data */
	Success,
	/** Not found data on the entry*/
	NotFound,
	/** Required type is mismatched with data on board */
	TypeMismatch,
	/** Error to set or get the data */
	Error,
};

/**
 * FNeAbilityDataBoardKey
 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityDataBoardKey
{
	GENERATED_USTRUCT_BODY()

	static const FNeAbilityDataBoardKey Invalid;

	FNeAbilityDataBoardKey() {}
	FNeAbilityDataBoardKey(const FGameplayTag& InEntryName) : EntryName(InEntryName)
	{}

	UPROPERTY(EditAnywhere, Category=DataBoard, meta=(Categories = "GameplayAbility.Data"))
	FGameplayTag EntryName = FGameplayTag::EmptyTag;

	bool IsValid() const
	{
		return EntryName.IsValid();
	}

	FORCEINLINE friend uint32 GetTypeHash(const FNeAbilityDataBoardKey& Key)
	{
		return GetTypeHash(Key.EntryName);
	}

	friend bool operator==(const FNeAbilityDataBoardKey& Lhs, const FNeAbilityDataBoardKey& Rhs)
	{
		return Lhs.EntryName == Rhs.EntryName;
	}

	friend FArchive& operator <<(FArchive& Ar, FNeAbilityDataBoardKey& Key)
	{
		Ar << Key.EntryName;

		return Ar;
	}
};

USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityDataBoardValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="DataBoard", meta=(BaseStruct="/Script/NeAbilitySystem.NeAbilityDataBoardValueType", ExcludeBaseStruct=true))
	FInstancedStruct Data;
	// TSharedPtr<FNeAbilityDataBoardValueType> Data;

	template<typename T>
	FORCEINLINE T* Get()
	{
		return Data.GetMutablePtr<T>();
		// return StaticCastSharedPtr<ValueType>(Data).Get();
	}

	template<typename T>
	FORCEINLINE const T* Get() const
	{
		return Data.GetPtr<T>();
		// return StaticCastSharedPtr<ValueType>(Data).Get();
	}

	template<typename T> void Make()
	{
		// Data = MakeShareable(new ValueType);
		Data.InitializeAs<T>();
	}

	bool IsValid() const { return Data.IsValid(); }
};

/** 技能数据板定义 */
USTRUCT(BlueprintType)
struct NEABILITYSYSTEM_API FNeAbilityDataBoard
{
	GENERATED_BODY()

	/** Optimized serialize function */
	bool Serialize(FArchive& Ar);
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, float& OutValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, int64& IntValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, double& OutValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, bool& OutValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, FVector& OutValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, FLinearColor& OutValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, FRotator& OutValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, UObject*& OutValue) const;
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, struct FHitResult& OutValue) const;
	EAbilityDataAccessResult GetValueStruct(const FNeAbilityDataBoardKey& Key, FInstancedStruct& OutValue) const;
	void SetValue(const FNeAbilityDataBoardKey& Key, float InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, int64 IntValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, double InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, bool InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, const FVector& InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, const FLinearColor& InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, const FRotator& InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, UObject* InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, const FHitResult& InValue);
	void SetValue(const FNeAbilityDataBoardKey& Key, const UScriptStruct* StructType, const uint8* StructData);

	template<typename ValueType>
	EAbilityDataAccessResult GetValue(const FNeAbilityDataBoardKey& Key, typename ValueType::FDataType& OutValue) const
	{
		if (const FNeAbilityDataBoardValue* BoardValue = Datas.Find(Key))
		{
			if (const ValueType* ValuePtr = BoardValue->Get<ValueType>())
			{
				OutValue = ValuePtr->Value;
				return EAbilityDataAccessResult::Success;
			}

			return EAbilityDataAccessResult::TypeMismatch;
		}

		return EAbilityDataAccessResult::NotFound;
	}

	template<typename ValueType>
	void SetValue(const FNeAbilityDataBoardKey& Key, const typename ValueType::FDataType& InValue)
	{
		ValueType* ValuePtr = nullptr;
		if (FNeAbilityDataBoardValue* BoardValue = Datas.Find(Key))
		{
			ValuePtr = BoardValue->Get<ValueType>();
		}
		if (ValuePtr == nullptr)
		{
			FNeAbilityDataBoardValue& NewBoardValue = Datas.Add(Key);
			NewBoardValue.Make<ValueType>();
			ValuePtr = NewBoardValue.Get<ValueType>();
		}
		check(ValuePtr);
		ValuePtr->Value = InValue;
	}

protected:
	friend class SNeAbilityEditorTab_DataBoard;

	// TMap<FNeAbilityDataBoardKey, TSharedPtr<FNeAbilityDataBoardValueType>> Datas;

	UPROPERTY(EditAnywhere, Category="DataBoard")
	TMap<FNeAbilityDataBoardKey, FNeAbilityDataBoardValue> Datas;
};

template<>
struct TStructOpsTypeTraits<FNeAbilityDataBoard> : public TStructOpsTypeTraitsBase2<FNeAbilityDataBoard>
{
	enum
	{
		WithSerializer = false,
		WithNetSerializer = false,
	};
};
