// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityPropertyBinding.h"

#include "NeAbility.h"
#include "Beams/NeAbilityBeam.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"

bool FNeAbilityPropertyBindingCurve::GetFloatValue(float& Value, float InTime) const
{
	if (const FRuntimeFloatCurve* FloatCurve = Curve.GetPtr<FRuntimeFloatCurve>())
	{
		if (FloatCurve->ExternalCurve)
		{
			Value = FloatCurve->ExternalCurve->GetFloatValue(InTime);
		}

		Value = FloatCurve->EditorCurveData.Eval(InTime);
		return true;
	}

	return false;
}

bool FNeAbilityPropertyBindingCurve::GetIntValue(int64& Value, float InTime) const
{
	float FloatValue;
	if (GetFloatValue(FloatValue, InTime))
	{
		Value = static_cast<int64>(FloatValue);
		return true;
	}

	return false;
}

bool FNeAbilityPropertyBindingCurve::GetVectorValue(FVector& Value, float InTime) const
{
	if (const FRuntimeVectorCurve* VectorCurve = Curve.GetPtr<FRuntimeVectorCurve>())
	{
		Value = VectorCurve->GetValue(InTime);
		return true;
	}

	return false;
}

bool FNeAbilityPropertyBindingCurve::GetRotatorValue(FRotator& Value, float InTime) const
{
	if (const FRuntimeVectorCurve* VectorCurve = Curve.GetPtr<FRuntimeVectorCurve>())
	{
		const FVector VectorValue = VectorCurve->GetValue(InTime);
		Value.Pitch = VectorValue.Y;
		Value.Yaw = VectorValue.Z;
		Value.Roll = VectorValue.X;
		return true;
	}

	return false;
}

bool FNeAbilityPropertyBindingCurve::GetLinearColorValue(FLinearColor& Value, float InTime) const
{
	if (const FRuntimeCurveLinearColor* LinearColorCurve = Curve.GetPtr<FRuntimeCurveLinearColor>())
	{
		Value = LinearColorCurve->GetLinearColorValue(InTime);
		return true;
	}

	return false;
}

void FNeAbilityPropertyBindingCurve::MakeFloatCurve()
{
	Curve.InitializeAs(FRuntimeFloatCurve::StaticStruct());
}

void FNeAbilityPropertyBindingCurve::MakeVectorCurve()
{
	Curve.InitializeAs(FRuntimeVectorCurve::StaticStruct());
}

void FNeAbilityPropertyBindingCurve::MakeColorCurve()
{
	Curve.InitializeAs(FRuntimeCurveLinearColor::StaticStruct());
}

TSharedPtr<FNeAbilityPropertyBindingEval> FNeAbilityPropertyBindingEval::Create(const FNeAbilityPropertyBinding& InBindingData)
{
	TSharedPtr<FNeAbilityPropertyBindingEval> OutEval = nullptr;
	switch (InBindingData.Type)
	{
	case ENeAbilityPropertyBindingType::Property:
		OutEval = MakeShareable(new FNeAbilityPropertyBindingEval_Property());
		break;
	case ENeAbilityPropertyBindingType::Function:
		OutEval = MakeShareable(new FNeAbilityPropertyBindingEval_Function());
		break;
	case ENeAbilityPropertyBindingType::DataBoard:
		OutEval = MakeShareable(new FNeAbilityPropertyBindingEval_DataBoard());
		break;
	case ENeAbilityPropertyBindingType::Curve:
		OutEval = MakeShareable(new FNeAbilityPropertyBindingEval_Curve());
		break;
	default:
		check(0);
	}

	OutEval->BindingData = &InBindingData;
	return OutEval;
}

void FNeAbilityPropertyBindingEval::InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance)
{
	check(BindingData->ObjectName == BeamData->GetName());
	if (BindingData->PropertyPath.IsValid())
	{
		BindingData->PropertyPath.Resolve(BeamInstance);
		DestProperty = BindingData->PropertyPath.GetFProperty();
		DestPropertyAddress = BindingData->PropertyPath.GetCachedAddress();
	}
	else
	{
		DestProperty = BeamData->GetClass()->FindPropertyByName(BindingData->PropertyName);
		DestPropertyAddress = DestProperty->ContainerPtrToValuePtr<void>(BeamInstance);
	}
	ensureMsgf(DestProperty, TEXT("Can not find property %s in class %s, make sure the property is still avaliable."), *BindingData->PropertyName.ToString(), *BeamData->GetClass()->GetName());
}

