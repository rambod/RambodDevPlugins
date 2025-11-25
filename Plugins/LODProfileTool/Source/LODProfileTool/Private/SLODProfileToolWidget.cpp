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
#include "UObject/TopLevelAssetPath.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"

void SLODProfileToolWidget::Construct(const FArguments& InArgs)
{
	EditableProfile = ULODProfileToolSettings::Get()->BuildProfile();

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
				SNew(SCheckBox)
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
				SNew(SCheckBox)
				.IsChecked(EditableProfile.bOverrideExisting ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State){ EditableProfile.bOverrideExisting = State == ECheckBoxState::Checked; })
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Override Existing")))
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
				.Text(FText::FromString(TEXT("Validate Profile")))
				.OnClicked(this, &SLODProfileToolWidget::OnValidateProfile)
			]
			+ SUniformGridPanel::Slot(1, 1)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Preview Selected Mesh")))
				.OnClicked(this, &SLODProfileToolWidget::OnPreviewMesh)
			]
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

	const ULODProfileToolSettings* Settings = ULODProfileToolSettings::Get();
	if (Settings->bWarnOnOverwrite && EditableProfile.bOverrideExisting)
	{
		UE_LOG(LogLODProfileTool, Warning, TEXT("Overwrite existing LODs is enabled. Existing settings may be replaced."));
	}

	const bool bAutoSave = Settings->bAutoSaveAssets;
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

	TArray<FAssetData> Assets;
	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	for (const FString& Path : SelectedPaths)
	{
		FARFilter Filter;
		Filter.PackagePaths.Add(*Path);
		Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
		Filter.bRecursivePaths = true;

		TArray<FAssetData> PathAssets;
		AssetRegistry.Get().GetAssets(Filter, PathAssets);
		Assets.Append(PathAssets);
	}

	const ULODProfileToolSettings* Settings = ULODProfileToolSettings::Get();
	if (Settings->bWarnOnOverwrite && EditableProfile.bOverrideExisting)
	{
		UE_LOG(LogLODProfileTool, Warning, TEXT("Overwrite existing LODs is enabled. Existing settings may be replaced."));
	}

	const bool bAutoSave = Settings->bAutoSaveAssets;
	const int32 Applied = FLODProfileApplicator::ApplyProfileToAssets(EditableProfile, Assets, bAutoSave);
	UE_LOG(LogLODProfileTool, Log, TEXT("Applied LOD profile to %d asset(s) from folders."), Applied);
	return FReply::Handled();
}

FReply SLODProfileToolWidget::OnValidateProfile()
{
	SyncProfileFromUI();

	FString Error;
	if (!FLODProfileApplicator::ValidateProfile(EditableProfile, Error))
	{
		UE_LOG(LogLODProfileTool, Error, TEXT("Profile invalid: %s"), *Error);
	}
	else
	{
		UE_LOG(LogLODProfileTool, Log, TEXT("Profile is valid (%d LODs)."), EditableProfile.NumLODs);
	}
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
