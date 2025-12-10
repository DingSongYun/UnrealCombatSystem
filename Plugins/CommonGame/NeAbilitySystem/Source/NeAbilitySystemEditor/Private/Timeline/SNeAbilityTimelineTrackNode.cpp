// Copyright NetEase Games, Inc. All Rights Reserved.

#include "SNeAbilityTimelineTrackNode.h"

#include "Editor.h"
#include "NeAbility.h"
#include "NeAbilityEditorTypes.h"
#include "NeAbilityTimelineMode.h"
#include "ScopedTransaction.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Timeline/SNeTimeline.h"

#define LOCTEXT_NAMESPACE "SNeAbilityTimelineTrackNode"


const float NotifyHeightOffset = 0.f;
const float NodePaddingY = 4.0f;
const FVector2D ScrubHandleSize(10.0f, 10.0f);
const FVector2D AlignmentMarkerSize(10.f, 20.f);
const FVector2D DefTextBorderSize(1.f, 4.f);
const FVector2D ChildTextBorderSize(1.f, 3.f);

#define NODE_HEIGHT(bSubNode) NeAbilityEditorConstants::GetTrackHeight(bSubNode)

//////////////////////////////////////////////////////////////////////////
//FNeAbilityTrackNodeData

FNeAbilityTrackNodeData::FNeAbilityTrackNodeData()
{
	
}

void FNeAbilityTrackNodeData::Initialize(FTrackNodeDataPtr InData, float InFrameRate)
{
	InnerData = InData;
	FrameRate = InFrameRate;

}

void FNeAbilityTrackNodeData::SetStartTime(float Time,  bool bSnapped)
{
	if (InnerData.IsValid())
	{
		const FScopedTransaction Transaction(LOCTEXT("FNeAbilityTrackNodeData", "SetStartTime"));
		Modify();
		
		float SnappedTime = Time;
		if (bSnapped && SNeTimeline::IsEnabledSnaps())
		{
			double OneFrameTimes = 1.0 / FrameRate;
			SnappedTime = FMath::FloorToInt((Time + OneFrameTimes * 0.5f) / OneFrameTimes) * OneFrameTimes;
		}

		if (InnerData->HasParent())
		{
			if (FTrackNodeDataPtr ParentSegment = InnerData.GetParentSegment(); ParentSegment.IsValid())
			{
				InnerData->SetStartTime(SnappedTime < ParentSegment->GetStartTime() ? InnerData->GetStartTime() : SnappedTime);
			}
		}
		else
		{
			InnerData->SetStartTime(SnappedTime);
		}
	}
}

void FNeAbilityTrackNodeData::SetDuration(float InDuration)
{
	if (InnerData.IsValid())
	{
		const FScopedTransaction Transaction(LOCTEXT("FNeAbilityTrackNodeData", "SetDuration"));
		Modify();

		InnerData->SetDuration(InDuration);
	}
}

float FNeAbilityTrackNodeData::GetStartTime() const
{
	return InnerData.IsValid() ? InnerData->GetStartTime() : 0.f;
}

float FNeAbilityTrackNodeData::GetDuration()  const
{
	if (!InnerData.IsValid())
	{
		return 0.f;
	}

	float Duration = InnerData->GetDuration();
	if (IsInstantType())
	{
		return FMath::Max(0.1f, 1.0f / FrameRate);
	}
	else if (InnerData->GetDurationPolicy() == EAbilityDurationPolicy::Infinite)
	{
		return 100000;
	}

	return FMath::Max(Duration, 1.0f / FrameRate);
}

bool FNeAbilityTrackNodeData::IsEnable() const
{
	if (InnerData.IsValid())
	{
		return InnerData->IsEnable();
	}

	return false;
}

void FNeAbilityTrackNodeData::ToggleEnable()
{
	if (InnerData.IsValid())
	{
		return InnerData->SetEnable(!InnerData->IsEnable());
	}
}

bool FNeAbilityTrackNodeData::IsInstantType() const
{
	if (InnerData.IsValid())
	{
		return InnerData->GetDurationPolicy() == EAbilityDurationPolicy::Instant;
	}
	
	return false;
}

bool FNeAbilityTrackNodeData::HasDuration() const
{
	if (InnerData.IsValid())
	{
		return InnerData->GetDurationPolicy() == EAbilityDurationPolicy::HasDuration;
	}

	return false;

}

void FNeAbilityTrackNodeData::Snapped()
{
	if (SNeTimeline::IsEnabledSnaps())
	{
		double OneFrameTimes = 1.0 / FrameRate;
		float StartTime = FMath::FloorToInt((GetStartTime() + OneFrameTimes * 0.5f) / OneFrameTimes) * OneFrameTimes;
		SetStartTime(StartTime);
	}
}

void FNeAbilityTrackNodeData::Modify()
{
	check(InnerData.IsValid());
	InnerData.GetOutter()->Modify();
}

