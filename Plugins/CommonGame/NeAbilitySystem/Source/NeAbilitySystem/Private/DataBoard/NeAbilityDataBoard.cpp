// Copyright NetEase Games, Inc. All Rights Reserved.

#include "DataBoard/NeAbilityDataBoard.h"

#include "AbilitySystemLog.h"
#include "Engine/NetConnection.h"
#include "Engine/PackageMapClient.h"
#include "Engine/HitResult.h"

const FNeAbilityDataBoardKey FNeAbilityDataBoardKey::Invalid = FNeAbilityDataBoardKey();

struct FNeAbilityDataBoardValueDeleter
{
	FORCEINLINE void operator()(FNeAbilityDataBoardValueType* Object) const
	{
		check(Object);
		UScriptStruct* ScriptStruct = Object->GetScriptStruct();
		check(ScriptStruct);
		ScriptStruct->DestroyStruct(Object);
		FMemory::Free(Object);
	}
};

bool FNeAbilityDataBoard::Serialize(FArchive& Ar)
{
	// Ar << Datas;
#if false
	TArray<FNeAbilityDataBoardKey> DataKeys;

	if (Ar.IsSaving())
	{
		Datas.GetKeys(DataKeys);
	}
	Ar << DataKeys;

	for (const FNeAbilityDataBoardKey& Key : DataKeys)
	{
		TSharedPtr<FNeAbilityDataBoardValueType>& Data = Ar.IsLoading() ? Datas.Add(Key) : *Datas.Find(Key);
		TCheckedObjPtr<UScriptStruct> ScriptStruct = Data.IsValid() ? Data->GetScriptStruct() : nullptr;

		Ar << ScriptStruct;

		if (ScriptStruct.IsValid())
		{
			if (Ar.IsLoading())
			{
				check(!Data.IsValid());

				FNeAbilityDataBoardValueType* NewData = static_cast<FNeAbilityDataBoardValueType*>(FMemory::Malloc(ScriptStruct->GetStructureSize()));
				ScriptStruct->InitializeStruct(NewData);
				Data = TSharedPtr<FNeAbilityDataBoardValueType>(NewData, FNeAbilityDataBoardValueDeleter());
			}

			void* ContainerPtr = Data.Get();
			
			ScriptStruct->SerializeItem(Ar, ContainerPtr, /* Defaults */ nullptr);
		}
		else if (ScriptStruct.IsError())
		{
			ABILITY_LOG(Error, TEXT("FNeAbilityDataBoard::Serialize: Bad ScriptStruct serialized, can't recover."));
			Ar.SetError();
		}

		if (Ar.IsError())
		{
			break;
		}
	}

	if (Ar.IsError())
	{
		// Erase the error data
		for (auto It = Datas.CreateIterator(); It; ++It)
		{
			if (It->Value.IsValid() == false)
			{
				It.RemoveCurrent();
			}
		}
		return false;
	}
#endif

	return true;
}

bool FNeAbilityDataBoard::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// Ar << Datas;
#if false
	TArray<FNeAbilityDataBoardKey> DataKeys;

	if (Ar.IsSaving())
	{
		Datas.GetKeys(DataKeys);
	}
	Ar << DataKeys;

	for (const FNeAbilityDataBoardKey& Key : DataKeys)
	{
		TSharedPtr<FNeAbilityDataBoardValueType>& Data = Ar.IsLoading() ? Datas.Add(Key) : *Datas.Find(Key);
		TCheckedObjPtr<UScriptStruct> ScriptStruct = Data.IsValid() ? Data->GetScriptStruct() : nullptr;

		Ar << ScriptStruct;

		if (ScriptStruct.IsValid())
		{
			if (Ar.IsLoading())
			{
				check(!Data.IsValid());

				FNeAbilityDataBoardValueType* NewData = static_cast<FNeAbilityDataBoardValueType*>(FMemory::Malloc(ScriptStruct->GetStructureSize()));
				ScriptStruct->InitializeStruct(NewData);
				Data = TSharedPtr<FNeAbilityDataBoardValueType>(NewData, FNeAbilityDataBoardValueDeleter());
			}

			void* ContainerPtr = Data.Get();

			if (ScriptStruct->StructFlags & STRUCT_NetSerializeNative)
			{
				ScriptStruct->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, ContainerPtr);
			}
			else
			{
				// 这里我们借鉴了两版对于非Native Struct的网络序列化
				// 第一版是 GAS中 FGameplayAbilityTargetDataHandle 的网络序列化
				// 第二版 是 InstancedStruct 的网络序列化
				// 这里我们采样 第二版
#if false
				// This won't work since FStructProperty::NetSerializeItem is deprecrated.
				//	1) we have to manually crawl through the topmost struct's fields since we don't have a FStructProperty for it (just the UScriptProperty)
				//	2) if there are any UStructProperties in the topmost struct's fields, we will assert in FStructProperty::NetSerializeItem.

				ABILITY_LOG(Fatal, TEXT("FGameplayAbilityTargetDataHandle::NetSerialize called on data struct %s without a native NetSerialize"), *ScriptStruct->GetName());

				for (TFieldIterator<FProperty> It(ScriptStruct.Get()); It; ++It)
				{
					if (It->PropertyFlags & CPF_RepSkip)
					{
						continue;
					}

					void* PropertyData = It->ContainerPtrToValuePtr<void*>(ContainerPtr);

					It->NetSerializeItem(Ar, Map, PropertyData);
				}
#endif

				UPackageMapClient* MapClient = Cast<UPackageMapClient>(Map);
				check(::IsValid(MapClient));

				UNetConnection* NetConnection = MapClient->GetConnection();
				check(::IsValid(NetConnection));
				check(::IsValid(NetConnection->GetDriver()));

				auto& NonConstStruct = ScriptStruct.Get();
				const TSharedPtr<FRepLayout> RepLayout = NetConnection->GetDriver()->GetStructRepLayout(NonConstStruct);
				check(RepLayout.IsValid());

				bool bHasUnmapped = false;
				RepLayout->SerializePropertiesForStruct(NonConstStruct, static_cast<FBitArchive&>(Ar), Map, ContainerPtr, bHasUnmapped);
				bOutSuccess = true;

			}
		}
		else if (ScriptStruct.IsError())
		{
			ABILITY_LOG(Error, TEXT("FNeAbilityDataBoard::Serialize: Bad ScriptStruct serialized, can't recover."));
			Ar.SetError();
		}

		if (Ar.IsError())
		{
			break;
		}
	}

	if (Ar.IsError())
	{
		// Erase the error data
		for (auto It = Datas.CreateIterator(); It; ++It)
		{
			if (It->Value.IsValid() == false)
			{
				It.RemoveCurrent();
			}
		}
		bOutSuccess = false;
		return false;
	}
