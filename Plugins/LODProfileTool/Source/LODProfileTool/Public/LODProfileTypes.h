// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LODProfileTypes.generated.h"

/**
 * Describes the full set of parameters needed to apply a LOD profile to meshes.
 */
USTRUCT(BlueprintType)
struct FLODProfile
{
	GENERATED_BODY()

	FLODProfile()
		: NumLODs(1)
		, bEnableReduction(false)
		, bOverrideExisting(true)
	{
		ScreenSizes.Add(1.0f);
		ReductionPercents.Add(100.0f);
	}

	/** Desired total number of LODs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	int32 NumLODs;

	/** Screen size threshold per LOD (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	TArray<float> ScreenSizes;

	/** Triangle percentage per LOD (0-100). Only applied when reduction is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	TArray<float> ReductionPercents;

	/** Whether triangle reduction should be applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bEnableReduction;

	/** When false, existing LODs are preserved and only missing LODs are added. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bOverrideExisting;
};