void FNeAbilityPropertyBindingEval_Property::InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance)
{
	FNeAbilityPropertyBindingEval::InitializePropertyBinding(InAbility, BeamData, BeamInstance);
	BindingData->SourcePropertyPath.Resolve(InAbility);
	SourceProperty = BindingData->SourcePropertyPath.GetFProperty();
	ensureMsgf(SourceProperty, TEXT("Can not find source property %s in class %s, make sure the property is still avaliable."), *BindingData->SourcePropertyPath.ToString(), *BeamData->GetClass()->GetName());
}

void FNeAbilityPropertyBindingEval_Property::EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	if (SourceProperty)
	{
		void* ContainerPtr = SegmentEvalContext.BeamInstance;
		if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(SourceProperty))
		{
			double NumberValue;
			SourceProperty->GetValue_InContainer(InAbility, &NumberValue);
			if (const FIntProperty* IntProperty = CastField<FIntProperty>(DestProperty))
			{
				const int64 IntValue = static_cast<int64>(NumberValue);
				IntProperty->SetValue_InContainer(ContainerPtr, IntValue);
			}
			else if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(DestProperty))
			{
				const float FloatValue = static_cast<float>(NumberValue);
				FloatProperty->SetValue_InContainer(ContainerPtr, FloatValue);
			}
			else if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(DestProperty))
			{
				const uint8 EnumValue = static_cast<uint8>(NumberValue);
				EnumProperty->SetValue_InContainer(ContainerPtr, &EnumValue);
			}
			else
			{
				DestProperty->SetValue_InContainer(ContainerPtr, &NumberValue);
			}
		}
		else
		{
			void const* SourceAddress = SourceProperty->ContainerPtrToValuePtr<void>(InAbility);
			DestProperty->CopyCompleteValue(DestPropertyAddress, SourceAddress);
		}
	}
}

void FNeAbilityPropertyBindingEval_Function::InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance)
{
	FNeAbilityPropertyBindingEval::InitializePropertyBinding(InAbility, BeamData, BeamInstance);
	if (BindingData->FunctionName.IsValid())
	{
		BindingFunction = InAbility->FindFunction(BindingData->FunctionName);
	}

	ensureMsgf(BindingFunction.IsValid(), TEXT("Can not find function %s in class %s, make sure the function is still avaliable."), *BindingData->FunctionName.ToString(), *BeamData->GetClass()->GetName());
}

void FNeAbilityPropertyBindingEval_Function::EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	if (BindingFunction.IsValid())
	{
		uint8* Params = (uint8*)FMemory_Alloca_Aligned(BindingFunction.Get()->ParmsSize, BindingFunction.Get()->GetMinAlignment());
		FMemory::Memzero( Params, BindingFunction.Get()->ParmsSize );

		InAbility->ProcessEvent(BindingFunction.Get(), Params);

		void* ContainerPtr = SegmentEvalContext.BeamInstance;
		for( TFieldIterator<FProperty> It(BindingFunction.Get()); It && It->HasAnyPropertyFlags(CPF_Parm); ++It )
		{
			if (It->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm))
			{
				if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(*It))
				{
					double NumberValue;
					It->GetValue_InContainer(Params, &NumberValue);
					if (const FIntProperty* IntProperty = CastField<FIntProperty>(DestProperty))
					{
						const int64 IntValue = static_cast<int64>(NumberValue);
						IntProperty->SetValue_InContainer(ContainerPtr, IntValue);
					}
					else if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(DestProperty))
					{
						const float FloatValue = static_cast<float>(NumberValue);
						FloatProperty->SetValue_InContainer(ContainerPtr, FloatValue);
					}
					else if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(DestProperty))
					{
						const uint8 EnumValue = static_cast<uint8>(NumberValue);
						EnumProperty->SetValue_InContainer(ContainerPtr, &EnumValue);
					}
					else
					{
						DestProperty->SetValue_InContainer(ContainerPtr, &NumberValue);
					}
				}
				else
				{
					void const* SourceAddress = It->ContainerPtrToValuePtr<void>(Params);
					DestProperty->CopyCompleteValue(DestPropertyAddress, SourceAddress);
				}
			}
			It->DestroyValue_InContainer(Params);
		}
	}
}

void FNeAbilityPropertyBindingEval_DataBoard::InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance)
{
	FNeAbilityPropertyBindingEval::InitializePropertyBinding(InAbility, BeamData, BeamInstance);
}

