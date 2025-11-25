// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "LODProfileTypes.h"

class UStaticMesh;

/**
 * Stateless helper that applies LOD profiles to static meshes.
 */
class FLODProfileApplicator
{
public:
	/** Applies the profile to the provided assets. Returns number of successes. */
	static int32 ApplyProfileToAssets(const FLODProfile& Profile, const TArray<FAssetData>& Assets, bool bAutoSave);

	/** Opens the editor for the first selected static mesh, if any. */
	static void OpenPreviewForAssets(const TArray<FAssetData>& Assets);

	/** Validates the profile and returns an error string if invalid. */
	static bool ValidateProfile(const FLODProfile& Profile, FString& OutError);

private:
	static bool ApplyProfileToMesh(UStaticMesh* Mesh, const FLODProfile& Profile, bool bAutoSave);
	static bool SaveMesh(UStaticMesh* Mesh);
};
