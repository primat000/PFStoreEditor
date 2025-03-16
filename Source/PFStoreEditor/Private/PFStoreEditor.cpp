// Copyright Epic Games, Inc. All Rights Reserved.

#include "PFStoreEditor.h"
#include "PFStoreEditorStyle.h"
#include "PFStoreEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName PFStoreEditorTabName("PFStoreEditor");

#define LOCTEXT_NAMESPACE "FPFStoreEditorModule"

void FPFStoreEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FPFStoreEditorStyle::Initialize();
	FPFStoreEditorStyle::ReloadTextures();

	FPFStoreEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPFStoreEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FPFStoreEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPFStoreEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PFStoreEditorTabName, FOnSpawnTab::CreateRaw(this, &FPFStoreEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FPFStoreEditorTabTitle", "PFStoreEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FPFStoreEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FPFStoreEditorStyle::Shutdown();

	FPFStoreEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PFStoreEditorTabName);
}

TSharedRef<SDockTab> FPFStoreEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FPFStoreEditorModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("PFStoreEditor.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FPFStoreEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(PFStoreEditorTabName);
}

void FPFStoreEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FPFStoreEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FPFStoreEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPFStoreEditorModule, PFStoreEditor)