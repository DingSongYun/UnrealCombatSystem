// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAbilitySystemModule.h"

#define LOCTEXT_NAMESPACE "FNeAbilitySystemModule"

class FNeAbilitySystemModule : public INeAbilitySystemModule
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{

	}

	virtual void ShutdownModule() override
	{
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNeAbilitySystemModule, NeAbilitySystem)