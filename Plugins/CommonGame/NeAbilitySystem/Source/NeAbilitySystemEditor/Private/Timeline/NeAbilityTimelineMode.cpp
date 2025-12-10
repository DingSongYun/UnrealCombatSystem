// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityTimelineMode.h"
#include "AnimatedRange.h"
#include "NeAbility.h"
#include "NeAbilityAssetNodeFactory.h"
#include "NeAbilityBlueprintCompiler.h"
#include "NeAbilityBlueprintEditor.h"
#include "NeAbilityEditorDelegates.h"
#include "NeAbilityEditorPlayer.h"
#include "NeAbilitySegmentEditorObject.h"
#include "NeAbilityTimelineGroupTrack.h"
#include "NeAbilityTimelineTrack.h"
#include "ScopedTransaction.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Beams/NeAbilityBeamLinkage.h"
#include "Beams/NeAbilityBeam_GameplayTask.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetGuidLibrary.h"
#include "TabFactory/SNeAbilityEditorTab_Palette.h"
#include "Tasks/NeAbilityTask_PlayAnimation.h"

#define LOCTEXT_NAMESPACE "NeTimelineMode_Ability"

FNeAbilityTimelineMode::FNeAbilityTimelineMode(const TSharedPtr<FNeAbilityBlueprintEditor>& InAssetEditorToolkit,
												int32 InSectionIndex, const TSharedRef<FUICommandList>& InCommandList)
	: FNeTimelineMode(InCommandList), AbilityEditor(InAssetEditorToolkit), SectionIndex(InSectionIndex)
{
	check(SectionIndex >= 0);
	bNodePickMode = false;
	SectionPtr = MakeWeakSectionPtr(GetEditingAbility(), SectionIndex);
}

FNeAbilityTimelineMode::~FNeAbilityTimelineMode()
{
}

void FNeAbilityTimelineMode::Initialize()
{
	UpdateRange();
}

float FNeAbilityTimelineMode::GetPlayLength() const
{
	return GetAbilitySection().SectionDuration;
}

double FNeAbilityTimelineMode::GetFrameRate() const
{
	return FNeTimelineMode::GetFrameRate();
}

float FNeAbilityTimelineMode::GetScrubTime() const
{
	if (const auto PreviewPlayer = GetPreviewPlayer())
	{
		return PreviewPlayer->GetPosition();
	}

	return 0.f;
}

void FNeAbilityTimelineMode::SetScrubTime(float NewTime)
{
	if (auto PreviewPlayer = GetPreviewPlayer())
	{
		PreviewPlayer->SetPosition(SectionIndex, NewTime);
	}
}

void FNeAbilityTimelineMode::SortTrackByStartTime()
{
	UNeAbility* AbilityAsset = GetEditingAbility();
	check(AbilityAsset);

	const FScopedTransaction Transaction(LOCTEXT("FNeAbilityTimelineMode", "SortTrackByStartTime"));
	AbilityAsset->Modify();

	struct Local
	{
		static int32 TrackSortValue(const FNeAbilitySegment& Segment)
		{
			return Segment.GetStartTime() * 1000;
		}

		static int32 TrackSortValue(const FNeAbilityTrack& Track)
		{
			float StartTime = 10000.0f;
			// for (int32 i = 0; i < Track.Segments.Num(); i++)
			// {
			// 	if (Track.Segments[i]->GetStartTime() < StartTime)
			// 	{
			// 		StartTime = Track.Segments[i]->GetStartTime();
			// 	}
			// }
			//
			return StartTime * 1000;
		}
	};

	const TArray<FNeAbilityTrackGroup>& Groups = SectionPtr->GetAllTrackGroups();
	for (int32 i = 0; i < Groups.Num(); i++)
	{
		TArray<FNeAbilityTrack*> Tracks = SectionPtr->GetTracks(Groups[i]);

		// Tracks.Sort([=](const FNeAbilityTrack* A, const FNeAbilityTrack* B) { return Local::TrackSortValue(A) <= Local::TrackSortValue(B); });
	}
	// SectionPtr->Segments.Sort([])

	RefreshTracks();
}

