// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LODProfileTypes.h"
#include "LODProfileToolSettings.generated.h"

/**
 * Project settings for the LOD Profile Tool, exposed under Plugins.
 */
UCLASS(Config = LODProfileTool, DefaultConfig)
class ULODProfileToolSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	ULODProfileToolSettings();

	/** UDeveloperSettings interface */
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FText GetSectionText() const override;

	/** Returns the settings singleton. */
	static const ULODProfileToolSettings* Get();

	/** Builds a profile object from current settings. */
	FLODProfile BuildProfile() const;

	/** Default number of LODs to generate. */
	UPROPERTY(EditAnywhere, Config, Category = "Profile", meta = (ClampMin = "1", ClampMax = "8"))
	int32 DefaultNumLODs;

	/** Screen size thresholds per LOD (0-1, descending). */
	UPROPERTY(EditAnywhere, Config, Category = "Profile", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	TArray<float> DefaultScreenSizes;

	/** Triangle percentages per LOD (0-100). */
	UPROPERTY(EditAnywhere, Config, Category = "Profile", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	TArray<float> DefaultReductionPercents;

	/** Enables triangle reduction when true. */
	UPROPERTY(EditAnywhere, Config, Category = "Behavior")
	bool bDefaultEnableReduction;

	/** When false, existing LODs are preserved and only missing ones are added. */
	UPROPERTY(EditAnywhere, Config, Category = "Behavior")
	bool bDefaultOverrideExisting;

	/** Whether to warn when overriding existing LODs. */
	UPROPERTY(EditAnywhere, Config, Category = "Behavior")
	bool bWarnOnOverwrite;

	/** Automatically save modified assets after applying a profile. */
	UPROPERTY(EditAnywhere, Config, Category = "Behavior")
	bool bAutoSaveAssets;
};
