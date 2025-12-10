// Copyright NetEase Games, Inc. All Rights Reserved.

#include "DataBoard/NeAbilityDataBoardValueType.h"

const FNeADBValueType_Int::FDataType FNeADBValueType_Int::DEFAULT_VALUE = FNeADBValueType_Int::FDataType(0);
const FNeADBValueType_Float::FDataType FNeADBValueType_Float::DEFAULT_VALUE = FNeADBValueType_Float::FDataType(0);
const FNeADBValueType_Bool::FDataType FNeADBValueType_Bool::DEFAULT_VALUE = false;
const FNeADBValueType_Vector::FDataType FNeADBValueType_Vector::DEFAULT_VALUE = FVector::ZeroVector;
const FNeADBValueType_LinearColor::FDataType FNeADBValueType_LinearColor::DEFAULT_VALUE = FLinearColor::White;
const FNeADBValueType_Rotator::FDataType FNeADBValueType_Rotator::DEFAULT_VALUE = FRotator::ZeroRotator;
const FNeADBValueType_Struct::FDataType FNeADBValueType_Struct::DEFAULT_VALUE = FNeADBValueType_Struct::FDataType();
const FNeADBValueType_Object::FDataType FNeADBValueType_Object::DEFAULT_VALUE = nullptr;