void FNeAbilityTimelineMode::UpdateRange()
{
	FAnimatedRange OldPlaybackRange = PlaybackRange;

	// update playback range
	PlaybackRange = FAnimatedRange(0.0, (double)GetPlayLength());



	if (OldPlaybackRange != PlaybackRange)
	{
		// Update view/range if playback range changed
		SetViewRange(PlaybackRange);
	}

}

void FNeAbilityTimelineMode::RefreshTracks()
{
	check(SectionPtr.IsValid());

	ClearTrackSelection();

	// Clear all tracks
	TArray<TSharedRef<FNeTimelineTrack>> OldTracks = RootTracks;
	RootTracks.Empty();
	TimelineTracks.Empty();

	//创建Timeline Track
	for (const FNeAbilityTrackGroup& Group : SectionPtr->GetAllTrackGroups())
	{
		TSharedRef<FNeAbilityTimelineGroupTrack> NewTimelineTrackGroup = MakeShareable(new FNeAbilityTimelineGroupTrack(
			SharedThis(this), Group, FText::FromName(Group.GroupName), LOCTEXT("TrackGroup_Tooltip", "Track Group")));

		// 保持Expanded
		const int32 OldTrackIndex = OldTracks.IndexOfByPredicate([=](const TSharedRef<FNeTimelineTrack>& Element)
		{
			const TSharedPtr<FNeAbilityTimelineGroupTrack> OldTrackGroup = StaticCastSharedRef<FNeAbilityTimelineGroupTrack>(Element);
			return OldTrackGroup.IsValid() && &OldTrackGroup->GetGroupData() == &Group;
		});
		const bool bExpanded = OldTrackIndex != INDEX_NONE ? OldTracks[OldTrackIndex]->IsExpanded() : true;
		NewTimelineTrackGroup->SetExpanded(bExpanded);

		TArray<FNeAbilityTrack*> ChildTracks;
		for (FNeAbilityTrack* Track : SectionPtr->GetTracks(Group))
		{
			// 如果子Track，延后创建，等待父Track创建完毕
			if (Track->bIsChildTrack)
			{
				ChildTracks.Add(Track);
				continue;
			}
			TSharedRef<FNeAbilityTimelineTrack> NewTimelineTrack = MakeShareable(new FNeAbilityTimelineTrack(
				SharedThis(this), *Track, FText::FromName(Track->TrackName), LOCTEXT("Track_Tooltip", "Ability Track")));
			NewTimelineTrackGroup->AddChild(NewTimelineTrack);
			
			TimelineTracks.Add(NewTimelineTrack);
		}

		// Create Child tracks
		for (FNeAbilityTrack* ChildTrack : ChildTracks)
		{
			check(ChildTrack->bIsChildTrack);
			const FNeAbilityTrack* Track = SectionPtr->GetTrackAttachedParent(*ChildTrack);
			if (Track == nullptr)
			{
				checkf(false, TEXT("Can not find parent track."));
				continue;
			}
			TSharedPtr<FNeAbilityTimelineTrack> ParentTimelineTrack = GetTimelineTrack(*Track);
			check(ParentTimelineTrack);
			ParentTimelineTrack->SetExpanded(true);

			TSharedRef<FNeAbilityTimelineTrack> NewChildTimelineTrack = MakeShareable(new FNeAbilityTimelineTrack(
				SharedThis(this), *ChildTrack, FText::FromName(ChildTrack->TrackName), LOCTEXT("Track_Tooltip", "Sub Track")));
			NewChildTimelineTrack->AttachToTrack(ParentTimelineTrack);
			ParentTimelineTrack->AddChild(NewChildTimelineTrack);

			TimelineTracks.Add(NewChildTimelineTrack);
		}

		RootTracks.Add(NewTimelineTrackGroup);
	}

	OnTracksChangedDelegate.Broadcast();

	SnapViewToPlayRange();
}

void FNeAbilityTimelineMode::ClearTrackSelection()
{
	FNeTimelineMode::ClearTrackSelection();
	SelectObjects({});
}