void FNeAbilityPropertyBindingEval_DataBoard::EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	if (BindingData->DataBoardEntry.IsValid() == false) return;

	if (DestProperty == nullptr) return ;

	void* ContainerPtr = SegmentEvalContext.BeamInstance;

	if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(DestProperty))
	{
		if (float Value; InAbility->GetDataFloat(BindingData->DataBoardEntry, Value) == EAbilityDataAccessResult::Success)
		{
			FloatProperty->SetValue_InContainer(ContainerPtr, Value);
		}
	}
	else if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(DestProperty))
	{
		if (double Value; InAbility->GetDataDouble(BindingData->DataBoardEntry, Value) == EAbilityDataAccessResult::Success)
		{
			DoubleProperty->SetValue_InContainer(ContainerPtr, Value);
		}
	}
	else if (FIntProperty* IntProperty = CastField<FIntProperty>(DestProperty))
	{
		if (int64 Value; InAbility->GetDataInteger(BindingData->DataBoardEntry, Value) == EAbilityDataAccessResult::Success)
		{
			IntProperty->SetValue_InContainer(ContainerPtr, Value);
		}
	}
	else if (const FStructProperty* StructProperty = CastField<FStructProperty>(DestProperty))
	{
		const FName StructName = StructProperty->Struct->GetFName();

		if (StructName == NAME_Vector ||
			StructName == NAME_Vector2D ||
			StructName == NAME_Vector4 ||
			StructName == NAME_Quat)
		{
			if (FVector Value; InAbility->GetDataVector(BindingData->DataBoardEntry, Value) == EAbilityDataAccessResult::Success)
			{
				StructProperty->SetValue_InContainer(ContainerPtr, &Value);
			}
		}
		else if (StructName == NAME_Rotation)
		{
			if (FRotator Value; InAbility->GetDataRotator(BindingData->DataBoardEntry, Value) == EAbilityDataAccessResult::Success)
			{
				StructProperty->SetValue_InContainer(ContainerPtr, &Value);
			}
		}
		else if (StructName == NAME_LinearColor)
		{
			if (FLinearColor Value; InAbility->GetDataLinearColor(BindingData->DataBoardEntry, Value) == EAbilityDataAccessResult::Success)
			{
				StructProperty->SetValue_InContainer(ContainerPtr, &Value);
			}
		}
		else
		{
			if (FInstancedStruct Value; InAbility->GetDataStruct(BindingData->DataBoardEntry, Value) == EAbilityDataAccessResult::Success)
			{
				if (StructProperty->Struct == Value.GetScriptStruct())
				{
					StructProperty->SetValue_InContainer(ContainerPtr, Value.GetMemory());
				}
			}
		}
	}
}

void FNeAbilityPropertyBindingEval_Curve::InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance)
{
	FNeAbilityPropertyBindingEval::InitializePropertyBinding(InAbility, BeamData, BeamInstance);
}

void FNeAbilityPropertyBindingEval_Curve::EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	if (BindingData->CurveData.IsValid() == false) return ;

	const float Time = BindingData->CurveData.bRemapping ?  GetRemappedTime(SegmentEvalContext) : SegmentEvalContext.BeamInstance->GetRunningTime();

	if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(DestProperty))
	{
		if (float Value; BindingData->CurveData.GetFloatValue(Value, Time))
		{
			FloatProperty->SetValue_InContainer(SegmentEvalContext.BeamInstance, Value);
		}
	}
	else if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(DestProperty))
	{
		if (float Value; BindingData->CurveData.GetFloatValue(Value, Time))
		{
			DoubleProperty->SetValue_InContainer(SegmentEvalContext.BeamInstance, Value);
		}
	}
	else if (FIntProperty* IntProperty = CastField<FIntProperty>(DestProperty))
	{
		if (int64 Value; BindingData->CurveData.GetIntValue(Value, Time))
		{
			IntProperty->SetValue_InContainer(SegmentEvalContext.BeamInstance, Value);
		}
	}
	else if (const FStructProperty* StructProperty = CastField<FStructProperty>(DestProperty))
	{
		const FName StructName = StructProperty->Struct->GetFName();

		if (StructName == NAME_Vector ||
			StructName == NAME_Vector2D ||
			StructName == NAME_Vector4 ||
			StructName == NAME_Quat)
		{
			if (FVector Value; BindingData->CurveData.GetVectorValue(Value, Time))
			{
				StructProperty->SetValue_InContainer(SegmentEvalContext.BeamInstance, &Value);
			}
		}
		else if (StructName == NAME_Rotation)
		{
			if (FRotator Value; BindingData->CurveData.GetRotatorValue(Value, Time))
			{
				StructProperty->SetValue_InContainer(SegmentEvalContext.BeamInstance, &Value);
			}
		}
		else if (StructName == NAME_LinearColor)
		{
			if (FLinearColor Value; BindingData->CurveData.GetLinearColorValue(Value, Time))
			{
				StructProperty->SetValue_InContainer(SegmentEvalContext.BeamInstance, &Value);
			}
		}
		else
		{
			// Not supported
			check(0);
		}
	}
}

float FNeAbilityPropertyBindingEval_Curve::GetRemappedTime(FNeAbilitySegmentEvalContext& SegmentEvalContext) const
{
	return 0.f;
}
