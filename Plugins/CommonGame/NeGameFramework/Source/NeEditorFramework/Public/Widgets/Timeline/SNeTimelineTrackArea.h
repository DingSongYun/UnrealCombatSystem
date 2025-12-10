// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Geometry.h"
#include "Input/CursorReply.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SlotBase.h"
#include "Layout/Children.h"
#include "Widgets/SPanel.h"

class FArrangedChildren;
class FPaintArgs;
class FSlateWindowElementList;
class FNeTimelineTrack;
class SNeTimelineTrack;
class SNeTimelineOutliner;
class FNeTimelineMode;
class FNeTimeSliderController;

/**
 * Structure representing a slot in the track area.
 */
class NEEDITORFRAMEWORK_API FTimelineTrackAreaSlot : public TSlotBase<FTimelineTrackAreaSlot>, public TAlignmentWidgetSlotMixin<FTimelineTrackAreaSlot>
{
public:

	/** Construction from a track lane */
	FTimelineTrackAreaSlot(const TSharedPtr<SNeTimelineTrack>& InSlotContent);

	/** Get the vertical position of this slot inside its parent. */
	float GetVerticalOffset() const;

	/** The track that we represent. */
	TWeakPtr<SNeTimelineTrack> TrackWidget;
};


/**
 * The area where tracks are displayed.
 */
class SNeTimelineTrackArea : public SPanel
{
public:

	SLATE_BEGIN_ARGS(SNeTimelineTrackArea)
	{
		_Clipping = EWidgetClipping::ClipToBounds;
	}
	SLATE_END_ARGS()

	SNeTimelineTrackArea()
		: Children(this)
	{}

	/** Construct this widget */
	void Construct(const FArguments& InArgs, const TSharedRef<FNeTimelineMode>& InTimelineMode, const TSharedRef<FNeTimeSliderController>& InTimeSliderController);

public:

	/** Empty the track area */
	void Empty();

	/** Add a new track slot to this area for the given node. The slot will be automatically cleaned up when all external references to the supplied slot are removed. */
	void AddTrackSlot(const TSharedRef<FNeTimelineTrack>& InTrack, const TSharedPtr<SNeTimelineTrack>& InTrackWidget);

	/** Attempt to find an existing slot relating to the given node */
	TSharedPtr<SNeTimelineTrack> FindTrackSlot(const TSharedRef<FNeTimelineTrack>& InTrack);

	/** Access the cached geometry for this track area */
	const FGeometry& GetCachedGeometry() const
	{
		return CachedGeometry;
	}

	/** Assign an outliner to this track area. */
	void SetOutliner(const TSharedPtr<SNeTimelineOutliner>& InOutliner);

public:

	/** SWidget interface */
	virtual FReply OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	virtual FCursorReply OnCursorQuery( const FGeometry& MyGeometry, const FPointerEvent& CursorEvent ) const override;
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	virtual void OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:

	/** Update any hover state required for the track area */
	void UpdateHoverStates(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	virtual void AddNewTrack(class FMenuBuilder& MenuBuilder);
	virtual void SortTrack();
private:

	/** The track area's children. */
	TPanelChildren<FTimelineTrackAreaSlot> Children;

private:

	/** Cached geometry. */
	FGeometry CachedGeometry;

	/** A map of child slot content that exists in our view. */
	TMap<TWeakPtr<FNeTimelineTrack>, TWeakPtr<SNeTimelineTrack>> TrackSlots;

	/** Weak pointer to the model. */
	TWeakPtr<FNeTimelineMode> WeakTimelineMode;

	/** Weak pointer to the time slider. */
	TWeakPtr<FNeTimeSliderController> WeakTimeSliderController;

	/** Weak pointer to the outliner (used for scrolling interactions). */
	TWeakPtr<SNeTimelineOutliner> WeakOutliner;
};
