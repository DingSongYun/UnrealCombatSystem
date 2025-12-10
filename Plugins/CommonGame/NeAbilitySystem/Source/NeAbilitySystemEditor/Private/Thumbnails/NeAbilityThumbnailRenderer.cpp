// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilityThumbnailRenderer.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "CanvasTypes.h"
#include "GameplayAbilityBlueprint.h"
#include "NeAbility.h"
#include "Animation/Skeleton.h"
#include "Beams/NeAbilityBeam_GameplayTask.h"
#include "Engine/Font.h"
#include "Styling/CoreStyle.h"
#include "Tasks/NeAbilityTask_PlayAnimation.h"
#include "UObject/Package.h"

ANeAbilityThumbnailPreviewActor::ANeAbilityThumbnailPreviewActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDebugSkelMeshComponent>(TEXT("SkeletalMeshComponent0")))
{

}

FNeAbilityThumbnailScene::FNeAbilityThumbnailScene() : FThumbnailPreviewScene()
{
	bForceAllUsedMipsResident = false;
	// Create preview actor
	// checked
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bNoFail = true;
	SpawnInfo.ObjectFlags = RF_Transient;
	PreviewActor = GetWorld()->SpawnActor<ANeAbilityThumbnailPreviewActor>(SpawnInfo);

	PreviewActor->SetActorEnableCollision(false);
}

bool FNeAbilityThumbnailScene::SetAbility(class UNeAbility* InAbility)
{
	PreviewActor->GetSkeletalMeshComponent()->OverrideMaterials.Empty();

	bool bSetSucessfully = false;

	PreviewAbility = InAbility;
	UAnimMontage* AbilityAnimation = nullptr;
	if (PreviewAbility)
	{
		FNeAbilitySection& Section = PreviewAbility->GetSection(0);
		for (FNeAbilitySegment& Segment : Section.Segments)
		{
			const UNeAbilityBeam_GameplayTask* TaskBeam = Cast<UNeAbilityBeam_GameplayTask>(Segment.GetAbilityBeam());
			if (const UNeAbilityTask_PlayAnimation* AnimationTask = TaskBeam ? Cast<UNeAbilityTask_PlayAnimation>(TaskBeam->TaskTemplate) : nullptr)
			{
				AbilityAnimation = AnimationTask->MontageAsset;
				break;
			}
		}
	}

	if (AbilityAnimation)
	{
		if (USkeleton* Skeleton = AbilityAnimation->GetSkeleton())
		{
			USkeletalMesh* PreviewSkeletalMesh = Skeleton->GetAssetPreviewMesh(AbilityAnimation);
			if (!PreviewSkeletalMesh)
			{
				PreviewSkeletalMesh = Skeleton->FindCompatibleMesh();
			}
			PreviewActor->GetSkeletalMeshComponent()->SetSkeletalMesh(PreviewSkeletalMesh);

			if (PreviewSkeletalMesh)
			{
				bSetSucessfully = true;

				if (AbilityAnimation->IsValidToPlay())
				{
					// Handle posing the mesh at the middle of the animation
					const float AnimPosition = AbilityAnimation->GetPlayLength() / 2.f;

					UDebugSkelMeshComponent* MeshComponent = CastChecked<UDebugSkelMeshComponent>(PreviewActor->GetSkeletalMeshComponent());

					MeshComponent->EnablePreview(true, AbilityAnimation);
					MeshComponent->Play(false);
					MeshComponent->Stop();
					MeshComponent->SetPosition(AnimPosition, false);

					UAnimSingleNodeInstance* SingleNodeInstance = PreviewActor->GetSkeletalMeshComponent()->GetSingleNodeInstance();
					if (SingleNodeInstance)
					{
						SingleNodeInstance->UpdateMontageWeightForTimeSkip(AnimPosition);
					}

					PreviewActor->GetSkeletalMeshComponent()->RefreshBoneTransforms(nullptr);
				}

				PreviewActor->SetActorLocation(FVector(0, 0, 0), false);
				PreviewActor->GetSkeletalMeshComponent()->UpdateBounds();

				// Center the mesh at the world origin then offset to put it on top of the plane
				const float BoundsZOffset = GetBoundsZOffset(PreviewActor->GetSkeletalMeshComponent()->Bounds);
				PreviewActor->SetActorLocation(-PreviewActor->GetSkeletalMeshComponent()->Bounds.Origin + FVector(0, 0, BoundsZOffset), false);
				PreviewActor->GetSkeletalMeshComponent()->RecreateRenderState_Concurrent();
			}
		}
	}

	if(!bSetSucessfully)
	{
		CleanupComponentChildren(PreviewActor->GetSkeletalMeshComponent());
		PreviewActor->GetSkeletalMeshComponent()->SetAnimation(NULL);
		PreviewActor->GetSkeletalMeshComponent()->SetSkeletalMesh(nullptr);
	}

	return bSetSucessfully;

}

