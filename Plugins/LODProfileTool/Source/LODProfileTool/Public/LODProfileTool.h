// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLODProfileTool, Log, All);

class SDockTab;
class FSpawnTabArgs;

class FLODProfileToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Opens the main LOD Profile tab. */
	void InvokeTab();

private:
	void RegisterMenus();
	TSharedRef<SDockTab> SpawnLODProfileTab(const FSpawnTabArgs& Args);
};