#endif

	bOutSuccess = true;
	return true;
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, int64& OutValue) const
{
	double DoubleValue;
	const EAbilityDataAccessResult Result = GetValue(Key, DoubleValue);
	OutValue = DoubleValue;
	return Result;
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, float& OutValue) const
{
	double DoubleValue;
	const EAbilityDataAccessResult Result = GetValue(Key, DoubleValue);
	OutValue = DoubleValue;
	return Result;
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, double& OutValue) const
{
	if (const FNeAbilityDataBoardValue* BoardValue = Datas.Find(Key))
	{
		if (const FNeADBValueType_Numeric* ValuePtr = BoardValue->Get<FNeADBValueType_Numeric>())
		{
			OutValue = ValuePtr->GetValue();
			return EAbilityDataAccessResult::Success;
		}

		return EAbilityDataAccessResult::TypeMismatch;
	}

	return EAbilityDataAccessResult::NotFound;
	// return GetValue<FNeADBValueType_Float>(Key, OutValue);
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, bool& OutValue) const
{
	return GetValue<FNeADBValueType_Bool>(Key, OutValue);
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, FVector& OutValue) const
{
	return GetValue<FNeADBValueType_Vector>(Key, OutValue);
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, FLinearColor& OutValue) const
{
	return GetValue<FNeADBValueType_LinearColor>(Key, OutValue);
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, FRotator& OutValue) const
{
	return GetValue<FNeADBValueType_Rotator>(Key, OutValue);
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, UObject*& OutValue) const
{
	return GetValue<FNeADBValueType_Object>(Key, OutValue);
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValue(const FNeAbilityDataBoardKey& Key, FHitResult& OutValue) const
{
	if (const FNeAbilityDataBoardValue* BoardValue = Datas.Find(Key))
	{
		if (const FNeADBValueType_Struct* ValuePtr = BoardValue->Get<FNeADBValueType_Struct>())
		{
			if (const FHitResult* HitResult = ValuePtr->Value.GetPtr<FHitResult>())
			{
				OutValue = *HitResult;
				return EAbilityDataAccessResult::Success;
			}
			return EAbilityDataAccessResult::TypeMismatch;
		}

		return EAbilityDataAccessResult::TypeMismatch;
	}

	return EAbilityDataAccessResult::NotFound;
}

EAbilityDataAccessResult FNeAbilityDataBoard::GetValueStruct(const FNeAbilityDataBoardKey& Key, FInstancedStruct& OutValue) const
{
	return GetValue<FNeADBValueType_Struct>(Key, OutValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, float InValue)
{
	SetValue<FNeADBValueType_Float>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, int64 InValue)
{
	SetValue<FNeADBValueType_Int>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, double InValue)
{
	SetValue<FNeADBValueType_Float>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, bool InValue)
{
	SetValue<FNeADBValueType_Bool>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, const FVector& InValue)
{
	SetValue<FNeADBValueType_Vector>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, const FLinearColor& InValue)
{
	SetValue<FNeADBValueType_LinearColor>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, const FRotator& InValue)
{
	SetValue<FNeADBValueType_Rotator>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, UObject* InValue)
{
	SetValue<FNeADBValueType_Object>(Key, InValue);
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, const FHitResult& InValue)
{
	SetValue(Key, InValue.StaticStruct(), (const uint8*)(&InValue));
}

void FNeAbilityDataBoard::SetValue(const FNeAbilityDataBoardKey& Key, const UScriptStruct* StructType, const uint8* StructData)
{
	FNeADBValueType_Struct* ValuePtr = nullptr;
	if (FNeAbilityDataBoardValue* BoardValue = Datas.Find(Key))
	{
		ValuePtr = BoardValue->Get<FNeADBValueType_Struct>();
	}
	if (ValuePtr == nullptr)
	{
		FNeAbilityDataBoardValue& NewBoardValue = Datas.Add(Key);
		NewBoardValue.Make<FNeADBValueType_Struct>();
		ValuePtr = NewBoardValue.Get<FNeADBValueType_Struct>();
	}
	check(ValuePtr);
	ValuePtr->Value.InitializeAs(StructType, StructData);
}
