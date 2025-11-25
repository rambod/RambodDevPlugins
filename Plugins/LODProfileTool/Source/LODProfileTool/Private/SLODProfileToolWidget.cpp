// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLODProfileToolWidget.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "LODProfileApplicator.h"
#include "LODProfileTool.h"
#include "LODProfileToolSettings.h"
#include "Modules/ModuleManager.h"
#include "Engine/StaticMesh.h"
#include "IContentBrowserSingleton.h"
#include "Misc/MessageDialog.h"
#include "UObject/TopLevelAssetPath.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"

void SLODProfileToolWidget::Construct(const FArguments& InArgs)
{
	EditableProfile = ULODProfileToolSettings::Get()->BuildProfile();
	bAutoSaveOverride = ULODProfileToolSettings::Get()->bAutoSaveAssets;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("Num LODs")))
			]
			+ SHorizontalBox::Slot()
			.Padding(8.f, 0.f)
			.AutoWidth()
			[
				SAssignNew(LODCountTextBox, SEditableTextBox)
				.Text(FText::AsNumber(EditableProfile.NumLODs))
				.MinDesiredWidth(60.f)
				.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
				{
					int32 NewCount = FCString::Atoi(*NewText.ToString());
					NewCount = FMath::Clamp(NewCount, 1, 8);
					ResizeArraysToLODCount(NewCount);
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(ReductionCheckBox, SCheckBox)
				.IsChecked(EditableProfile.bEnableReduction ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State){ EditableProfile.bEnableReduction = State == ECheckBoxState::Checked; })
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Enable Reduction")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(12.f, 0.f)
			.VAlign(VAlign_Center)
			[
				SAssignNew(OverrideCheckBox, SCheckBox)
				.IsChecked(EditableProfile.bOverrideExisting ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State){ EditableProfile.bOverrideExisting = State == ECheckBoxState::Checked; })
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Override Existing")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(12.f, 0.f)
			.VAlign(VAlign_Center)
			[
				SAssignNew(AutoSaveCheckBox, SCheckBox)
				.IsChecked(bAutoSaveOverride ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State){ bAutoSaveOverride = State == ECheckBoxState::Checked; })
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Auto-save after apply")))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SAssignNew(LODList, SVerticalBox)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SUniformGridPanel)
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Apply to Selection")))
				.OnClicked(this, &SLODProfileToolWidget::OnApplyToSelection)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Apply to Folder")))
				.OnClicked(this, &SLODProfileToolWidget::OnApplyToFolder)
			]
			+ SUniformGridPanel::Slot(0, 1)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Preview Selected Mesh")))
				.OnClicked(this, &SLODProfileToolWidget::OnPreviewMesh)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromString(TEXT("Usage: Select static meshes or folders in the Content Browser, then Apply to Selection/Folder. Toggle Override Existing to replace current LODs. Auto-save saves immediately after apply. Preview Selected Mesh opens the first selected mesh in the Static Mesh Editor.")))
		]
	];

	RebuildLODEntries();
}

void SLODProfileToolWidget::RebuildLODEntries()
{
	if (!LODList.IsValid())
	{
		return;
	}

	LODList->ClearChildren();
	ScreenSizeTextBoxes.Reset();
	ReductionTextBoxes.Reset();

	for (int32 Index = 0; Index < EditableProfile.NumLODs; ++Index)
	{
		LODList->AddSlot()
		.AutoHeight()
		.Padding(4.f)
		[
			BuildLODRow(Index)
		];
	}
}