void FNeAbilityTimelineMode::SelectObjects(const TArray<UObject*>& Objects)
{
	UNeAbilitySegmentEditorObject* SelectedSegmentObject = nullptr;
	for (UObject* SelectedObject : Objects)
	{
		if (UNeAbilitySegmentEditorObject* SegmentObject = Cast<UNeAbilitySegmentEditorObject>(SelectedObject))
		{
			SelectedSegmentObject = SegmentObject;
			break;
		}
	}

	if (bNodePickMode)
	{
		if (SelectedSegmentObject && OnPickNodeCallback.IsBound()) OnPickNodeCallback.Execute(SelectedSegmentObject->SegmentPtr);
		ExitNodePickMode();
	}
	else
	{
		FNeTimelineMode::SelectObjects(Objects);
		AbilityEditor.Pin()->ShowInDetailsView(Objects);
		if (SelectedSegmentObject)
		{
			FNeAbilityEditorDelegates::SegmentSelectionChangedDelegate.Broadcast(SelectedSegmentObject->SegmentPtr);
		}
	}
}

bool FNeAbilityTimelineMode::OnDropAction(const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation->IsOfType<FGraphSchemaActionDragDropAction_AbilityPaletteItem>())
	{
		const auto& PaletteItem = StaticCastSharedPtr<FGraphSchemaActionDragDropAction_AbilityPaletteItem>(Operation);

		AddNewTrack(SectionPtr->GetDefaultTrackGroup(), PaletteItem->GetAbilityActionClass());

		return true;
	}
	else if (Operation->IsOfType<FAssetDragDropOp>())
	{
		const auto& AssetDragDropOp = StaticCastSharedPtr<FAssetDragDropOp>(Operation);
		if (AssetDragDropOp->HasAssets())
		{
			const FAssetData AssetData = AssetDragDropOp->GetAssets()[0];
			if (UObject* Asset = AssetData.GetAsset())
			{
				const UNeAbilityNodeFactory* Factory = UNeAbilityNodeFactory::FindFactory(AssetData);
				if (Factory)
				{
					if (FNeAbilityTrack* NewTrack = Factory->CreateNewTrack(Asset, *this, NAME_None))
					{
						FNeAbilitySegment* NewSegment = GetAbilitySection().GetSegmentByID(NewTrack->Segments[0]);
						Factory->PostCreateNewTrack(Asset, *NewTrack, *NewSegment);
					}
					Modify();
				}
			}
		}
		return true;
	}

	return false;
}

int32 FNeAbilityTimelineMode::GetPlayFrameCount() const
{
	return GetPlayLength() * GetFrameRate();
}

UNeAbility* FNeAbilityTimelineMode::GetEditingAbility() const
{
	return AbilityEditor.Pin()->GetEditingAbility();
}

FNeAbilitySection& FNeAbilityTimelineMode::GetAbilitySection() const
{
	UNeAbility* Ability = GetEditingAbility();
	check(GetEditingAbility());
	check(Ability->Sections.IsValidIndex(SectionIndex));

	return Ability->Sections[SectionIndex];
}

FNeAbilitySegment& FNeAbilityTimelineMode::GetAbilitySegment(uint32 SegmentID) const
{
	FNeAbilitySection& Section = GetAbilitySection();
	check(Section.IsValidSegmentID(SegmentID));
	return *GetAbilitySection().GetSegmentByID(SegmentID);
}

FNeAbilitySectionPtr FNeAbilityTimelineMode::GetAbilitySectionPtr() const
{
	return SectionPtr;
	// return MakeWeakSectionPtr(GetEditingAbility(), SectionIndex);
}

FNeAbilitySegmentPtr FNeAbilityTimelineMode::GetAbilitySegmentPtr(uint32 SegmentID) const
{
	return MakeWeakSegmentPtr(SectionPtr, SegmentID);
}

TSharedPtr<FNeAbilityEditorPlayerBase> FNeAbilityTimelineMode::GetPreviewPlayer() const
{
	return AbilityEditor.Pin()->GetAbilityPreviewPlayer();
}

void FNeAbilityTimelineMode::SetSectionDuration(float NewDuration)
{
	Modify();
	SectionPtr->SectionDuration = NewDuration;
	UpdateRange();
}

void FNeAbilityTimelineMode::SetSectionName(FName NewName)
{
	Modify();
	SectionPtr->SectionName = NewName;
}

bool FNeAbilityTimelineMode::GetSectionLoop() const
{
	return SectionPtr->bEditorLoop;
}

void FNeAbilityTimelineMode::SetSectionLoop(bool bLoop)
{
	SectionPtr->bEditorLoop = bLoop;
}

