// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityPreviewInstance.h"

#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityEditorDelegates.h"
#include "NeAbilityPreviewScene.h"
#include "Beams/NeAbilityBeam_Trigger.h"
#include "Misc/NeAbilityGizmoActor.h"
#include "Timeline/NeAbilitySegmentEditorObject.h"

UNeAbilityPreviewInstance::UNeAbilityPreviewInstance(const FObjectInitializer& Initializer) : Super(Initializer)
{
	CurrentPosition = 0.f;
}

bool FAbilityPreviewSegment::IsValid() const
{
	return SegmentPtr.IsValid();
}

int32 FAbilityPreviewSegment::GetSegmentID() const
{
	return SegmentPtr->GetID();
}

int32 FAbilityPreviewSegment::GetSectionIndex() const
{
	return SegmentPtr.GetSectionIndex();
}

const FNeAbilitySegment& FAbilityPreviewSegment::GetSegment() const
{
	return SegmentPtr.Get();
}

void UNeAbilityPreviewInstance::InitializeFor(UNeAbilitySystemComponent* AbilityComponent, UNeAbility* Ability, const TSharedPtr<class FNeAbilityBlueprintEditor>& InHostEditor)
{
	AbilityEditor = InHostEditor;
	RawAbilityAsset = Ability;

	BuildPreviewSegments();

	FNeAbilityEditorDelegates::AddNewSegmentDelegate.AddUObject(this, &UNeAbilityPreviewInstance::OnAddNewSegment);
	FNeAbilityEditorDelegates::PreDeleteSegmentDelegate.AddUObject(this, &UNeAbilityPreviewInstance::OnDeleteSegment);
	FNeAbilityEditorDelegates::MoveSegmentDelegate.AddUObject(this, &UNeAbilityPreviewInstance::OnMoveSegment);
	// FNeAbilityEditorDelegates::OnDetailPropertyChanged.AddUObject(this, &UNeAbilityPreviewInstance::OnSegmentPropertyChanged);
}

void UNeAbilityPreviewInstance::BuildPreviewSegments()
{
	PreviewSegments.Empty();
	const int32 SectionNum = RawAbilityAsset->GetSectionNums();
	for (int32 Index = 0; Index < SectionNum; ++ Index)
	{
		FNeAbilitySection& Section = RawAbilityAsset->GetSection(Index);
		FNeAbilitySectionPtr SectionPtr = MakeWeakSectionPtr(RawAbilityAsset, Index);

		for (const FNeAbilitySegment& Segment : Section.Segments)
		{
			FAbilityPreviewSegment& PreviewSegment = PreviewSegments.AddDefaulted_GetRef();
			PreviewSegment.SegmentPtr = MakeWeakSegmentPtr(SectionPtr, Segment.GetID());
			PreviewSegment.State = EAbilitySegmentPreviewState::Awaiting;
		}
	}

	SetPosition(0, 0.f);
}

void UNeAbilityPreviewInstance::SetPosition(int32 InSectionIndex, float NewPosition)
{
	SetPosition(InSectionIndex, CurrentPosition, NewPosition);
}

