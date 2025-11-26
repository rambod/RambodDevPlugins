// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "LODProfileTypes.h"

class SEditableTextBox;

/**
 * Main Slate widget for the LOD Profile Tool tab.
 */
class SLODProfileToolWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLODProfileToolWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void RebuildLODEntries();
	TSharedRef<SWidget> BuildLODRow(int32 Index);

	// Actions
	FReply OnApplyToSelection();

	// Helpers
	void SyncProfileFromUI();
	void ResizeArraysToLODCount(int32 NewCount);
	bool ConfirmOverwriteIfNeeded(int32 AssetCount) const;

	FLODProfile EditableProfile;
	bool bAutoSaveOverride = false;

	TSharedPtr<SEditableTextBox> LODCountTextBox;
	TArray<TSharedPtr<SEditableTextBox>> ScreenSizeTextBoxes;
	TArray<TSharedPtr<SEditableTextBox>> ReductionTextBoxes;
	TSharedPtr<class SCheckBox> ReductionCheckBox;
	TSharedPtr<class SCheckBox> OverrideCheckBox;
	TSharedPtr<class SCheckBox> AutoSaveCheckBox;

	TSharedPtr<SVerticalBox> LODList;
};
