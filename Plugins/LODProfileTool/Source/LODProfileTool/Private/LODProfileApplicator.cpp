// Copyright Epic Games, Inc. All Rights Reserved.

#include "LODProfileApplicator.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "LODProfileTool.h"
#include "LODProfileToolSettings.h"
#include "Misc/ScopedSlowTask.h"
#include "StaticMeshResources.h"
#include "UObject/Package.h"
#include "Editor.h"
#include "EditorFramework/AssetImportData.h"
#include "FileHelpers.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Engine/StaticMesh.h"

namespace
{
	float GetScreenSize(const FLODProfile& Profile, int32 LODIndex)
	{
		if (Profile.ScreenSizes.IsValidIndex(LODIndex))
		{
			return Profile.ScreenSizes[LODIndex];
		}
		// Fallback: decreasing sizes
		return LODIndex == 0 ? 1.0f : FMath::Max(0.0f, 1.0f - (LODIndex * 0.25f));
	}

	float GetReductionPercent(const FLODProfile& Profile, int32 LODIndex)
	{
		if (Profile.ReductionPercents.IsValidIndex(LODIndex))
		{
			return Profile.ReductionPercents[LODIndex];
		}
		return 100.0f;
	}
}

bool FLODProfileApplicator::ValidateProfile(const FLODProfile& Profile, FString& OutError)
{
	if (Profile.NumLODs <= 0)
	{
		OutError = TEXT("Number of LODs must be greater than zero.");
		return false;
	}

	if (Profile.ScreenSizes.Num() < Profile.NumLODs)
	{
		OutError = TEXT("ScreenSizes should contain at least as many entries as NumLODs.");
		return false;
	}

	for (int32 Index = 0; Index < Profile.NumLODs; ++Index)
	{
		const float Size = Profile.ScreenSizes[Index];
		if (Size < 0.0f || Size > 1.0f)
		{
			OutError = FString::Printf(TEXT("Screen size at index %d is out of range [0,1]."), Index);
			return false;
		}
	}

	if (Profile.bEnableReduction)
	{
		if (Profile.ReductionPercents.Num() < Profile.NumLODs)
		{
			OutError = TEXT("ReductionPercents should contain at least as many entries as NumLODs.");
			return false;
		}
		for (int32 Index = 0; Index < Profile.NumLODs; ++Index)
		{
			const float Percent = Profile.ReductionPercents[Index];
			if (Percent < 0.0f || Percent > 100.0f)
			{
				OutError = FString::Printf(TEXT("Reduction percent at index %d is out of range [0,100]."), Index);
				return false;
			}
		}
	}

	return true;
}

int32 FLODProfileApplicator::ApplyProfileToAssets(const FLODProfile& Profile, const TArray<FAssetData>& Assets, bool bAutoSave)
{
	int32 Successes = 0;
	FScopedSlowTask Progress(Assets.Num(), NSLOCTEXT("LODProfileTool", "ApplyProfileProgress", "Applying LOD Profile..."));
	const bool bShowProgress = Assets.Num() > 3;
	if (bShowProgress)
	{
		Progress.MakeDialog(/*AllowCancel=*/false);
	}

	for (const FAssetData& AssetData : Assets)
	{
		if (bShowProgress)
		{
			Progress.EnterProgressFrame(1.f, FText::FromName(AssetData.AssetName));
		}

		UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
		if (!Mesh)
		{
			UE_LOG(LogLODProfileTool, Warning, TEXT("Skipping asset %s (not a static mesh)."), *AssetData.AssetName.ToString());
			continue;
		}

		if (ApplyProfileToMesh(Mesh, Profile, bAutoSave))
		{
			++Successes;
		}
	}
	return Successes;
}

bool FLODProfileApplicator::ApplyProfileToMesh(UStaticMesh* Mesh, const FLODProfile& Profile, bool bAutoSave)
{
	if (!Mesh)
	{
		return false;
	}

	Mesh->Modify();

	const int32 ExistingLODCount = Mesh->GetNumSourceModels();
	const int32 TargetLODCount = Profile.bOverrideExisting ? Profile.NumLODs : FMath::Max(ExistingLODCount, Profile.NumLODs);
	Mesh->SetNumSourceModels(TargetLODCount);
	Mesh->bAutoComputeLODScreenSize = false;

	for (int32 LODIndex = 0; LODIndex < TargetLODCount; ++LODIndex)
	{
		FStaticMeshSourceModel& SourceModel = Mesh->GetSourceModel(LODIndex);

		if (Profile.bOverrideExisting || LODIndex >= ExistingLODCount)
		{
			SourceModel.ScreenSize.Default = GetScreenSize(Profile, LODIndex);

			if (Profile.bEnableReduction)
			{
				const float Percent = FMath::Clamp(GetReductionPercent(Profile, LODIndex), 0.0f, 100.0f);
				SourceModel.ReductionSettings.PercentTriangles = Percent / 100.0f;
			}
		}
	}

	Mesh->Build(false);
	Mesh->MarkPackageDirty();

	if (bAutoSave)
	{
		SaveMesh(Mesh);
	}

	return true;
}

bool FLODProfileApplicator::SaveMesh(UStaticMesh* Mesh)
{
	if (!Mesh)
	{
		return false;
	}

	UPackage* Package = Mesh->GetOutermost();
	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(Package);

	const bool bSuccess = FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, /*bCheckDirty=*/false, /*bPromptToSave=*/false) != EAppReturnType::Cancel;
	if (!bSuccess)
	{
		UE_LOG(LogLODProfileTool, Warning, TEXT("Failed to save package %s"), *Package->GetName());
	}
	return bSuccess;
}

void FLODProfileApplicator::OpenPreviewForAssets(const TArray<FAssetData>& Assets)
{
	if (Assets.Num() == 0)
	{
		return;
	}

	UStaticMesh* Mesh = Cast<UStaticMesh>(Assets[0].GetAsset());
	if (!Mesh)
	{
		return;
	}

	if (GEditor)
	{
		if (UAssetEditorSubsystem* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			AssetEditor->OpenEditorForAsset(Mesh);
		}
	}
}