void FNeAbilityThumbnailScene::GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom) const
{
	check(PreviewAbility);
	check(PreviewActor->GetSkeletalMeshComponent());
	check(PreviewActor->GetSkeletalMeshComponent()->GetSkeletalMeshAsset());

	const float HalfFOVRadians = FMath::DegreesToRadians<float>(InFOVDegrees) * 0.5f;
	const float HalfMeshSize = PreviewActor->GetSkeletalMeshComponent()->Bounds.SphereRadius;
	const float BoundsZOffset = GetBoundsZOffset(PreviewActor->GetSkeletalMeshComponent()->Bounds);
	const float TargetDistance = HalfMeshSize / FMath::Tan(HalfFOVRadians);

	// USceneThumbnailInfo* ThumbnailInfo = Cast<USceneThumbnailInfo>(PreviewAnimation->ThumbnailInfo);
	USceneThumbnailInfo* ThumbnailInfo = USceneThumbnailInfo::StaticClass()->GetDefaultObject<USceneThumbnailInfo>();
	float OrbitZoom = ThumbnailInfo->OrbitZoom;
	if (TargetDistance + OrbitZoom < 0)
	{
		OrbitZoom = -TargetDistance;
	}

	OutOrigin = FVector(0, 0, -BoundsZOffset);
	OutOrbitPitch = ThumbnailInfo->OrbitPitch;
	OutOrbitYaw = ThumbnailInfo->OrbitYaw;
	OutOrbitZoom = TargetDistance + OrbitZoom;
}

void FNeAbilityThumbnailScene::CleanupComponentChildren(USceneComponent* Component)
{
	if (Component)
	{
		for(int32 ComponentIdx = Component->GetAttachChildren().Num() - 1 ; ComponentIdx >= 0 ; --ComponentIdx)
		{
			CleanupComponentChildren(Component->GetAttachChildren()[ComponentIdx]);
			Component->GetAttachChildren()[ComponentIdx]->DestroyComponent();
		}
		check(Component->GetAttachChildren().Num() == 0);
	}
}

UNeAbilityThumbnailRenderer::UNeAbilityThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> TextureObject;
		UFont* NameFont = nullptr;
		FConstructorStatics()
			: TextureObject(TEXT("/Engine/EditorMaterials/ParticleSystems/PSysThumbnail_NoImage"))
		{
			NameFont = NewObject<UFont>(GetTransientPackage(), NAME_None, RF_Transient);
			NameFont->FontCacheType = EFontCacheType::Runtime;
			NameFont->LegacyFontSize = 27.0f;
			NameFont->CompositeFont = *FCoreStyle::GetDefaultFont();
		}
	};
	static FConstructorStatics ConstructorStatics;

	TitleImage = ConstructorStatics.TextureObject.Get();
	NameFont = ConstructorStatics.NameFont;
	ThumbnailScene = nullptr;
}

void UNeAbilityThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UGameplayAbilityBlueprint* AbilityBlueprintToRender = Cast<UGameplayAbilityBlueprint>(Object);
	const bool bIsBlueprintValid = IsValid(AbilityBlueprintToRender)
		&& IsValid(AbilityBlueprintToRender->GeneratedClass)
		&& AbilityBlueprintToRender->bHasBeenRegenerated
		&& !AbilityBlueprintToRender->bBeingCompiled
		&& !AbilityBlueprintToRender->HasAnyFlags(RF_Transient)
		&& !AbilityBlueprintToRender->GeneratedClass->HasAnyClassFlags(CLASS_Deprecated | CLASS_Abstract)
		&& AbilityBlueprintToRender->GeneratedClass->IsChildOf(UNeAbility::StaticClass());
	if (!bIsBlueprintValid)
	{
		return;
	}
	if (UNeAbility* AbilityAsset = AbilityBlueprintToRender->GeneratedClass->GetDefaultObject<UNeAbility>())
	{
		if ( ThumbnailScene == nullptr )
		{
			ThumbnailScene = new FNeAbilityThumbnailScene();
		}

		if(ThumbnailScene->SetAbility(AbilityAsset))
		{
			FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
				.SetTime(UThumbnailRenderer::GetTime())
				.SetAdditionalViewFamily(bAdditionalViewFamily));

			ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
			ViewFamily.EngineShowFlags.MotionBlur = 0;
			ViewFamily.EngineShowFlags.LOD = 0;

			RenderViewFamily(Canvas, &ViewFamily, ThumbnailScene->CreateView(&ViewFamily, X, Y, Width, Height));
			ThumbnailScene->SetAbility(nullptr);
		}


		// Canvas->DrawTile(X, Y, Width, Height, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor::Black, nullptr, false);
		static FLinearColor LinearColor = FLinearColor::White;
		static FLinearColor ShadowColor = FColor(235, 177, 77);

		const int32 LineHeight = FMath::TruncToInt(NameFont->GetMaxCharHeight());
		FText DisplayName = AbilityAsset->Name.IsEmpty() ? AbilityAsset->GetClass()->GetDisplayNameText() : AbilityAsset->Name;
		const int32 MessageWidth = NameFont->GetStringSize(*DisplayName.ToString());
		float Xpos = FMath::Max(((int32)Width - MessageWidth) / 2, 0.f);
		Canvas->DrawShadowedText(Xpos, LineHeight*1.5, DisplayName, NameFont, LinearColor, ShadowColor);
	}
}

void UNeAbilityThumbnailRenderer::BeginDestroy()
{
	if ( ThumbnailScene != nullptr )
	{
		delete ThumbnailScene;
		ThumbnailScene = nullptr;
	}

	Super::BeginDestroy();
}