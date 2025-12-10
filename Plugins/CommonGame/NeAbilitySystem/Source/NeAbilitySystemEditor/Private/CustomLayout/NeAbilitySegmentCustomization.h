// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"

class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IPropertyHandle;
class IDetailCategoryBuilder;

class FNeAbilitySegmentCustomization : public IDetailCustomization
{
public:
	static FName GetTypeName();
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FNeAbilitySegmentCustomization());
	}

	bool CustomizeProperty(IDetailCategoryBuilder& CategoryBuilder, class UNeAbilityBeam* Beam, TSharedPtr<IPropertyHandle> Property);
	bool CustomizeGameplayTaskTemplate(IDetailCategoryBuilder& CategoryBuilder, class UAbilityTask* TaskTemplate, TSharedPtr<IPropertyHandle> Property);

private:
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder );
};