void FNeAbilityTimelineMode::Modify()
{
	AbilityEditor.Pin()->GetBlueprintObj()->Modify();
}

void FNeAbilityTimelineMode::EnterNodePickMode(FOnPickNodeDelegate OnPickCallback)
{
	bNodePickMode = true;
	OnPickNodeCallback = OnPickCallback;
}

void FNeAbilityTimelineMode::ExitNodePickMode()
{
	bNodePickMode = false;
	OnPickNodeCallback = FOnPickNodeDelegate();
}

TSharedPtr<FNeAbilityTimelineTrack> FNeAbilityTimelineMode::GetTimelineTrack(const FNeAbilityTrack& Track) const
{
	for (const auto& TimelineTrack : TimelineTracks)
	{
		if (&TimelineTrack->GetTrackData() == &Track)
		{
			return TimelineTrack;
		}
	}

	return nullptr;
}

void FNeAbilityTimelineMode::ChangeTrackGroupName(const FNeAbilityTrackGroup& TrackGroup, FName Name) const
{
	if (Name.IsNone()) return;
	FNeAbilityTrackGroup* MutableGroup = const_cast<FNeAbilityTrackGroup*>(&TrackGroup);
	// 如果跟已有Group重名，忽略
	for (const FNeAbilityTrackGroup& Group : SectionPtr->GetAllTrackGroups())
	{
		if (Group.GroupName == Name)
		{
			return ;
		}
	}

	MutableGroup->GroupName = Name;
}

void FNeAbilityTimelineMode::ChangeTrackName(const FNeAbilityTrack& TrackData, FName Name) const
{
	FNeAbilityTrack* MutableTrack = const_cast<FNeAbilityTrack*>(&TrackData);
	MutableTrack->TrackName = Name;
}

FNeAbilityTrackGroup& FNeAbilityTimelineMode::AddNewTrackGroup(const FName& InGroupName)
{
	check(SectionPtr.IsValid());

	Modify();

	FName GroupName = InGroupName.IsNone() ? "New Group" : InGroupName;

	// Validate group name
	const TArray<FNeAbilityTrackGroup>& AllGroup = SectionPtr->GetAllTrackGroups();
	for (const FNeAbilityTrackGroup& Group : AllGroup)
	{
		if (Group.GroupName == InGroupName)
		{
			GroupName = *FString::Printf(TEXT("%s_%s"), *GroupName.ToString(), *UKismetGuidLibrary::NewGuid().ToString(EGuidFormats::Short));
			break;
		}
	}

	FNeAbilityTrackGroup& NewGroup = SectionPtr->AddNewTrackGroup();
	NewGroup.GroupName = GroupName;

	RefreshTracks();

	return NewGroup;
}

void FNeAbilityTimelineMode::DeleteTrackGroup(const FNeAbilityTrackGroup& TrackGroup)
{
	check(SectionPtr.IsValid());

	TArray<FNeAbilityTrackGroup>& TrackGroups = SectionPtr->GetAllTrackGroups();

	// 保证至少有一个Group
	if (TrackGroups.Num() <= 1) return;

	Modify();

	// 先将Group中Track移动到其他Group
	const FNeAbilityTrackGroup& DefaultGroup = SectionPtr->GetDefaultTrackGroup();
	TArray<FNeAbilityTrack*> GroupTracks = SectionPtr->GetTracks(TrackGroup);
	for (FNeAbilityTrack* Track : GroupTracks)
	{
		if (Track != nullptr)
		{
			SwapTrackToGroup(*Track, DefaultGroup);
		}
	}

	// 删除Group
	const int32 IndexToDelete = TrackGroups.Find(TrackGroup);
	TrackGroups.RemoveAt(IndexToDelete);

	RefreshTracks();
}

void FNeAbilityTimelineMode::SwapTrackToGroup(FNeAbilityTrack& SrcTrack, const FNeAbilityTrackGroup& DestGroupData)
{
	SrcTrack.TrackName = DestGroupData.GroupName;
	RefreshTracks();
	Modify();
}

void FNeAbilityTimelineMode::SwapTrackPosition(FNeAbilityTrack& SrcTrack, bool bSrcChildTrack, FNeAbilityTrack& DestTrack, bool bDestChildTrack)
{
	//TODO: 需要实现
}

