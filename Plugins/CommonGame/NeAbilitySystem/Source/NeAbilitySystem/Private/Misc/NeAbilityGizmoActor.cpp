// Copyright NetEase Games, Inc. All Rights Reserved.


#include "Misc/NeAbilityGizmoActor.h"

#include "NeAbilitySegmentEvalQueue.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"

ANeAbilityGizmoActor::ANeAbilityGizmoActor(const FObjectInitializer& Initializer) : Super(Initializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bSyncOnTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent->Mobility = EComponentMobility::Movable;

	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		if (GetOwner())	RootComponent->RegisterComponent();
		RootComponent->TransformUpdated.AddUObject(this, &ANeAbilityGizmoActor::OnTransformUpdated);
	}
}

void ANeAbilityGizmoActor::InitializeFor(const FWeakAbilitySegmentPtr& InSegmentPtr, const FGizmoArgs& Args)
{
	BindingSegment = InSegmentPtr;
	EvalContext = Args.EvalContext;
	PreviewPlayer = Args.PreviewPlayer;
	PreviewTarget = Args.PreviewTarget;
	ReceiveInitializeFor(BindingSegment.Get());
	SynchronizeFromBinding();
}

void ANeAbilityGizmoActor::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (bSyncOnTick)
	{
		SynchronizeFromBinding();
	}
}

void ANeAbilityGizmoActor::PostTaskChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (bSyncOnTick) return ;

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	const FName MemberPropertyName = PropertyChangedEvent.GetMemberPropertyName();
	for(FName Property : ObservedProperties)
	{
		if (PropertyName == Property || MemberPropertyName == Property)
		{
			SynchronizeFromBinding();
			break;
		}
	}
}

const FNeAbilitySegmentEvalContext& ANeAbilityGizmoActor::GetEvalContext() const
{
	FNeAbilitySegmentEvalContext* SegmentEvalContext = EvalContext.Get();
	if (SegmentEvalContext) return *SegmentEvalContext;

	static FNeAbilitySegmentEvalContext EvalContext_Invalid;
	return EvalContext_Invalid;
}

AActor* ANeAbilityGizmoActor::GetPreviewPlayer() const
{
	return PreviewPlayer.Get();
}

AActor* ANeAbilityGizmoActor::GetPreviewTarget() const
{
	return PreviewTarget.Get();
}

void ANeAbilityGizmoActor::OnTransformUpdated(USceneComponent* InRootComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	if (!bLockGizmoMove)
	{
		NotifyGizmoMoved();
	}
}

void ANeAbilityGizmoActor::SynchronizeFromBinding()
{
	if (bLockGizmoSync) return;

	bLockGizmoMove = true;
	OnSynchronizeFromBinding();
	ReceiveSynchronizeFromBinding();
	bLockGizmoMove = false;
}

void ANeAbilityGizmoActor::NotifyGizmoMoved()
{
	bLockGizmoSync = true;
	OnGizmoMoved();
	ReceiveOnGizmoMoved();
	bLockGizmoSync = false;
}

/**
 * ANeAbilityTransformGizmo
 */
ANeAbilityTransformGizmo::ANeAbilityTransformGizmo(const FObjectInitializer& Initializer) : Super(Initializer)
{
#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> GizmoTextureObject;
			FName ID;
			FText NAME;
			FConstructorStatics()
				: GizmoTextureObject(TEXT("/Engine/EditorResources/EmptyActor"))
				, ID(TEXT("TransformGizmo"))
				, NAME(NSLOCTEXT( "SpriteCategory", "Gizmo", "TransformGizmo" ))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.GizmoTextureObject.Get();
		SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->bHiddenInGame = false;
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID;
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->SetupAttachment(RootComponent);
	}

	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		ArrowComponent->SetupAttachment(RootComponent);
	}
#endif
}
