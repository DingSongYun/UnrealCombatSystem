// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilitySegment.h"
#include "NeAbility.h"
#include "Beams/NeAbilityBeamLinkage.h"
#include "Misc/NeAbilityGizmoActor.h"

uint32 FNeAbilitySegment::INVALID_ID = 0;

FNeAbilitySegment::FNeAbilitySegment(uint32 SegID) : ID(SegID)
{
	StartTime = 0.f;
	bEnable = true;
	bNeverExpire = true;
}

void FNeAbilitySegment::OnActive()
{
	// Implemented by sub class
}

void FNeAbilitySegment::OnEnd()
{
	// Implemented by sub class
}

EAbilityDurationPolicy FNeAbilitySegment::GetDurationPolicy() const
{
	return Beam ? Beam->GetDurationPolicy() : EAbilityDurationPolicy::Instant;
}

bool FNeAbilitySegment::IsInstant() const
{
	return Beam ? Beam->IsInstant() : true;
}

float FNeAbilitySegment::GetDuration() const
{
	return Beam ? Beam->GetDuration() : 0;
}

void FNeAbilitySegment::SetDuration(float NewDuration) const
{
	if (Beam)
	{
		Beam->SetDuration(NewDuration);
	}
}

FName FNeAbilitySegment::GetName() const
{
	return SegmentName_Override.IsNone() ? Beam->GetFName() : SegmentName_Override;
}

bool FNeAbilitySegment::IsCompound() const
{
	return Beam ? Beam->IsCompound() : false;
}

#if WITH_EDITORONLY_DATA

UNeAbilityBeam* FNeAbilitySegment::CreateBeam(UObject* Outter, UClass* Class, UObject* Template) const
{
	UNeAbilityBeam* OutBeam = nullptr;
	if (UNeAbilityBeamLinkage::IsBeamLinkType(Class))
	{
		const UNeAbilityBeamLinkage::FBeamLinkageDesc* Desc = UNeAbilityBeamLinkage::GetBeamLinkageDesc(Class);
		OutBeam = NewObject<UNeAbilityBeam>(Outter, Desc->LinkageBeamClass, NAME_None, RF_Transactional, Template);
		UNeAbilityBeamLinkage* BeamLinkage = Cast<UNeAbilityBeamLinkage>(OutBeam);
		check(BeamLinkage);
		BeamLinkage->InitialLink(Class);
	}
	else if (Class->IsChildOf(UNeAbilityBeam::StaticClass()))
	{
		OutBeam = NewObject<UNeAbilityBeam>(Outter, Class, NAME_None, RF_Transactional, Template);
	}

	check(OutBeam);
	return OutBeam;
}

FText FNeAbilitySegment::GetToolTipText() const
{
	if (Beam)
	{
		return Beam->GetClass()->GetToolTipText();
	}

	return FText();
}

#endif

#if WITH_EDITOR
void FNeAbilitySegment::Construct(UNeAbility* OutterAbility, const FNeAbilitySegmentDef& SegmentDef)
{
	UObject* BeamTemplate = SegmentDef.Template ? SegmentDef.Template->Beam : nullptr;
	Beam = CreateBeam(OutterAbility, SegmentDef.ActionClass, BeamTemplate);
}

FText FNeAbilitySegment::GetDisplayText() const
{
	return SegmentName_Override.IsNone() ? Beam->GetDisplayText() : FText::FromName(SegmentName_Override);
}

void FNeAbilitySegment::SetStartTime(float NewStartTime)
{
	StartTime = NewStartTime;

	// TODO: Broadcast start time changed if need.
}

int32 FNeAbilitySegment::AddChild(uint32 ChildID)
{
	return Children.AddUnique(ChildID);
}

int32 FNeAbilitySegment::RemoveChild(uint32 ChildID)
{
	return Children.Remove(ChildID);
}

bool FNeAbilitySegment::ShouldCreateGizmo() const
{
	return Beam ? Beam->bNeedGizmo : false;
}

UClass* FNeAbilitySegment::GetGizmoActorType() const
{
	if (Beam)
	{
		return Beam->GizmoType.Get();
	}

	return nullptr;
}
#endif

//~=============================================================================
/**
 * FAbilitySegmentReference
 */
FNeAbilitySegment& FNeAbilitySegmentReference::Resolve(UNeAbility* Ability)
{
	check(Ability && Ability->IsValidSection(SectionIndex));
	check(SegmentIndex != INDEX_NONE);
	return Ability->GetSection(SectionIndex).Segments[SegmentIndex];
}

const FNeAbilitySegment& FNeAbilitySegmentReference::Resolve(UNeAbility* Ability) const
{
	check(Ability && Ability->IsValidSection(SectionIndex));
	check(SegmentIndex != INDEX_NONE);
	return Ability->GetSection(SectionIndex).Segments[SegmentIndex];
}

void FNeAbilitySegmentEvalContext::PreReplicatedRemove(const FAbilitySegmentQueue& InArraySerializer)
{
}

void FNeAbilitySegmentEvalContext::PostReplicatedAdd(const FAbilitySegmentQueue& InArraySerializer)
{
}

bool FNeAbilitySegmentEvalContext::ShouldReplicate() const
{
	if (BeamInstance && BeamInstance->ShouldReplicate())
	{
		return true;
	}

	return false;
}

const FNeAbilitySegment& FNeAbilitySegmentEvalContext::GetSegment() const
{
	return Segment.Resolve(Ability);
}

UAbilitySystemComponent* FNeAbilitySegmentEvalContext::GetAbilitySystemComponent() const
{
	return Ability->GetAbilitySystemComponentFromActorInfo();
}

AActor* FNeAbilitySegmentEvalContext::GetOwningActor() const
{
	return Ability->GetOwningActorFromActorInfo();
}

AActor* FNeAbilitySegmentEvalContext::GetInstigator() const
{
	return Ability->GetInstigator();
}

bool FNeAbilitySegmentEvalContext::IsLocallyControlled() const
{
	return Ability->GetActorInfo().IsLocallyControlled();
}

bool FNeAbilitySegmentEvalContext::IsNetAuthority() const
{
	return Ability->GetActorInfo().IsNetAuthority();
}

void FNeAbilitySegmentEvalContext::NotifySegmentChildrenTriggered()
{
	Ability->TriggerSegmentChildren(*this);
}

void FNeAbilitySegmentEvalContext::EvaluatePropertyBindings()
{
	Ability->EvaluatePropertyBindings(*this);
}

FString FNeAbilitySegmentEvalContext::ToString() const
{
	return FString::Printf(TEXT("[%d-%d] %s"),
		Segment.GetSectionIndex(),
		Segment.GetSegmentIndex(),
		*GetSegment().GetName().ToString());
}

void FAbilitySegmentQueue::AddSegment(int32 SectionIndex, int32 SegmentIndex)
{
	check(SectionIndex != INDEX_NONE && SegmentIndex != INDEX_NONE);
	if (SegmentReadyScopeLockCount > 0)
	{
		PendingReady.Add({SectionIndex, SegmentIndex});
		return ;
	}
	Ready.Add({SectionIndex, SegmentIndex});
}

void FAbilitySegmentQueue::DecrementSegmentReadyListLock()
{
	-- SegmentReadyScopeLockCount;
	check(SegmentReadyScopeLockCount >= 0);

	if (SegmentReadyScopeLockCount == 0 && PendingReady.Num() > 0)
	{
		Ready.Append(PendingReady);
		PendingReady.Empty();
	}
}