FText FNeAbilityTrackNodeData::GetDisplayText() const
{
	return  InnerData.IsValid() ? InnerData->GetDisplayText() : FText::FromName(NAME_Error);
}

TOptional<FLinearColor> FNeAbilityTrackNodeData::GetEditorColor()  const
{
	if (InnerData.IsValid() == false || InnerData->IsEnable() == false)
	{
		return NeAbilityEditorConstants::TrackNodeColorDisable;
	}
	return InnerData->IsInstant()
		? NeAbilityEditorConstants::TrackInstantNodeColorEnable
		: NeAbilityEditorConstants::TrackNodeColorEnable;
}

FText FNeAbilityTrackNodeData::GetNodeTooltip()  const
{
	FString ToolTipString;

	if (InnerData.IsValid())
	{
		ToolTipString = FString::Printf(TEXT("@start: %.2f, dura: %.2f\n%s\n%s"), GetStartTime(), GetDuration(),
			*InnerData->GetName().ToString(), *InnerData->GetToolTipText().ToString());
	}

	return  FText::FromString(ToolTipString);
}


//////////////////////////////////////////////////////////////////////////
// SNeAbilityTimelineTrackNode

const float SNeAbilityTimelineTrackNode::MinimumStateDuration = (1.0f / 30.0f);

void SNeAbilityTimelineTrackNode::Construct(const FArguments& InArgs,  TWeakPtr<FNeAbilityTimelineMode> InController, bool IsChildNode, bool IsDebugger)
{
	Font = FCoreStyle::GetDefaultFontStyle("Regular", 10);
	bBeingDragged = false;
	CurrentDragHandle = ENotifyStateHandleHit::None;
	bDrawTooltipToRight = true;
	bSelected = false;
	DragMarkerTransactionIdx = INDEX_NONE;

	WeakTimelineMode = InController;
	bIsChildNode = IsChildNode;
	bIsDebugger = IsDebugger;

	EditorNodeData.Initialize(InArgs._NodeData, InArgs._FrameRate.Get());
	NodeIndex = InArgs._NodeIndex.Get();
	OnNodeDragStarted = InArgs._OnNodeDragStarted;
	//OnTrackNodeSelectChanged = InArgs._OnTrackNodeSelectChanged;
	OnUpdatePanel = InArgs._OnUpdatePanel;

	ViewInputMin = InArgs._ViewInputMin;
	ViewInputMax = InArgs._ViewInputMax;
	TimelinePlayLength = InArgs._TimelinePlayLength.Get();

	SetClipping(EWidgetClipping::ClipToBounds);


	SetToolTipText(TAttribute<FText>(this, &SNeAbilityTimelineTrackNode::GetNodeTooltip));
}

FReply SNeAbilityTimelineTrackNode::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FVector2D ScreenNodePosition = FVector2D(MyGeometry.AbsolutePosition);

	// Whether the drag has hit a duration marker
	bool bDragOnMarker = false;
	bBeingDragged = true;

	if (GetDurationSize() > 0.0f && !bIsDebugger)
	{
		ENotifyStateHandleHit::Type MarkerHit = DurationHandleHitTest(LastMouseDownPosition);
		if (MarkerHit == ENotifyStateHandleHit::Start || MarkerHit == ENotifyStateHandleHit::End)
		{
			bDragOnMarker = true;
			bBeingDragged = false;
			CurrentDragHandle = MarkerHit;

			// Modify the owning sequence as we're now dragging the marker and begin a transaction
			//check(DragMarkerTransactionIdx == INDEX_NONE);
			DragMarkerTransactionIdx = GEditor->BeginTransaction(NSLOCTEXT("AnimNotifyNode", "StateNodeDragTransation", "Drag State Node Marker"));
		}

	}

	return OnNodeDragStarted.Execute(SharedThis(this), MouseEvent, ScreenNodePosition, bDragOnMarker);
}

FLinearColor SNeAbilityTimelineTrackNode::GetNodeColor() const
{
	FLinearColor BaseColor = EditorNodeData.GetEditorColor().GetValue();
	if (bIsChildNode)
	{
		BaseColor = EditorNodeData.IsEnable() ? NeAbilityEditorConstants::SubTrackNodeColorEnable : NeAbilityEditorConstants::SubTrackNodeColorDisable;
	}
	BaseColor.A = 0.67f;

	if (bIsDebugger && EditorNodeData.IsEnable())
	{
		const EAbilityNodeExecutionState ExecutionType = WeakTimelineMode.Pin()->GetTrackNodeExecutionState(EditorNodeData.GetNodeData());
		if (ExecutionType == EAbilityNodeExecutionState::UnActivated)
		{
			BaseColor = NeAbilityEditorConstants::DebuggerNodeUnActiveColor;
		}
		else if (ExecutionType == EAbilityNodeExecutionState::Activating)
		{
			BaseColor = NeAbilityEditorConstants::DebuggerNodeActivatingColor;
		}
		else if (ExecutionType == EAbilityNodeExecutionState::Activated)
		{
			BaseColor = NeAbilityEditorConstants::DebuggerNodeActivedColor;
		}
	}

	return BaseColor;
}

