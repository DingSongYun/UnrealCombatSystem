// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbility.h"
#include "NeAbilityPreviewInstance.generated.h"

class UNeAbilitySystemComponent;

/** Segment Preview State */
UENUM(BlueprintType)
enum class EAbilitySegmentPreviewState : uint8
{
	Awaiting,
	Running,
	Passed
};

/** FAbilityPreviewSegment */
USTRUCT(BlueprintType)
struct FAbilityPreviewSegment
{
	GENERATED_USTRUCT_BODY()

public:
	bool IsValid() const;
	int32 GetSegmentID() const;
	int32 GetSectionIndex() const;
	const FNeAbilitySegment& GetSegment() const;

public:
	FWeakAbilitySegmentPtr SegmentPtr;

	UPROPERTY(Transient)
	FNeAbilitySegmentEvalContext EvalContext;

	/** Segment 预览状态 */
	UPROPERTY(Transient)
	EAbilitySegmentPreviewState State = EAbilitySegmentPreviewState::Awaiting;
};

/**
 * UNeAbilityPreviewInstance
 *
 * 预览时所用的技能实例
 */
UCLASS()
class UNeAbilityPreviewInstance : public UNeAbility
{
	GENERATED_UCLASS_BODY()
public:
	/** Initialize */
	void InitializeFor(UNeAbilitySystemComponent* AbilityComponent, UNeAbility* Ability, const TSharedPtr<class FNeAbilityBlueprintEditor>& InHostEditor);
	void SetPosition(int32 InSectionIndex, float NewPosition);

	//~BEGIN: UNeAbility interface
	virtual FNeAbilitySection& GetSection(int32 Index) override { return RawAbilityAsset->GetSection(Index); }
	virtual bool IsValidSection(int32 Index) const override { return RawAbilityAsset->IsValidSection(Index); }
	virtual const FNeAbilitySection& GetSection(int32 Index) const override { return RawAbilityAsset->GetSection(Index); }
	virtual void ActivateSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext) override;
	virtual void EndSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, EAbilityBeamEndReason EndReason) override;
	virtual UNeAbilityBeam* CreateBeamEvalInstance(FNeAbilitySegmentEvalContext& SegmentEvalContext, UNeAbilityBeam* ArcheType) override;
	//~END: UNeAbility interface
	void SampleSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, const float Position, const float PreviousPosition);

	/** Find Evaluate context of segment */
	FNeAbilitySegmentEvalContext* GetSegmentPreviewContext(const FWeakAbilitySegmentPtr& InSegmentPtr);

protected:
	/** Build Segment preview infos */
	void BuildPreviewSegments();

	/** Set preview position */
	void SetPosition(int32 InSectionIndex, const float PreviousPosition, const float NewPosition);

	/** 检测在自定位置上Segment的状态 */
	EAbilitySegmentPreviewState TestPosition(const FNeAbilitySegment& Segment, float Position) const;

	void OnAddNewSegment(const FWeakAbilitySegmentPtr& NeAbilitySegment);
	void OnDeleteSegment(const FWeakAbilitySegmentPtr& NeAbilitySegment);
	void OnMoveSegment(const FWeakAbilitySegmentPtr& NeAbilitySegment);
	void OnSegmentPropertyChanged(UObject* EditingObject, const FPropertyChangedEvent& PropertyChangedEvent);

private:
	/** Ability Editor */
	TWeakPtr<class FNeAbilityBlueprintEditor> AbilityEditor;

	UPROPERTY(Transient)
	TObjectPtr<UNeAbility> RawAbilityAsset = nullptr;

	/** 预览用的Segment */
	UPROPERTY(Transient)
	TArray<FAbilityPreviewSegment> PreviewSegments;

	/** 当前预览的位置 */
	UPROPERTY(Transient)
	float CurrentPosition = -1.f;
};