void FNeAbilityTimelineMode::SwapNodeToTrack(const FNeAbilitySegmentPtr& SrcNode, bool bSrcChildNode, const FNeAbilityTrack& DestSrcTrack, bool bDestChildNode)
{
	//TODO: 需要实现
}

void FNeAbilityTimelineMode::MoveTrackTo(const FNeAbilityTrackGroup& AtGroup, FNeAbilityTrack& Track, int32 AtPosition)
{
	TArray<FNeAbilityTrack*> GroupTracks = SectionPtr->GetTracks(AtGroup);
	if (AtPosition == 0)
	{
		MoveTrackBefore(Track, *GroupTracks[0]);
	}
	else if (AtPosition > 0 && AtPosition <= GroupTracks.Num())
	{
		MoveTrackAfter(Track, *GroupTracks[AtPosition - 1]);
	}
}

void FNeAbilityTimelineMode::MoveTrackBefore(FNeAbilityTrack& Track, FNeAbilityTrack& DestTrack)
{
	const int32 ToIndex = SectionPtr->IndexOfTrack(DestTrack);
	check(ToIndex != INDEX_NONE);

	TArray<FNeAbilityTrack>& AllTracks = SectionPtr->GetAllTracks();
	AllTracks.Insert(Track, ToIndex);

	const int32 OriginIndex = SectionPtr->IndexOfTrack(Track);
	check(OriginIndex);
	AllTracks.RemoveAt(OriginIndex);
}

void FNeAbilityTimelineMode::MoveTrackAfter(FNeAbilityTrack& Track, FNeAbilityTrack& DestTrack)
{
	const int32 ToIndex = SectionPtr->IndexOfTrack(DestTrack);
	check(ToIndex != INDEX_NONE);

	TArray<FNeAbilityTrack>& AllTracks = SectionPtr->GetAllTracks();
	AllTracks.Insert(Track, ToIndex + 1);

	const int32 OriginIndex = SectionPtr->IndexOfTrack(Track);
	check(OriginIndex);
	AllTracks.RemoveAt(OriginIndex);

}

FNeAbilityTrack& FNeAbilityTimelineMode::AddNewTrack(const FNeAbilityTrackGroup& AtGroup, const FName& InTrackName, int32 AtPosition)
{
	const FScopedTransaction Transaction(LOCTEXT("FNeAbilityTimelineMode", "AddNewTrack"));
	Modify();

	const FName TrackName = InTrackName.IsNone() ? "New Track" : InTrackName;

	FNeAbilityTrack& NewTrack = SectionPtr->AddNewTrack();
	NewTrack.TrackName = TrackName;
	NewTrack.GroupName = AtGroup.GroupName;
	if (AtPosition > 0)
	{
		MoveTrackTo(AtGroup, NewTrack, AtPosition);
	}

	RefreshTracks();

	return NewTrack;
}

FNeAbilitySegment& FNeAbilityTimelineMode::AddNewSegment(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentDef& SegmentDef)
{
	const FScopedTransaction Transaction(LOCTEXT("FNeAbilityTimelineMode", "AddNewSegment~"));
	Modify();

	UNeAbility* Ability = GetEditingAbility();
	FNeAbilitySegment& NewSegment = Ability->AddNewSegment(SegmentDef, SectionIndex);
	AtTrack.Segments.Add(NewSegment.GetID());
	UNeAbilityBeam* AbilityBeam = NewSegment.GetAbilityBeam();
	if (AbilityBeam && AbilityBeam->IsA<UNeAbilityBeamLinkage>())
	{
		FBeamLinkageCompilerContext::PostConstructBeamLinkage(GetAbilitySegmentPtr(NewSegment.GetID()), Cast<UNeAbilityBeamLinkage>(AbilityBeam));
	}

	FNeAbilityEditorDelegates::AddNewSegmentDelegate.Broadcast(GetAbilitySegmentPtr(NewSegment.GetID()));

	return NewSegment;
}

void FNeAbilityTimelineMode::RemoveSegment(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentPtr& InSegment)
{
	const FScopedTransaction Transaction(LOCTEXT("FNeAbilityTimelineMode", "RemoveSegment~"));
	Modify();

	UNeAbility* Ability = GetEditingAbility();

	// Broadcast we are going to delete segment
	FNeAbilityEditorDelegates::PreDeleteSegmentDelegate.Broadcast(InSegment);

	AtTrack.Segments.Remove(InSegment->GetID());
	Ability->RemoveSegment(InSegment.GetSectionIndex(), InSegment.Get());
}

