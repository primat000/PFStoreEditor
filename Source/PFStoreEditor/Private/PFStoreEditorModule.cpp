// MIT Licensed. Copyright (c) 2025 Olga Taranova

#include "PFStoreEditorModule.h"
#include "PFStoreEditorSettings.h"


void FMyPluginModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner("MyPluginTab",
		FOnSpawnTab::CreateRaw(this, &FMyPluginModule::OnSpawnPluginTab))
		.SetDisplayName(NSLOCTEXT("MyPluginNamespace", "MyPluginTabTitle", "My Plugin"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	const UPFStoreEditorSettings* Settings = GetDefault<UPFStoreEditorSettings>();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("MyPluginTab"));
}

TSharedRef<SDockTab> FMyPluginModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock).Text(NSLOCTEXT("MyPluginNamespace", "MyPluginGreeting", "Hello from My Plugin!"))
				]
		];
}