FText SNeAbilityTimelineTrackNode::GetNotifyText() const
{
	return EditorNodeData.GetDisplayText();
}

FText SNeAbilityTimelineTrackNode::GetNodeTooltip() const
{
	return EditorNodeData.GetNodeTooltip();
}


void SNeAbilityTimelineTrackNode::DropCancelled()
{
	bBeingDragged = false;

	//EditorNodeData.Snapped();
}

FVector2D SNeAbilityTimelineTrackNode::ComputeDesiredSize(float) const
{
	return GetSize();
}

bool SNeAbilityTimelineTrackNode::HitTest(const FGeometry& AllottedGeometry, FVector2D MouseLocalPose, int32& OutHitInnerNodeIndex) const
{
	const FVector2D Position = GetWidgetPosition();
	const FVector2D Size = GetSize();

	OutHitInnerNodeIndex = -1;
	const bool bHited = FBox2D(Position, Position + Size).IsInside(MouseLocalPose);
	if (bHited)
	{
		const int32 BaseNodeHeight = NODE_HEIGHT(bIsChildNode);
		float Offset = MouseLocalPose.Y - (Position.Y + BaseNodeHeight);
		while(Offset > 0)
		{
			OutHitInnerNodeIndex ++;
			Offset -= NeAbilityEditorConstants::CompositeTrackHeight;
		}
	}

	return bHited;
}

ENotifyStateHandleHit::Type SNeAbilityTimelineTrackNode::DurationHandleHitTest(const FVector2D& CursorTrackPosition) const
{
	ENotifyStateHandleHit::Type MarkerHit = ENotifyStateHandleHit::None;

	// Make sure this node has a duration box (meaning it is a state node)
	if (NotifyDurationSizeX > 0.0f)
	{
		// Test for mouse inside duration box with handles included
		float ScrubHandleHalfWidth = ScrubHandleSize.X / 2.0f;

		// Position and size of the notify node including the scrub handles
		FVector2D NotifyNodePosition(NotifyScrubHandleCentre - ScrubHandleHalfWidth, 0.0f);
		FVector2D NotifyNodeSize(NotifyDurationSizeX + ScrubHandleHalfWidth * 2.0f, NODE_HEIGHT(bIsChildNode));

		FVector2D MouseRelativePosition(CursorTrackPosition - GetWidgetPosition());

		if( FBox2D(NotifyNodePosition, NotifyNodePosition + NotifyNodeSize).IsInside(MouseRelativePosition) )
		// if (MouseRelativePosition > NotifyNodePosition && MouseRelativePosition < (NotifyNodePosition + NotifyNodeSize))
		{
			// Definitely inside the duration box, need to see which handle we hit if any
			if (MouseRelativePosition.X <= (NotifyNodePosition.X + ScrubHandleSize.X))
			{
				// Left Handle
				MarkerHit = ENotifyStateHandleHit::Start;
			}
			else if (MouseRelativePosition.X >= (NotifyNodePosition.X + NotifyNodeSize.X - ScrubHandleSize.X))
			{
				// Right Handle
				MarkerHit = ENotifyStateHandleHit::End;
			}
		}
	}

	return MarkerHit;
}

void SNeAbilityTimelineTrackNode::UpdateSizeAndPosition(const FGeometry& AllottedGeometry)
{
	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0, 0, AllottedGeometry.Size);

	// Cache the geometry information, the alloted geometry is the same size as the track.
	CachedAllotedGeometrySize = AllottedGeometry.Size * AllottedGeometry.Scale;

	NotifyTimePositionX = ScaleInfo.InputToLocalX(EditorNodeData.GetStartTime());
	NotifyDurationSizeX = ScaleInfo.PixelsPerInput * EditorNodeData.GetDuration();

	FVector2D TextBorderSize = bIsChildNode ? ChildTextBorderSize : DefTextBorderSize;

	const TSharedRef< FSlateFontMeasure > FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	TextSize = FontMeasureService->Measure(GetNotifyText(), Font);
	LabelWidth = TextSize.X + (TextBorderSize.X * 2.f) + (ScrubHandleSize.X / 2.f);

	if (EditorNodeData.IsInstantType())
	{
		//NotifyDurationSizeX = FMath::Max(NotifyDurationSizeX, LabelWidth + 5);
	}

	// Work out where we will have to draw the tool tip
	FVector2D Size = GetSize();
	float LeftEdgeToNotify = NotifyTimePositionX;
	float RightEdgeToNotify = AllottedGeometry.Size.X - NotifyTimePositionX;
	bDrawTooltipToRight = NotifyDurationSizeX > 0.0f || ((RightEdgeToNotify > LabelWidth) || (RightEdgeToNotify > LeftEdgeToNotify));

	// Widget position of the notify marker
	NotifyScrubHandleCentre = bDrawTooltipToRight ? GetScrubHandleWidth() / 2.f : LabelWidth;
}