void UNeAbilityPreviewInstance::SetPosition(int32 InSectionIndex, const float PreviousPosition, const float NewPosition)
{
	if (PreviousPosition == NewPosition) return ;

	FNeAbilitySection& Section = GetSection(InSectionIndex);

	TArray<FAbilityPreviewSegment*> SampleNodes;
	for (auto It = PreviewSegments.CreateIterator(); It; ++ It)
	{
		FAbilityPreviewSegment& PreviewSegment = *It;
		if (PreviewSegment.GetSectionIndex() != InSectionIndex || !PreviewSegment.IsValid())
			continue;

		if (PreviewSegment.SegmentPtr->HasParent())
		{
			if (const FNeAbilitySegment* ParentSegment = PreviewSegment.SegmentPtr.GetSectionPtr()->GetSegmentByID(PreviewSegment.SegmentPtr->GetParentSegment()))
			{
				const UNeAbilityBeam_Trigger* TriggerBeam = Cast<UNeAbilityBeam_Trigger>(ParentSegment->GetAbilityBeam());
				if (TriggerBeam == nullptr || !TriggerBeam->bOpenForSimulate)
				{
					continue;
				}
			}
			// UAbilityBranchBase* BranchBaseTask = Cast<UAbilityBranchBase>(PreviewSegment.SegmentPtr->GetParentTask());
			// if (!BranchBaseTask || !BranchBaseTask->bOpenSimulator)
			// {
			// 	continue;
			// }
		}

		if (!PreviewSegment.SegmentPtr->IsEnable()) continue;

		EAbilitySegmentPreviewState NewPreviewState = TestPosition(PreviewSegment.GetSegment(), NewPosition);
		EAbilitySegmentPreviewState OldPreviewState = PreviewSegment.State;

		bool bNeedEval = false;

		if (NewPreviewState == EAbilitySegmentPreviewState::Awaiting)
		{
			/*if (NewPosition < PreviousPosition && OldPreviewState == ETaskPreviewState::Passed)
			{
				SampleTasks.Add(&PreviewTask);
				bNeedInstance = true;
			}*/
		}
		else if (NewPreviewState == EAbilitySegmentPreviewState::Running)
		{
			SampleNodes.Add(&PreviewSegment);
			bNeedEval = true;
		}
		else if (NewPreviewState == EAbilitySegmentPreviewState::Passed)
		{
			/*if (NewPosition > PreviousPosition && OldPreviewState == ETaskPreviewState::Awaiting)
			{
				SampleTasks.Add(&PreviewTask);
				bNeedInstance = true;
			}*/
		}

		if (bNeedEval)
		{
			if (!PreviewSegment.EvalContext.IsValid())
			{
				const int32 SegmentIndex = PreviewSegment.SegmentPtr.GetSectionPtr()->FindSegmentIndexByID(PreviewSegment.SegmentPtr.GetSegmentID());
				PreviewSegment.EvalContext.Segment = { PreviewSegment.SegmentPtr.GetSectionIndex(), SegmentIndex };
				PreviewSegment.EvalContext.Ability = this;
				PreviewSegment.EvalContext.GizmoActor = AbilityEditor.Pin()->GetAbilityPreviewScene()->FindGizmoActor(PreviewSegment.SegmentPtr);
				ActivateSegment(PreviewSegment.EvalContext);

				if (PreviewSegment.EvalContext.GizmoActor.IsValid())
				{
					PreviewSegment.EvalContext.GizmoActor->SynchronizeFromBinding();
				}
			}
		}
		else
		{
			if (PreviewSegment.EvalContext.IsValid())
			{
				EndSegment(PreviewSegment.EvalContext, EAbilityBeamEndReason::PreviewOver);
				PreviewSegment.EvalContext = {};
			}
		}

		PreviewSegment.State = NewPreviewState;
	}

	for (FAbilityPreviewSegment* SampleNode : SampleNodes)
	{
		if (SampleNode->State == EAbilitySegmentPreviewState::Running && SampleNode->EvalContext.IsValid())
		{
			const float TaskStartTime = SampleNode->GetSegment().GetStartTime();
			const float PositionOffset = NewPosition - TaskStartTime;
			check(PositionOffset >= 0);
			const float PreviousPositionOffset = FMath::Max(PreviousPosition - TaskStartTime, 0);
			SampleSegment(SampleNode->EvalContext, PositionOffset, PreviousPositionOffset);
		}
	}
	CurrentPosition = NewPosition;
}

void UNeAbilityPreviewInstance::ActivateSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext)
{
	Super::ActivateSegment(SegmentEvalContext);
}

void UNeAbilityPreviewInstance::EndSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, EAbilityBeamEndReason EndReason)
{
	Super::EndSegment(SegmentEvalContext, EndReason);
}

UNeAbilityBeam* UNeAbilityPreviewInstance::CreateBeamEvalInstance(FNeAbilitySegmentEvalContext& SegmentEvalContext, UNeAbilityBeam* ArcheType)
{
	check(IsValid(ArcheType));
	SegmentEvalContext.BeamInstance = SegmentEvalContext.GetSegment().GetAbilityBeam();
	check(SegmentEvalContext.BeamInstance);
	SegmentEvalContext.BeamInstance->OwnerAbility = this;
	return SegmentEvalContext.BeamInstance;
}

