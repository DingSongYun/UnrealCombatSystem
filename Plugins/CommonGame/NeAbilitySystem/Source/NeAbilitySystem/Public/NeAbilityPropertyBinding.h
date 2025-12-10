// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "PropertyPathHelpers.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "NeAbilityPropertyBinding.generated.h"

class UNeAbility;
class UNeAbilityBeam;
struct FNeAbilitySegmentEvalContext;

#define Name_PropBindingMeta FName("PropBinding")

UENUM()
enum class ENeAbilityPropertyBindingType : uint8
{
	DataBoard,
	Curve,
	Property,
	Function,
};

USTRUCT(Blueprintable)
struct NEABILITYSYSTEM_API FNeAbilityPropertyBindingCurve
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, meta=(StructTypeConst))
	FInstancedStruct Curve;

	UPROPERTY(EditAnywhere)
	bool bRemapping = true;

	bool GetFloatValue(float& Value, float InTime) const;
	bool GetIntValue(int64& Value, float InTime) const;
	bool GetVectorValue(FVector& Value, float InTime) const;
	bool GetRotatorValue(FRotator& Value, float InTime) const;
	bool GetLinearColorValue(FLinearColor& Value, float InTime) const;
	void MakeFloatCurve();
	void MakeVectorCurve();
	void MakeColorCurve();
	bool IsValid() const { return Curve.IsValid(); }
};

/**
 * FNeAbilityPropertyBinding
 *
 * 存储属性绑定信息
 *
 * TODO: 会编辑时的无用内存冗余，在序列化或者cook时候清理下
 */
USTRUCT(Blueprintable)
struct NEABILITYSYSTEM_API FNeAbilityPropertyBinding
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	ENeAbilityPropertyBindingType Type = ENeAbilityPropertyBindingType::DataBoard;

	UPROPERTY(EditAnywhere)
	FGameplayTag DataBoardEntry = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere)
	FString ObjectName = TEXT("");

	UPROPERTY(EditAnywhere)
	FName PropertyName = NAME_None;

	UPROPERTY(EditAnywhere)
	FName FunctionName = NAME_None;

	UPROPERTY(EditAnywhere)
	FNeAbilityPropertyBindingCurve CurveData;

	/** 进行属性绑定的属性path */
	UPROPERTY(EditAnywhere)
	FCachedPropertyPath PropertyPath;

	/** 来源属性 */
	UPROPERTY(EditAnywhere)
	FCachedPropertyPath SourcePropertyPath;

#if WITH_EDITORONLY_DATA
	/** TODO: 用GUID来处理Function Rename问题，之后实现 */
	UPROPERTY()
	FGuid MemberGuid;
#endif
};

class NEABILITYSYSTEM_API FNeAbilityPropertyBindingEval : TSharedPtr<FNeAbilityPropertyBindingEval>
{
public:
	virtual ~FNeAbilityPropertyBindingEval() {}

	/**
	 * @param InBindingData 属性绑定数据
	 * @return 根据不同的属性绑定类型创建 属性绑定计算器
	 */
	static TSharedPtr<FNeAbilityPropertyBindingEval> Create(const FNeAbilityPropertyBinding& InBindingData);

	/**
	 * 初始化属性绑定数据,
	 * 子类实现这个方法来缓存各自的计算信息
	 *
	 * @param InAbility 技能实例对象
	 * @param BeamData	beam数据对象
	 * @param BeamInstance  beam实例对象
	 */
	virtual void InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance);
	virtual void EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext) {}

	/** 属性绑定信息 */
	const FNeAbilityPropertyBinding* BindingData = nullptr;

	/** 绑定的属性 */
	FProperty* DestProperty = nullptr;
	void* DestPropertyAddress = nullptr;
};

class NEABILITYSYSTEM_API FNeAbilityPropertyBindingEval_Property : public FNeAbilityPropertyBindingEval
{
	virtual void InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance) override;
	virtual void EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext) override;

	FProperty* SourceProperty = nullptr;
};

class NEABILITYSYSTEM_API FNeAbilityPropertyBindingEval_Function : public FNeAbilityPropertyBindingEval
{
	virtual void InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance) override;
	virtual void EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext) override;

	TWeakObjectPtr<UFunction> BindingFunction = nullptr;
};

class NEABILITYSYSTEM_API FNeAbilityPropertyBindingEval_DataBoard : public FNeAbilityPropertyBindingEval
{
	virtual void InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance) override;
	virtual void EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext) override;
};

class NEABILITYSYSTEM_API FNeAbilityPropertyBindingEval_Curve : public FNeAbilityPropertyBindingEval
{
	virtual void InitializePropertyBinding(UNeAbility* InAbility, const UNeAbilityBeam* BeamData, UNeAbilityBeam* BeamInstance) override;
	virtual void EvalProperty(UNeAbility* InAbility, FNeAbilitySegmentEvalContext& SegmentEvalContext) override;
	float GetRemappedTime(FNeAbilitySegmentEvalContext& SegmentEvalContext) const;
};