/** @return the Node's position within the track */
FVector2D SNeAbilityTimelineTrackNode::GetWidgetPosition() const
{
	float WidgetX = bDrawTooltipToRight ? (NotifyTimePositionX - (GetScrubHandleWidth() / 2.f)) : (NotifyTimePositionX - LabelWidth);

	return FVector2D(WidgetX, NotifyHeightOffset);
}

FVector2D SNeAbilityTimelineTrackNode::GetNotifyPosition() const
{
	return FVector2D(NotifyTimePositionX, NotifyHeightOffset);
}

FVector2D SNeAbilityTimelineTrackNode::GetNotifyPositionOffset() const
{
	return GetNotifyPosition() - GetWidgetPosition();
}

float SNeAbilityTimelineTrackNode::GetScrubHandleWidth() const
{
	return FMath::Max(ScrubHandleSize.X, AlignmentMarkerSize.X * 2);
}

float SNeAbilityTimelineTrackNode::GetDesiredHeight() const
{
	int32 Height = NODE_HEIGHT(bIsChildNode);
	// if (UAbilityBeamComposite* TaskComposite = Cast<UAbilityBeamComposite>(TaskNodeData.Get()))
	// {
	// 	Height += TaskComposite->GetVailidTaskNum() * NeAbilityEditorConstants::CompositeTrackHeight;
	// }
	return Height;
}

FVector2D SNeAbilityTimelineTrackNode::GetSize() const
{
	float Width = bDrawTooltipToRight ? (NotifyDurationSizeX > 0.0f ? NotifyDurationSizeX : FMath::Max(LabelWidth, NotifyDurationSizeX)) : (LabelWidth + NotifyDurationSizeX);
	Width += GetScrubHandleWidth();
	float Height = GetDesiredHeight();

	return {Width, Height};
}

int32 SNeAbilityTimelineTrackNode::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 MarkerLayer = LayerId + 1;
	int32 ScrubHandleID = MarkerLayer + 1;
	int32 TextLayerID = ScrubHandleID + 1;
	int32 BranchPointLayerID = TextLayerID + 1;

	const FSlateBrush* StyleInfo = FAppStyle::GetBrush(TEXT("SpecialEditableTextImageNormal"));

	FText Text = GetNotifyText();
	FLinearColor NodeColor = SNeAbilityTimelineTrackNode::GetNodeColor();

	bool bSelectTrack = bSelected && SelectedSubNode == -1;
	FLinearColor BoxColor = bSelectTrack ? FAppStyle::GetSlateColor("SelectionColor").GetSpecifiedColor() : NodeColor;

	float HalfScrubHandleWidth = ScrubHandleSize.X / 2.0f;

	const FVector2D TextBorderSize = bIsChildNode ? ChildTextBorderSize : DefTextBorderSize;

	// 1. Node 底框
	//check(NotifyDurationSizeX > 0);
	FVector2D DurationBoxSize = FVector2D(NotifyDurationSizeX, NODE_HEIGHT(bIsChildNode) - 2 * NodePaddingY);
	FVector2D DurationBoxPosition = FVector2D(NotifyScrubHandleCentre, NodePaddingY);
	//(NODE_HEIGHT(bIsChildNode) - TextSize.Y) * 0.5f - NodePaddingY);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(DurationBoxSize, FSlateLayoutTransform(DurationBoxPosition)),
		StyleInfo,
		ESlateDrawEffect::None,
		BoxColor);

	// 2. Draw Text
	FVector2D LabelSize = TextSize + TextBorderSize * 2.f;
	LabelSize.X += HalfScrubHandleWidth;

	FVector2D LabelPosition(bDrawTooltipToRight ? NotifyScrubHandleCentre : NotifyScrubHandleCentre - LabelSize.X, (NODE_HEIGHT(bIsChildNode)- TextSize.Y) * 0.5f);
	FVector2D TextPosition = LabelPosition + TextBorderSize;
	if (bDrawTooltipToRight)
	{
		TextPosition.X += HalfScrubHandleWidth;
	}
	TextPosition.Y -= NodePaddingY;

	FVector2D DrawTextSize;
	DrawTextSize.X = (NotifyDurationSizeX > 0.0f ? FMath::Min(NotifyDurationSizeX - ScrubHandleSize.X, TextSize.X) : TextSize.X);
	DrawTextSize.Y = TextSize.Y;

	FPaintGeometry TextGeometry = AllottedGeometry.ToPaintGeometry(DrawTextSize, FSlateLayoutTransform(TextPosition));
	OutDrawElements.PushClip(FSlateClippingZone(TextGeometry));

	FSlateDrawElement::MakeText(
		OutDrawElements,
		TextLayerID,
		TextGeometry,
		Text,
		Font,
		ESlateDrawEffect::None,
		FLinearColor::Black
	);

	OutDrawElements.PopClip();

	// Draw Scrub
	if (EditorNodeData.HasDuration() && !bIsDebugger)
	{
		// Left
		DrawScrubHandle(NotifyScrubHandleCentre, OutDrawElements, ScrubHandleID, AllottedGeometry, MyCullingRect, NodeColor);
		// Right
		DrawScrubHandle(DurationBoxPosition.X + DurationBoxSize.X, OutDrawElements, ScrubHandleID, AllottedGeometry, MyCullingRect, NodeColor);
	}

	if (bIsDebugger)
	{
		DrawBreakPoint(NotifyDurationSizeX, OutDrawElements, ScrubHandleID, AllottedGeometry, MyCullingRect, FLinearColor::White);
	}

	return OnPaintCompositeNodes(Args, AllottedGeometry, MyCullingRect, OutDrawElements, TextLayerID, InWidgetStyle, bParentEnabled);
}