FNeAbilityTrack& FNeAbilityTimelineMode::AddNewTrack(const FNeAbilityTrackGroup& AtGroup, const FNeAbilitySegmentDef& SegmentDef)
{
	check(SegmentDef.IsValid());

	// 先创建Track
	FNeAbilityTrack& NewTrack = AddNewTrack(AtGroup, *SegmentDef.ActionClass->GetDisplayNameText().ToString());

	AddNewSegment(NewTrack, SegmentDef);

	RefreshTracks();

	return NewTrack;
}

FNeAbilityTrack& FNeAbilityTimelineMode::AddSubTrack(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentDef& SegmentDef)
{
	//TODO: 需要实现
	FNeAbilityTrack& NewTrack = AddNewTrack(*SectionPtr->FindGroupOfTrack(AtTrack), *SegmentDef.ActionClass->GetDisplayNameText().ToString());

	return NewTrack;
}

bool FNeAbilityTimelineMode::RemoveTrack(FNeAbilityTrack& AtTrack)
{
	// Remove Segment
	for (TArray<int32> SegmentToRemove = AtTrack.Segments; const int32 SegmentID : SegmentToRemove)
	{
		RemoveSegment(AtTrack, MakeWeakSegmentPtr(SectionPtr, SegmentID));
	}
	AtTrack.Segments.Empty();

	TArray<FNeAbilityTrack>& AllTracks = SectionPtr->GetAllTracks();
	if (int32 Index = AllTracks.Find(AtTrack); Index != INDEX_NONE)
	{
		AllTracks.RemoveAt(Index);
	}
	Modify();
	RefreshTracks();

	return true;
}

bool FNeAbilityTimelineMode::RemoveTrackNode(FNeAbilityTrack& AtTrack, const FNeAbilitySegmentPtr& SegmentPtr)
{
	RemoveSegment(AtTrack, SegmentPtr);
	return true;
}

EAbilityNodeExecutionState FNeAbilityTimelineMode::GetTrackNodeExecutionState(const FNeAbilitySegmentPtr& SegmentPtr) const
{
	//TODO: 需要实现
	return EAbilityNodeExecutionState::UnActivated;
}

void FNeAbilityTimelineMode::AttachTo(FNeAbilitySegmentPtr& InParent, FNeAbilitySegmentPtr& InSegment)
{
	check(&InParent != &InSegment);

	if (!ensureMsgf(InParent->IsCompound(), TEXT("Can not attach to non-compound segment!!!"))) return ;

	if (!ensureMsgf(!InParent->HasParent(), TEXT("Can not attach to child segment!!!"))) return ;

	if (!ensureMsgf(InParent.GetSectionIndex() == InSegment.GetSectionIndex(), TEXT("Can not attach to other segment!!!"))) return ;

	if (!ensureMsgf(InSegment->GetAllChildSegments().Num() == 0, TEXT("It's not safe to convert a parent segment to child segment!!!"))) return ;

	const FNeAbilityTrack* ParentTrack = GetAbilitySectionPtr()->FindTrackOfSegment(InParent.Get());
	FNeAbilityTrack* SegmentTrack = GetAbilitySectionPtr()->FindTrackOfSegment(InSegment.Get());

	//非child，相互Attach
	if (!InSegment->HasParent())
	{
		SegmentTrack->bIsChildTrack = true;
	}
	else //child Attach new parent
	{
		FWeakAbilitySegmentPtr OldParentSegment = InSegment.GetParentSegment();
		OldParentSegment->RemoveChild(InSegment->GetID());
	}

	// Make attach
	InParent->AddChild(InSegment->GetID());
	InSegment->SetParentSegment(InParent->GetID());

	// Make same group
	SegmentTrack->GroupName = ParentTrack->GroupName;

	//align start time
	if (InSegment->GetStartTime() < InParent->GetStartTime())
	{
		InSegment->SetStartTime(InParent->GetStartTime());
	}

	RefreshTracks();

	Modify();
}

