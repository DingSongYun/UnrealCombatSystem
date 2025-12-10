// Copyright Epic Games, Inc. All Rights Reserved.

#include "SCommonPreLoadingScreenWidget.h"

#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "SCommonPreLoadingScreenWidget"

void SCommonPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	if (InArgs._Image)
	{
		LoadingImageBrush = FSlateBrush();
		LoadingImageBrush.SetResourceObject(InArgs._Image);
		LoadingImageBrush.ImageSize = FVector2D(InArgs._Image->GetSizeX(), InArgs._Image->GetSizeY());
	}
	else
	{
		LoadingImageBrush = FSlateOptionalBrush();
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0)
		.Content()
		[
			LoadingImageBrush.IsSet() ? SNew(SImage).Image(&LoadingImageBrush) : SNullWidget::NullWidget
		]
	];
}

void SCommonPreLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	//WidgetAssets.AddReferencedObjects(Collector);
}

FString SCommonPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SCommonPreLoadingScreenWidget");
}

#undef LOCTEXT_NAMESPACE