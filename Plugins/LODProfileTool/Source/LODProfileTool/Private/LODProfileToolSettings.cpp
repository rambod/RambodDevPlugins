// Copyright Epic Games, Inc. All Rights Reserved.

#include "LODProfileToolSettings.h"

#include "LODProfileTool.h"

ULODProfileToolSettings::ULODProfileToolSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("LOD Profile Tool");

	DefaultNumLODs = 3;
	bDefaultEnableReduction = true;
	bDefaultOverrideExisting = true;
	bWarnOnOverwrite = true;
	bAutoSaveAssets = false;

	// Reasonable defaults
	DefaultScreenSizes = {1.0f, 0.5f, 0.25f};
	DefaultReductionPercents = {100.0f, 60.0f, 35.0f};
}

FText ULODProfileToolSettings::GetSectionText() const
{
	return NSLOCTEXT("LODProfileTool", "SettingsSection", "LOD Profile Tool");
}

const ULODProfileToolSettings* ULODProfileToolSettings::Get()
{
	return GetDefault<ULODProfileToolSettings>();
}

FLODProfile ULODProfileToolSettings::BuildProfile() const
{
	FLODProfile Profile;
	Profile.NumLODs = DefaultNumLODs;
	Profile.ScreenSizes = DefaultScreenSizes;
	Profile.ReductionPercents = DefaultReductionPercents;
	Profile.bEnableReduction = bDefaultEnableReduction;
	Profile.bOverrideExisting = bDefaultOverrideExisting;
	return Profile;
}
