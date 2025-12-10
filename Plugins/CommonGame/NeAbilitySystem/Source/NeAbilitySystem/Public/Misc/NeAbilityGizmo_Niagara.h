// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeAbilityGizmoActor.h"
#include "NeAbilityGizmo_Niagara.generated.h"

UCLASS()
class NEABILITYSYSTEM_API ANeAbilityGizmo_Niagara : public ANeAbilityTransformGizmo
{
	GENERATED_BODY()

public:
	ANeAbilityGizmo_Niagara();

	//~BEGIN: ANeAbilityTransformGizmo interface
	virtual void OnSynchronizeFromBinding() override;
	virtual void OnGizmoMoved() override;
	//~END: ANeAbilityTransformGizmo interface

protected:
	void UpdatePreviewNiagara(const FTransform& NewTransform) const;
};
