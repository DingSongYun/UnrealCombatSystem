// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Utilities/NeCurveLibrary.h"

float UNeCurveLibrary::GetFloatValue(const FRuntimeFloatCurve& Curve, float Time)
{
	return Curve.GetRichCurveConst()->Eval(Time);
}