int32 SNeAbilityTimelineTrackNode::OnPaintCompositeNodes(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
#if false
	UNeAbilityBeamComposite* TaskComposite = Cast<UNeAbilityBeamComposite>(TaskNodeData.Get());
	if (TaskComposite == nullptr) return LayerId;

	const FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0, 0, CachedAllotedGeometrySize);
	const FSlateBrush* StyleInfo = FAppStyle::GetBrush(TEXT("SpecialEditableTextImageNormal"));
	const TSharedRef< FSlateFontMeasure > FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	float OffsetY = NODE_HEIGHT(bIsChildNode) - NodePaddingY;

	for (int32 Index = 0; Index < TaskComposite->TaskBarriers.Num(); ++ Index)
	{
		const FNeAbilityBeamBarrier& TaskBarrier = TaskComposite->TaskBarriers[Index];
		if (TaskBarrier.InnerTask == nullptr) continue;

		bool bTaskSelected = bSelected && SelectedInnerTaskIndex == Index;
		FLinearColor NodeColor = bTaskSelected ? FAppStyle::GetSlateColor("SelectionColor").GetSpecifiedColor()
			: (TaskBarrier.bEnable ? NeAbilityEditorConstants::CompositeTaskNodeColorEnable : NeAbilityEditorConstants::CompositeTaskNodeColorDisable);

		// Draw Node Backgroup
		const float NodeWidth = ScaleInfo.PixelsPerInput * FMath::Max(TaskBarrier.InnerTask->GetDuration(), 1.f/ EditorNodeData.FrameRate);
		float NodeHeight = NeAbilityEditorConstants::CompositeTrackHeight;
		FVector2D NodePosition = FVector2D(NotifyScrubHandleCentre /*+ GetScrubHandleWidth() / 2*/, OffsetY);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId ++ ,
			AllottedGeometry.ToPaintGeometry(NodePosition, FVector2D(NodeWidth, NodeHeight)),
			StyleInfo,
			ESlateDrawEffect::None,
			NodeColor);

		// Draw Node Text
		FText NodeText = TaskBarrier.InnerTask->GetTaskName();
		FVector2D NodeTextSize = FontMeasureService->Measure(GetNotifyText(), Font);
		FVector2D NodeTextPosition(NodePosition.X + ChildTextBorderSize.X, NodePosition.Y + ChildTextBorderSize.Y);

		FPaintGeometry TextGeometry = AllottedGeometry.ToPaintGeometry(NodeTextPosition, NodeTextSize);
		//OutDrawElements.PushClip(FSlateClippingZone(TextGeometry));

		// 如果字体超出Node的底框
		if (NodeTextSize.X > NodeWidth)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId ++ ,
				AllottedGeometry.ToPaintGeometry(NodeTextPosition, NodeTextSize),
				StyleInfo,
				ESlateDrawEffect::None,
				FLinearColor(0.96f, 0.81f, 0.095f, 0.8f));
		}

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId ++ ,
			TextGeometry,
			NodeText,
			Font,
			ESlateDrawEffect::None,
			FLinearColor::Black
		);

		//OutDrawElements.PopClip();

		OffsetY += NodeHeight + 1;
	}

	// Draw outline border
	float BorderSize = 4.f;
	{
		const FSlateBrush* BorderStyleInfo = FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder"));
		bool bSelectTrack = bSelected && SelectedInnerTaskIndex == -1;
		FLinearColor BorderColor = bSelectTrack ? FAppStyle::GetSlateColor("SelectionColor").GetSpecifiedColor() : SNeAbilityTimelineTrackNode::GetNodeColor();
		FVector2D BorderSizeVertical(BorderSize, GetDesiredHeight() - NodePaddingY);
		FVector2D BorderSizeHorizontal(NotifyDurationSizeX, BorderSize);
		FVector2D BorderPositionLeft = FVector2D(NotifyScrubHandleCentre, NODE_HEIGHT(bIsChildNode) - NodePaddingY);
		FVector2D BorderPositionRight = FVector2D(NotifyScrubHandleCentre + NotifyDurationSizeX - BorderSize, NODE_HEIGHT(bIsChildNode) - NodePaddingY);
		FVector2D BorderPositionBottom = FVector2D(NotifyScrubHandleCentre, GetDesiredHeight() - BorderSize);

#define DRAW_BORDER(Position, Size) \
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId ++ , AllottedGeometry.ToPaintGeometry(Position, Size), StyleInfo, ESlateDrawEffect::None, BorderColor);

		DRAW_BORDER(BorderPositionLeft, BorderSizeVertical);
		DRAW_BORDER(BorderPositionRight, BorderSizeVertical);
		DRAW_BORDER(BorderPositionBottom, BorderSizeHorizontal);
		DRAW_BORDER(BorderPositionLeft, BorderSizeHorizontal);
		//DRAW_BORDER(BorderPositionRight, BorderSizeVertical);
