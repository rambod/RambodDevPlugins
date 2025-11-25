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
	FReply OnApplyToFolder();
	FReply OnValidateProfile();
	FReply OnPreviewMesh();

	// Helpers
	void SyncProfileFromUI();
	void ResizeArraysToLODCount(int32 NewCount);

	FLODProfile EditableProfile;

	TSharedPtr<SEditableTextBox> LODCountTextBox;
	TArray<TSharedPtr<SEditableTextBox>> ScreenSizeTextBoxes;
	TArray<TSharedPtr<SEditableTextBox>> ReductionTextBoxes;

	TSharedPtr<SVerticalBox> LODList;
};
