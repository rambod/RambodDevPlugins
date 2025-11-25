// Copyright Epic Games, Inc. All Rights Reserved.

#include "LODProfileTool.h"

#include "LODProfileApplicator.h"
#include "LODProfileToolSettings.h"
#include "SLODProfileToolWidget.h"
#include "ToolMenus.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenu.h"
#include "Textures/SlateIcon.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "FLODProfileToolModule"

DEFINE_LOG_CATEGORY(LogLODProfileTool);

static const FName LODProfileTabName = TEXT("LODProfileTool");

void FLODProfileToolModule::StartupModule()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLODProfileToolModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LODProfileTabName,
		FOnSpawnTab::CreateRaw(this, &FLODProfileToolModule::SpawnLODProfileTab))
		.SetDisplayName(LOCTEXT("LODProfileToolTabTitle", "LOD Profile Tool"))
		.SetTooltipText(LOCTEXT("LODProfileToolTooltip", "Configure and apply LOD profiles."))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"))
		.SetMenuType(ETabSpawnerMenuType::Hidden); // avoid duplicate automatic menu entries
}

void FLODProfileToolModule::ShutdownModule()
{
	if (UToolMenus* Menus = UToolMenus::Get())
	{
		Menus->UnregisterOwner(this);
	}

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LODProfileTabName);
	}
}

void FLODProfileToolModule::InvokeTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LODProfileTabName);
}

TSharedRef<SDockTab> FLODProfileToolModule::SpawnLODProfileTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLODProfileToolWidget)
		];
}

void FLODProfileToolModule::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.StaticMesh");
	FToolMenuSection& Section = Menu->AddSection("LODProfileTool", LOCTEXT("LODProfileToolMenuLabel", "LOD Tools"));

	FToolMenuEntry& ApplyEntry = Section.AddMenuEntry(
		"ApplyLODProfile",
		LOCTEXT("ApplyLODProfile_Label", "Apply LOD Profile"),
		LOCTEXT("ApplyLODProfile_Tooltip", "Apply the configured LOD profile to selected static meshes."),
		FSlateIcon(),
		FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& Context)
		{
			if (const UContentBrowserAssetContextMenuContext* AssetContext = Context.FindContext<UContentBrowserAssetContextMenuContext>())
			{
				if (AssetContext->SelectedAssets.IsEmpty())
				{
					return;
				}

				const ULODProfileToolSettings* Settings = ULODProfileToolSettings::Get();
				FLODProfile Profile = Settings->BuildProfile();

				if (Settings->bWarnOnOverwrite && Profile.bOverrideExisting)
				{
					UE_LOG(LogLODProfileTool, Warning, TEXT("Overwrite existing LODs is enabled. Existing settings may be replaced."));
				}

				FString Error;
				if (!FLODProfileApplicator::ValidateProfile(Profile, Error))
				{
					UE_LOG(LogLODProfileTool, Error, TEXT("Profile invalid: %s"), *Error);
					return;
				}

				FLODProfileApplicator::ApplyProfileToAssets(Profile, AssetContext->SelectedAssets, Settings->bAutoSaveAssets);
			}
		})
	);
	ApplyEntry.InsertPosition = FToolMenuInsert("CommonAssetActions", EToolMenuInsertType::After);

	// Optional: add a command to open the tab
	Section.AddMenuEntry(
		"OpenLODProfileTool",
		LOCTEXT("OpenLODProfileTool_Label", "Open LOD Profile Tool"),
		LOCTEXT("OpenLODProfileTool_Tooltip", "Open the LOD Profile Tool window."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
		FUIAction(FExecuteAction::CreateRaw(this, &FLODProfileToolModule::InvokeTab)),
		EUserInterfaceActionType::Button,
		NAME_None
	);

	// Single Window menu entry (tab spawner menu hidden to avoid duplication)
	UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& WindowSection = WindowMenu->FindOrAddSection("WindowLayout");
	WindowSection.AddMenuEntry(
		"OpenLODProfileToolWindow",
		LOCTEXT("OpenLODProfileToolWindow", "LOD Profile Tool"),
		LOCTEXT("OpenLODProfileToolWindow_Tooltip", "Open the LOD Profile Tool tab."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
		FUIAction(FExecuteAction::CreateRaw(this, &FLODProfileToolModule::InvokeTab)),
		EUserInterfaceActionType::Button,
		NAME_None
	);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLODProfileToolModule, LODProfileTool)