#undef DRAW_BORDER
	}
#endif
	return LayerId;
}

FReply SNeAbilityTimelineTrackNode::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Don't do scrub handle dragging if we haven't captured the mouse.
	//if (!this->HasMouseCapture()) return FReply::Unhandled();

	if (CurrentDragHandle == ENotifyStateHandleHit::None)
	{
		// We've had focus taken away - realease the mouse
		FSlateApplication::Get().ReleaseAllPointerCapture();
		return FReply::Handled();
	}

	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0, 0, CachedAllotedGeometrySize);

	float XPositionInTrack = MyGeometry.AbsolutePosition.X - CachedTrackGeometry.AbsolutePosition.X;
	float TrackScreenSpaceXPosition = MyGeometry.AbsolutePosition.X - XPositionInTrack;
	float TrackScreenSpaceOrigin = CachedTrackGeometry.LocalToAbsolute(FVector2D(ScaleInfo.InputToLocalX(0.0f), 0.0f)).X;
	float TrackScreenSpaceLimit = CachedTrackGeometry.LocalToAbsolute(FVector2D(ScaleInfo.InputToLocalX(TimelinePlayLength), 0.0f)).X;

	if (CurrentDragHandle == ENotifyStateHandleHit::Start)
	{
		// Check track bounds
		float OldDisplayTime = EditorNodeData.GetStartTime();

		if (MouseEvent.GetScreenSpacePosition().X >= TrackScreenSpaceXPosition && MouseEvent.GetScreenSpacePosition().X <= TrackScreenSpaceXPosition + CachedAllotedGeometrySize.X)
		{
			float NewDisplayTime = ScaleInfo.LocalXToInput((FVector2f(MouseEvent.GetScreenSpacePosition()) - MyGeometry.AbsolutePosition + XPositionInTrack).X);
			float NewDuration = EditorNodeData.GetDuration() + OldDisplayTime - NewDisplayTime;

			// Check to make sure the duration is not less than the minimum allowed
			if (NewDuration < MinimumStateDuration)
			{
				NewDisplayTime -= MinimumStateDuration - NewDuration;
			}

			EditorNodeData.SetStartTime(FMath::Max(0.0f, NewDisplayTime));
			EditorNodeData.SetDuration(EditorNodeData.GetDuration() + OldDisplayTime - EditorNodeData.GetStartTime());
		}
		else if (EditorNodeData.GetDuration() > MinimumStateDuration)
		{
			float Overflow = HandleOverflowPan(MouseEvent.GetScreenSpacePosition(), TrackScreenSpaceXPosition, TrackScreenSpaceOrigin, TrackScreenSpaceLimit);

			// Update scale info to the new view inputs after panning
			ScaleInfo.ViewMinInput = ViewInputMin.Get();
			ScaleInfo.ViewMaxInput = ViewInputMax.Get();

			float NewDisplayTime = ScaleInfo.LocalXToInput((FVector2f(MouseEvent.GetScreenSpacePosition()) - MyGeometry.AbsolutePosition + XPositionInTrack).X);
			EditorNodeData.SetStartTime(FMath::Max(0.0f, NewDisplayTime));
			EditorNodeData.SetDuration(EditorNodeData.GetDuration() + OldDisplayTime - EditorNodeData.GetStartTime());

			// Adjust incase we went under the minimum
			if (EditorNodeData.GetDuration() < MinimumStateDuration)
			{
				float EndTimeBefore = EditorNodeData.GetStartTime() + EditorNodeData.GetDuration();
				EditorNodeData.SetStartTime(EditorNodeData.GetStartTime() + EditorNodeData.GetDuration() - MinimumStateDuration);
				EditorNodeData.SetDuration(MinimumStateDuration);
				float EndTimeAfter = EditorNodeData.GetStartTime() + EditorNodeData.GetDuration();
			}
		}
	}
	else
	{
		if (MouseEvent.GetScreenSpacePosition().X >= TrackScreenSpaceXPosition && MouseEvent.GetScreenSpacePosition().X <= TrackScreenSpaceXPosition + CachedAllotedGeometrySize.X)
		{
			float NewDuration = ScaleInfo.LocalXToInput((FVector2f(MouseEvent.GetScreenSpacePosition()) - MyGeometry.AbsolutePosition + XPositionInTrack).X) - EditorNodeData.GetStartTime();

			EditorNodeData.SetDuration(FMath::Max(NewDuration, MinimumStateDuration));
		}
		else if (EditorNodeData.GetDuration() > MinimumStateDuration)
		{
			float Overflow = HandleOverflowPan(MouseEvent.GetScreenSpacePosition(), TrackScreenSpaceXPosition, TrackScreenSpaceOrigin, TrackScreenSpaceLimit);

			// Update scale info to the new view inputs after panning
			ScaleInfo.ViewMinInput = ViewInputMin.Get();
			ScaleInfo.ViewMaxInput = ViewInputMax.Get();

			float NewDuration = ScaleInfo.LocalXToInput((FVector2f(MouseEvent.GetScreenSpacePosition()) - MyGeometry.AbsolutePosition + XPositionInTrack).X) - EditorNodeData.GetStartTime();
			EditorNodeData.SetDuration(FMath::Max(NewDuration, MinimumStateDuration));
		}

		if (EditorNodeData.GetStartTime() + EditorNodeData.GetDuration() > TimelinePlayLength)
		{
			EditorNodeData.SetDuration(TimelinePlayLength - EditorNodeData.GetStartTime());
		}
	}

	return FReply::Handled();
}