TSharedRef<SWidget> SLODProfileToolWidget::BuildLODRow(int32 Index)
{
	const float ScreenSize = EditableProfile.ScreenSizes.IsValidIndex(Index) ? EditableProfile.ScreenSizes[Index] : 1.0f;
	const float Reduction = EditableProfile.ReductionPercents.IsValidIndex(Index) ? EditableProfile.ReductionPercents[Index] : 100.0f;

	TSharedPtr<SEditableTextBox> ScreenSizeBox;
	TSharedPtr<SEditableTextBox> ReductionBox;

	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::Format(FText::FromString(TEXT("LOD {0}")), FText::AsNumber(Index)))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(8.f, 0.f)
		[
			SAssignNew(ScreenSizeBox, SEditableTextBox)
			.Text(FText::AsNumber(ScreenSize))
			.MinDesiredWidth(80.f)
			.OnTextCommitted_Lambda([this, Index](const FText& NewText, ETextCommit::Type)
			{
				const float Value = FCString::Atof(*NewText.ToString());
				EditableProfile.ScreenSizes[Index] = FMath::Clamp(Value, 0.0f, 1.0f);
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Screen Size")))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(12.f, 0.f)
		[
			SAssignNew(ReductionBox, SEditableTextBox)
			.Text(FText::AsNumber(Reduction))
			.MinDesiredWidth(80.f)
			.OnTextCommitted_Lambda([this, Index](const FText& NewText, ETextCommit::Type)
			{
				const float Value = FCString::Atof(*NewText.ToString());
				EditableProfile.ReductionPercents[Index] = FMath::Clamp(Value, 0.0f, 100.0f);
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("% Triangles")))
		];

	ScreenSizeTextBoxes.Add(ScreenSizeBox);
	ReductionTextBoxes.Add(ReductionBox);
	return Row;
}

void SLODProfileToolWidget::SyncProfileFromUI()
{
	int32 NewLODCount = EditableProfile.NumLODs;
	NewLODCount = FCString::Atoi(*LODCountTextBox->GetText().ToString());
	NewLODCount = FMath::Clamp(NewLODCount, 1, 8);
	ResizeArraysToLODCount(NewLODCount);
}

void SLODProfileToolWidget::ResizeArraysToLODCount(int32 NewCount)
{
	EditableProfile.NumLODs = NewCount;
	const int32 OldScreenSizeCount = EditableProfile.ScreenSizes.Num();
	const int32 OldReductionCount = EditableProfile.ReductionPercents.Num();
	EditableProfile.ScreenSizes.SetNum(NewCount);
	EditableProfile.ReductionPercents.SetNum(NewCount);

	for (int32 Index = OldScreenSizeCount; Index < NewCount; ++Index)
	{
		EditableProfile.ScreenSizes[Index] = Index == 0 ? 1.0f : FMath::Max(0.0f, 1.0f - Index * 0.25f);
	}
	for (int32 Index = OldReductionCount; Index < NewCount; ++Index)
	{
		EditableProfile.ReductionPercents[Index] = 100.0f;
	}

	if (LODCountTextBox.IsValid())
	{
		LODCountTextBox->SetText(FText::AsNumber(NewCount));
	}
	RebuildLODEntries();

}

FReply SLODProfileToolWidget::OnApplyToSelection()
{
	SyncProfileFromUI();

	FString Error;
	if (!FLODProfileApplicator::ValidateProfile(EditableProfile, Error))
	{
		UE_LOG(LogLODProfileTool, Error, TEXT("Profile invalid: %s"), *Error);
		return FReply::Handled();
	}

	FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	CBModule.Get().GetSelectedAssets(SelectedAssets);

	if (!ConfirmOverwriteIfNeeded(SelectedAssets.Num()))
	{
		return FReply::Handled();
	}

	const ULODProfileToolSettings* Settings = ULODProfileToolSettings::Get();
	if (Settings->bWarnOnOverwrite && EditableProfile.bOverrideExisting)
	{
		UE_LOG(LogLODProfileTool, Warning, TEXT("Overwrite existing LODs is enabled. Existing settings may be replaced."));
	}

	const bool bAutoSave = bAutoSaveOverride;
	const int32 Applied = FLODProfileApplicator::ApplyProfileToAssets(EditableProfile, SelectedAssets, bAutoSave);
	UE_LOG(LogLODProfileTool, Log, TEXT("Applied LOD profile to %d asset(s)."), Applied);
	return FReply::Handled();
}

FReply SLODProfileToolWidget::OnApplyToFolder()
{
	SyncProfileFromUI();

	FString Error;
	if (!FLODProfileApplicator::ValidateProfile(EditableProfile, Error))
	{
		UE_LOG(LogLODProfileTool, Error, TEXT("Profile invalid: %s"), *Error);
		return FReply::Handled();
	}

	FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FString> SelectedPaths;
	CBModule.Get().GetSelectedFolders(SelectedPaths);
	if (SelectedPaths.Num() == 0)
	{
		TArray<FString> PathViewFolders;
		CBModule.Get().GetSelectedPathViewFolders(PathViewFolders);
		SelectedPaths.Append(PathViewFolders);
	}

	if (SelectedPaths.Num() == 0)
	{
		UE_LOG(LogLODProfileTool, Warning, TEXT("No folders selected. Select a folder in the Content Browser and try again."));
		return FReply::Handled();
	}

	TArray<FAssetData> Assets;
	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TSet<FName> UniquePaths;
	for (const FString& PathString : SelectedPaths)
	{
		if (!PathString.StartsWith(TEXT("/")))
		{
			UE_LOG(LogLODProfileTool, Warning, TEXT("Skipping invalid folder path: %s"), *PathString);
			continue;
		}

		const FName PackagePath(*PathString);
		if (UniquePaths.Contains(PackagePath))
		{
			continue;
		}
		UniquePaths.Add(PackagePath);

		TArray<FAssetData> PathAssets;
		AssetRegistry.Get().GetAssetsByPath(PackagePath, PathAssets, /*bRecursive=*/true, /*bIncludeOnlyOnDiskAssets=*/false);
		for (const FAssetData& AssetData : PathAssets)
		{
			if (AssetData.AssetClassPath == UStaticMesh::StaticClass()->GetClassPathName())
			{
				Assets.Add(AssetData);
			}
		}
		UE_LOG(LogLODProfileTool, Log, TEXT("Found %d asset(s) under %s"), PathAssets.Num(), *PathString);
	}

	if (!ConfirmOverwriteIfNeeded(Assets.Num()))
	{
		return FReply::Handled();
	}

	const ULODProfileToolSettings* Settings = ULODProfileToolSettings::Get();
	if (Settings->bWarnOnOverwrite && EditableProfile.bOverrideExisting)
	{
		UE_LOG(LogLODProfileTool, Warning, TEXT("Overwrite existing LODs is enabled. Existing settings may be replaced."));
	}

	const bool bAutoSave = bAutoSaveOverride;
	const int32 Applied = FLODProfileApplicator::ApplyProfileToAssets(EditableProfile, Assets, bAutoSave);
	UE_LOG(LogLODProfileTool, Log, TEXT("Applied LOD profile to %d asset(s) from folders."), Applied);
	return FReply::Handled();
}

FReply SLODProfileToolWidget::OnPreviewMesh()
{
	FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	CBModule.Get().GetSelectedAssets(SelectedAssets);
	FLODProfileApplicator::OpenPreviewForAssets(SelectedAssets);
	return FReply::Handled();
}

bool SLODProfileToolWidget::ConfirmOverwriteIfNeeded(int32 AssetCount) const
{
	const ULODProfileToolSettings* Settings = ULODProfileToolSettings::Get();
	if (!Settings->bWarnOnOverwrite || !EditableProfile.bOverrideExisting || AssetCount <= 1)
	{
		return true;
	}

	const FText Message = FText::Format(
		FText::FromString(TEXT("Overwrite existing LODs for {0} assets? This will replace current LOD settings.")),
		FText::AsNumber(AssetCount));
	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, Message);
	return Response == EAppReturnType::Yes;
}