void UNeAbilityPreviewInstance::SampleSegment(FNeAbilitySegmentEvalContext& SegmentEvalContext, const float Position, const float PreviousPosition)
{
	if (SegmentEvalContext.BeamInstance)
	{
		SegmentEvalContext.BeamInstance->SamplePosition(Position, PreviousPosition);
	}
}

FNeAbilitySegmentEvalContext* UNeAbilityPreviewInstance::GetSegmentPreviewContext(const FWeakAbilitySegmentPtr& InSegmentPtr)
{
	for (FAbilityPreviewSegment& PreviewSegment : PreviewSegments)
	{
		if (PreviewSegment.SegmentPtr == InSegmentPtr) return &PreviewSegment.EvalContext;
	}

	return nullptr;
}

void UNeAbilityPreviewInstance::OnAddNewSegment(const FWeakAbilitySegmentPtr& NeAbilitySegment)
{
	BuildPreviewSegments();
}

void UNeAbilityPreviewInstance::OnDeleteSegment(const FWeakAbilitySegmentPtr& NeAbilitySegment)
{
	BuildPreviewSegments();
}

void UNeAbilityPreviewInstance::OnMoveSegment(const FWeakAbilitySegmentPtr& NeAbilitySegment)
{
	check(NeAbilitySegment.IsValid());
	uint32 Index = PreviewSegments.IndexOfByPredicate([&](const FAbilityPreviewSegment& Element) { return Element.GetSegmentID() == NeAbilitySegment->GetID(); });
	if (Index != INDEX_NONE)
	{
		FAbilityPreviewSegment& PreviewSegment = PreviewSegments[Index];
		PreviewSegment.State = TestPosition(PreviewSegment.GetSegment(), CurrentPosition);
	}
}

void UNeAbilityPreviewInstance::OnSegmentPropertyChanged(UObject* EditingObject, const FPropertyChangedEvent& PropertyChangedEvent)
{
	UNeAbilitySegmentEditorObject* SegmentEditorObject = Cast<UNeAbilitySegmentEditorObject>(EditingObject);
	if (SegmentEditorObject == nullptr) return; // Ignore

	if (const UNeAbilityBeam* BeamEditingObject = Cast<UNeAbilityBeam>(PropertyChangedEvent.GetObjectBeingEdited(0)))
	{
		FNeAbilitySegmentEvalContext* EvalContext = GetSegmentPreviewContext(SegmentEditorObject->SegmentPtr);
		if (BeamEditingObject && EvalContext && EvalContext->BeamInstance && BeamEditingObject != EvalContext->BeamInstance)
		{
			check(BeamEditingObject->GetClass() == EvalContext->BeamInstance->GetClass());
			// Beam property sync
			const FProperty* Property = PropertyChangedEvent.Property;
			Property->SetValue_InContainer(EvalContext->BeamInstance, Property->ContainerPtrToValuePtr<void>(BeamEditingObject));
		}
	}
}

EAbilitySegmentPreviewState UNeAbilityPreviewInstance::TestPosition(const FNeAbilitySegment& Segment, const float Position) const
{
	const EAbilityDurationPolicy DurationPolicy = Segment.GetDurationPolicy();
	const float StartTime = Segment.GetStartTime();
	const float Duration = Segment.GetDuration();
	EAbilitySegmentPreviewState PositionInfo = EAbilitySegmentPreviewState::Passed;

	switch (DurationPolicy)
	{
	case EAbilityDurationPolicy::Instant:
		if (Position < StartTime)											PositionInfo = EAbilitySegmentPreviewState::Awaiting;
		else if (Position >= StartTime && Position <= StartTime + 0.033)	PositionInfo = EAbilitySegmentPreviewState::Running;
		else if (Position > StartTime + 0.033)								PositionInfo = EAbilitySegmentPreviewState::Passed;
		break;
	case EAbilityDurationPolicy::HasDuration:
		if (Position < StartTime)					PositionInfo = EAbilitySegmentPreviewState::Awaiting;
		else if (Position > StartTime + Duration)	PositionInfo = EAbilitySegmentPreviewState::Passed;
		else										PositionInfo = EAbilitySegmentPreviewState::Running;
		break;
	case EAbilityDurationPolicy::Infinite:
		if (Position < StartTime)					PositionInfo = EAbilitySegmentPreviewState::Awaiting;
		else										PositionInfo = EAbilitySegmentPreviewState::Running;
		break;
	}
	return PositionInfo;
}