FReply SNeAbilityTimelineTrackNode::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (CurrentDragHandle != ENotifyStateHandleHit::None)
		{
			// Clear the drag marker and give the mouse back
			CurrentDragHandle = ENotifyStateHandleHit::None;

			EditorNodeData.Snapped();

			// Signal selection changing so details panels get updated
			OnTrackNodeSelectChanged.ExecuteIfBound(NodeIndex, -1);

			// End drag transaction before handing mouse back
			check(DragMarkerTransactionIdx != INDEX_NONE);
			GEditor->EndTransaction();
			DragMarkerTransactionIdx = INDEX_NONE;

			OnUpdatePanel.ExecuteIfBound();

			return FReply::Handled().ReleaseMouseCapture();
		}
	}

	return FReply::Unhandled();
}

FReply SNeAbilityTimelineTrackNode::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::SetDirectly, true);
}

float SNeAbilityTimelineTrackNode::HandleOverflowPan(const FVector2D& ScreenCursorPos, float TrackScreenSpaceXPosition, float TrackScreenSpaceMin, float TrackScreenSpaceMax)
{
	float Overflow = 0.0f;

	if (ScreenCursorPos.X < TrackScreenSpaceXPosition && TrackScreenSpaceXPosition > TrackScreenSpaceMin - 10.0f)
	{
		// Overflow left edge
		Overflow = FMath::Min(ScreenCursorPos.X - TrackScreenSpaceXPosition, -10.0f);
	}
	else if (ScreenCursorPos.X > CachedAllotedGeometrySize.X && (TrackScreenSpaceXPosition + CachedAllotedGeometrySize.X) < TrackScreenSpaceMax + 10.0f)
	{
		// Overflow right edge
		Overflow = FMath::Max(ScreenCursorPos.X - (TrackScreenSpaceXPosition + CachedAllotedGeometrySize.X), 10.0f);
	}

//	PanTrackRequest.ExecuteIfBound(Overflow, CachedAllotedGeometrySize);

	return Overflow;
}


void SNeAbilityTimelineTrackNode::DrawScrubHandle(float ScrubHandleCentre, FSlateWindowElementList& OutDrawElements, int32 ScrubHandleID, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FLinearColor NodeColour) const
{
	FVector2D ScrubHandlePosition(ScrubHandleCentre - ScrubHandleSize.X / 2.0f, (NODE_HEIGHT(bIsChildNode) - ScrubHandleSize.Y) / 2.f);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		ScrubHandleID,
		AllottedGeometry.ToPaintGeometry(ScrubHandleSize, FSlateLayoutTransform(ScrubHandlePosition)),
		FAppStyle::GetBrush(TEXT("Sequencer.KeyDiamond")),
		ESlateDrawEffect::None,
		NodeColour
	);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		ScrubHandleID,
		AllottedGeometry.ToPaintGeometry(ScrubHandleSize,FSlateLayoutTransform(ScrubHandlePosition)),
		FAppStyle::GetBrush(TEXT("Sequencer.KeyDiamondBorder")),
		ESlateDrawEffect::None,
		bSelected ? FAppStyle::GetSlateColor("SelectionColor").GetSpecifiedColor() : FLinearColor::Black
	);
}

