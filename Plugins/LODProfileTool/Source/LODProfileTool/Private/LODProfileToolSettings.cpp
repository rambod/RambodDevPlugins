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

	// Ensure arrays have at least NumLODs entries; fill missing slots with sensible fallbacks
	if (Profile.ScreenSizes.Num() < Profile.NumLODs)
	{
		for (int32 Index = Profile.ScreenSizes.Num(); Index < Profile.NumLODs; ++Index)
		{
			const float Fallback = Index == 0 ? 1.0f : FMath::Max(0.0f, 1.0f - Index * 0.25f);
			Profile.ScreenSizes.Add(Fallback);
		}
	}

	if (Profile.ReductionPercents.Num() < Profile.NumLODs)
	{
		static const float DefaultPercents[] = { 100.0f, 60.0f, 35.0f };
		for (int32 Index = Profile.ReductionPercents.Num(); Index < Profile.NumLODs; ++Index)
		{
			const float Fallback = Index < UE_ARRAY_COUNT(DefaultPercents) ? DefaultPercents[Index] : 100.0f;
			Profile.ReductionPercents.Add(Fallback);
		}
	}

	// Clamp ranges
	for (float& Size : Profile.ScreenSizes)
	{
		Size = FMath::Clamp(Size, 0.0f, 1.0f);
	}
	for (float& Percent : Profile.ReductionPercents)
	{
		Percent = FMath::Clamp(Percent, 0.0f, 100.0f);
	}
	return Profile;
}