void FNeAbilityTimelineMode::Detach(FNeAbilitySegmentPtr& InSegment)
{
	if (!ensureMsgf(InSegment->HasParent(), TEXT("Can only detach child segment!!!"))) return ;

	FWeakAbilitySegmentPtr ParentSegment = InSegment.GetParentSegment();
	check(ParentSegment.IsValid());

	ParentSegment->RemoveChild(InSegment.GetSegmentID());
	InSegment->SetParentSegment(FNeAbilitySegment::INVALID_ID);

	FNeAbilityTrack* SegmentTrack = GetAbilitySectionPtr()->FindTrackOfSegment(InSegment.Get());
	SegmentTrack->bIsChildTrack = false;

	RefreshTracks();
}

void FNeAbilityTimelineMode::AlignSectionTimeWithSegment(const FNeAbilitySegmentPtr& InSegmentPtr)
{
	float NewSectionTime = SectionPtr->SectionDuration;
	if (InSegmentPtr->GetDurationPolicy() == EAbilityDurationPolicy::HasDuration)
	{
		NewSectionTime = InSegmentPtr->GetDuration();
	}

	if (NewSectionTime != SectionPtr->SectionDuration)
	{
		SetSectionDuration(NewSectionTime);
		RefreshTracks();
	}
}

void FNeAbilityTimelineMode::AlignSectionTimeWithAnimation(bool bAllSection)
{
	float NewSectionTime = SectionPtr->SectionDuration;
	for (FNeAbilitySegment& Segment: SectionPtr->Segments)
	{
		UNeAbilityBeam* Beam = Segment.GetAbilityBeam();
		if (Beam == nullptr) continue;

		if (UNeAbilityBeam_GameplayTask* TaskBeam = Cast<UNeAbilityBeam_GameplayTask>(Beam))
		{
			if (const UNeAbilityTask_PlayAnimation* AnimTask = Cast<UNeAbilityTask_PlayAnimation>(TaskBeam->TaskTemplate))
			{
				const float AnimDuration = AnimTask->EvalAnimRelevanceDuration();
				Segment.SetStartTime(0.f);
				TaskBeam->SetDuration(AnimDuration);
				NewSectionTime = TaskBeam->Duration;
			}
		}
	}

	if (NewSectionTime != SectionPtr->SectionDuration)
	{
		SetSectionDuration(NewSectionTime);
		RefreshTracks();
	}
}

void FNeAbilityTimelineMode::SnapViewToPlayRange()
{
	const double Resolution = GetTickResolution();
	const TRange<FFrameNumber> TimelinePlaybackRange = GetPlaybackRange();
	SetViewRange(TRange<double>(TimelinePlaybackRange.GetLowerBoundValue().Value / Resolution, TimelinePlaybackRange.GetUpperBoundValue().Value / Resolution));
}

void FNeAbilityTimelineMode::AddBreakPoint(const FNeAbilitySegmentPtr& InSegment) const
{
	//TODO: 需要实现
}

void FNeAbilityTimelineMode::RemoveBreakPoint(const FNeAbilitySegmentPtr& InSegment) const
{
	//TODO: 需要实现
}

bool FNeAbilityTimelineMode::HasBreakpointToggled(const FNeAbilitySegmentPtr& SegmentPtr) const
{
	//TODO: 需要实现
	return false;
}

bool FNeAbilityTimelineMode::IsBreakpointTriggered(const FNeAbilitySegmentPtr& SegmentPtr) const
{
	//TODO: 需要实现
	return false;
}

void FNeAbilityTimelineMode::CopySelectionSegments()
{
	//TODO: 需要实现
}

void FNeAbilityTimelineMode::DuplicateSelectionSegment()
{
	//TODO: 需要实现
}

void FNeAbilityTimelineMode::DoPaste()
{
	//TODO: 需要实现
}

void FNeAbilityTimelineMode::DoPasteNodeToTrack(FNeAbilityTrack* TrackData, bool bChildTrack, float StartTime)
{
	//TODO: 需要实现
	// FNeAbilityEditorDelegates::AddNewSegmentDelegate.Broadcast(GetAbilitySegmentPtr(NewSegment.GetID()));
}

void FNeAbilityTimelineMode::HandleUndoRedo()
{
	RefreshTracks();
}

#undef LOCTEXT_NAMESPACE