void SNeAbilityTimelineTrackNode::DrawHandleOffset(const float& Offset, const float& HandleCentre, FSlateWindowElementList& OutDrawElements, int32 MarkerLayer, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FLinearColor NodeColor) const
{
	FVector2D MarkerPosition;
	FVector2D MarkerSize = AlignmentMarkerSize;

	if (Offset < 0.f)
	{
		MarkerPosition.Set(HandleCentre - AlignmentMarkerSize.X, (NODE_HEIGHT(bIsChildNode) - AlignmentMarkerSize.Y) / 2.f);
	}
	else
	{
		MarkerPosition.Set(HandleCentre + AlignmentMarkerSize.X, (NODE_HEIGHT(bIsChildNode) - AlignmentMarkerSize.Y) / 2.f);
		MarkerSize.X = -AlignmentMarkerSize.X;
	}

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		MarkerLayer,
		AllottedGeometry.ToPaintGeometry(MarkerSize, FSlateLayoutTransform(MarkerPosition)),
		FAppStyle::GetBrush(TEXT("Sequencer.Timeline.NotifyAlignmentMarker")),
		ESlateDrawEffect::None,
		NodeColor
	);
}

void SNeAbilityTimelineTrackNode::DrawBreakPoint(float ScrubHandleCentre, FSlateWindowElementList& OutDrawElements, int32 ScrubHandleID, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FLinearColor NodeColour) const
{
	if (!WeakTimelineMode.IsValid())
		return;

	bool bIsBreakpointEnabled = WeakTimelineMode.Pin()->HasBreakpointToggled(EditorNodeData.GetNodeData());
	bool bDebuggerMarkBreakpointTrigger = WeakTimelineMode.Pin()->IsBreakpointTriggered(EditorNodeData.GetNodeData());
	if (!bIsBreakpointEnabled && !bDebuggerMarkBreakpointTrigger)
		return;

	if (bIsBreakpointEnabled)
	{
		FOverlayBrushInfo BreakpointOverlayInfo;
		BreakpointOverlayInfo.Brush = FAppStyle::GetBrush(TEXT("Kismet.Breakpoint.EnabledAndValid"));

		FVector2D ImageSize = BreakpointOverlayInfo.Brush->ImageSize ;
		FPaintGeometry BouncedGeometry = AllottedGeometry.ToPaintGeometry(ImageSize, FSlateLayoutTransform(FVector2D(0, ImageSize.Y /2 - 4.0f)));

		FLinearColor FinalColorAndOpacity = FLinearColor::Red;
		FinalColorAndOpacity.A = 0.8f;

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			ScrubHandleID,
			BouncedGeometry,
			BreakpointOverlayInfo.Brush,
			ESlateDrawEffect::None,
			FinalColorAndOpacity
		);
	}

	if(bIsBreakpointEnabled && bDebuggerMarkBreakpointTrigger)
	{
		FOverlayBrushInfo BreakpointOverlayInfo;
		BreakpointOverlayInfo.Brush = FAppStyle::GetBrush(TEXT("BTEditor.DebuggerOverlay.BreakOnBreakpointPointer"));

		FVector2D ImageSize = BreakpointOverlayInfo.Brush->ImageSize * 0.2f;
		FPaintGeometry BouncedGeometry = AllottedGeometry.ToPaintGeometry(ImageSize, FSlateLayoutTransform(FVector2D(ScrubHandleCentre /  2,  0)));

		FLinearColor FinalColorAndOpacity = FLinearColor::Red;

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			ScrubHandleID,
			BouncedGeometry,
			BreakpointOverlayInfo.Brush,
			ESlateDrawEffect::None,
			FinalColorAndOpacity
		);
	}
}

void SNeAbilityTimelineTrackNode::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	ScreenPosition = FVector2D(AllottedGeometry.AbsolutePosition);
}

void SNeAbilityTimelineTrackNode::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	if (CurrentDragHandle != ENotifyStateHandleHit::None)
	{
		// Lost focus while dragging a state node, clear the drag and end the current transaction
		CurrentDragHandle = ENotifyStateHandleHit::None;

		check(DragMarkerTransactionIdx != INDEX_NONE);
		GEditor->EndTransaction();
		DragMarkerTransactionIdx = INDEX_NONE;
	}
}

bool SNeAbilityTimelineTrackNode::SupportsKeyboardFocus() const
{
	// Need to support focus on the node so we can end drag transactions if the user alt-tabs
	// from the editor while in the proceess of dragging a state notify duration marker.
	return true;
}

FCursorReply SNeAbilityTimelineTrackNode::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	// Show resize cursor if the cursor is hoverring over either of the scrub handles of a notify state node
	if (IsHovered() && GetDurationSize() > 0.0f && EditorNodeData.HasDuration())
	{
		FVector2D RelMouseLocation = MyGeometry.AbsoluteToLocal(CursorEvent.GetScreenSpacePosition());

		const float HandleHalfWidth = ScrubHandleSize.X / 2.0f;
		const float DistFromFirstHandle = FMath::Abs(RelMouseLocation.X - NotifyScrubHandleCentre);
		const float DistFromSecondHandle = FMath::Abs(RelMouseLocation.X - (NotifyScrubHandleCentre + NotifyDurationSizeX));

		if (DistFromFirstHandle < HandleHalfWidth || DistFromSecondHandle < HandleHalfWidth || CurrentDragHandle != ENotifyStateHandleHit::None)
		{
			return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
		}
	}
	return FCursorReply::Unhandled();
}


#undef LOCTEXT_NAMESPACE
