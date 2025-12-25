// MIT Licensed. Copyright (c) 2025 Olga Taranova

#include "SStoreManagerPanel.h"
#include "SlateOptMacros.h"
#include "Engine/DataTable.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "ItemDiffWindow.h"
#include "SCompareAndMergePanel.h"
#include "SEditorEconomyPanel.h"
#include "PFStoreEditorSettings.h"
#include "PFHelpers.h"

#include "Core/PlayFabAdminAPI.h" 
#include "PlayFab.h"
#include "PlayFabAdminAPI.h"
#include "PlayFabAdminModels.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SStoreManagerPanel::Construct(const FArguments& InArgs)
{
	auto MakeTabButton = [this](const FString& Label, int32 Index)
		{
			return SNew(SButton)
				.Text(FText::FromString(Label))
				.OnClicked_Lambda([this, Index]()
					{
						CurrentTabIndex = Index;
						return FReply::Handled();
					})

				.ButtonColorAndOpacity_Lambda([this, Index]()
					{
						return (CurrentTabIndex == Index)
							? FLinearColor(0.25f, 0.25f, 0.9f, 1.f)
							: FLinearColor(0.08f, 0.08f, 0.08f, 1.f);
					})

				.ForegroundColor_Lambda([this, Index]()
					{
						return (CurrentTabIndex == Index)
							? FLinearColor::White
							: FLinearColor(0.7f, 0.7f, 0.7f, 1.f);
					});
		};

	ChildSlot
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
							[
								MakeTabButton(TEXT("Editor Economy"), 0)
							]
						+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
							[
								MakeTabButton(TEXT("Compare & Merge"), 1)
							]
						+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
							[
								MakeTabButton(TEXT("Upload to Playfab Economy"), 2)
							]

				]

				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SWidgetSwitcher)
						.WidgetIndex_Lambda([this]() { return CurrentTabIndex; })

						+ SWidgetSwitcher::Slot()
							[
								BuildEditorEconomyPanel()   // Editor Economy
							]
						+ SWidgetSwitcher::Slot()
							[
								BuildCompareAndMergePanel()   // Compare & Merge
							]
						+ SWidgetSwitcher::Slot()
							[
								BuildSplitterPanel()   // Upload to Playfab Economy
							]
				]
		];
}

TSharedRef<SWidget> SStoreManagerPanel::BuildSplitterPanel()
{
	return SNew(SSplitter)

		// --- Upload ---
		+ SSplitter::Slot().Value(0.5f)
		[
			SNew(SBorder).Padding(8)
				[
					SNew(SGridPanel).FillColumn(1, 1.0f)

						// Choose path
						+ SGridPanel::Slot(0, 0).Padding(2).VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString("Choose path"))
						]
						+ SGridPanel::Slot(1, 0).Padding(2)
						[
							SNew(SHorizontalBox)

								// Path
								+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
								[
									SAssignNew(UploadPathTextBox, SEditableTextBox)
										.HintText(FText::FromString("Select file..."))
								]

								// Browse
								+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
								[
									SNew(SButton)
										.Text(FText::FromString("Browse..."))
										.OnClicked(this, &SStoreManagerPanel::OnUploadBrowseClicked)
								]
						]

						// Catalog name
						/*+ SGridPanel::Slot(0, 1).Padding(2).VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString("Catalog name"))
						]
						+ SGridPanel::Slot(1, 1).Padding(2)
						[
							SNew(SEditableTextBox)
								.HintText(FText::FromString("e.g. Main"))
						]*/

						// Upload button
						+ SGridPanel::Slot(1, 2).Padding(2).HAlign(HAlign_Left)
						[
							SNew(SButton)
								.Text(FText::FromString("Upload Economy"))
								.OnClicked(this, &SStoreManagerPanel::OnUploadClicked)
								.IsEnabled_Lambda([this]()
									{
										return	UploadPathTextBox.IsValid() &&
												!UploadPathTextBox->GetText().IsEmpty();
									})
						]
				]
		];
}

TSharedRef<SWidget> SStoreManagerPanel::BuildCompareAndMergePanel()
{
	return SNew(SCompareAndMergePanel);
}

TSharedRef<SWidget> SStoreManagerPanel::BuildEditorEconomyPanel()
{
	return SNew(SEditorEconomyPanel);
}

FReply SStoreManagerPanel::OnUploadClicked()
{
	const FString Path = UploadPathTextBox->GetText().ToString();
	UploadCatalogItemsToPlayFab(Path);
	return FReply::Handled();
}

bool SStoreManagerPanel::PickFileDialog(const FString& Title, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, FString& OutFile)
{
	if (FDesktopPlatformModule::Get())
	{
		void* ParentWindowHandle = const_cast<void*>(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr)
			);

		TArray<FString> OutFiles;
		bool bOpened = FDesktopPlatformModule::Get()->OpenFileDialog(
			ParentWindowHandle,
			Title,
			DefaultPath,
			DefaultFile,
			FileTypes,
			0,
			OutFiles
		);

		if (bOpened && OutFiles.Num() > 0)
		{
			OutFile = OutFiles[0];
			return true;
		}
	}
	return false;
}

FReply SStoreManagerPanel::OnUploadBrowseClicked()
{
	FString FilePath;
	if (PickFileDialog(
		TEXT("Choose CSV File"),
		FPaths::ProjectDir(),
		TEXT(""),
		TEXT("CSV files (*.csv)|*.csv"),
		FilePath))
	{
		UploadPathTextBox->SetText(FText::FromString(FilePath));
	}
	return FReply::Handled();
}

void SStoreManagerPanel::UploadCatalogItemsToPlayFab(const FString& File)
{
	PlayFabAdminPtr adminAPI = IPlayFabModuleInterface::Get().GetAdminAPI();
	TSharedPtr<PlayFab::AdminModels::FUpdateCatalogItemsRequest> Request = MakeShared<PlayFab::AdminModels::FUpdateCatalogItemsRequest>();

	TArray<PlayFab::AdminModels::FCatalogItem> OutItems;
	PFHelpers::ImportItemsFromCsv(File, OutItems);

	Request->Catalog = OutItems;
	// TODO: ? remove
	//Request->SetAsDefaultCatalog = true;
	const UPFStoreEditorSettings* Settings = GetDefault<UPFStoreEditorSettings>();
	Request->CatalogVersion = Settings->DefaultCatalogVersion;

	PlayFab::FPlayFabErrorDelegate OnError;
	OnError.BindLambda([](const PlayFab::FPlayFabCppError& Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to update catalog: %s"), *Error.ErrorMessage);
		});

	PlayFab::UPlayFabAdminAPI::FUpdateCatalogItemsDelegate OnSuccess;
	OnSuccess.BindLambda([](const PlayFab::AdminModels::FUpdateCatalogItemsResult& Result)
		{
			UE_LOG(LogTemp, Log, TEXT("Catalog successfully updated from JSON!"));
		});


	adminAPI->UpdateCatalogItems(*Request, OnSuccess, OnError);
}

void SStoreManagerPanel::UploadCatalogDropTablesToPlayFab(const FString& File)
{
	PlayFabAdminPtr adminAPI = IPlayFabModuleInterface::Get().GetAdminAPI();
	TSharedPtr<PlayFab::AdminModels::FUpdateRandomResultTablesRequest> Request = MakeShared<PlayFab::AdminModels::FUpdateRandomResultTablesRequest>();

	TArray<PlayFab::AdminModels::FRandomResultTable> OutTables;
	//MyPlayFabHelpers::ImportDropTablesFromCsv(File, OutTables);

	Request->Tables = OutTables;
	const UPFStoreEditorSettings* Settings = GetDefault<UPFStoreEditorSettings>();
	Request->CatalogVersion = Settings->DefaultCatalogVersion;

	PlayFab::FPlayFabErrorDelegate OnError;
	OnError.BindLambda([](const PlayFab::FPlayFabCppError& Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to update catalog: %s"), *Error.ErrorMessage);
		});

	PlayFab::UPlayFabAdminAPI::FUpdateRandomResultTablesDelegate OnSuccess;
	OnSuccess.BindLambda([](const PlayFab::AdminModels::FUpdateRandomResultTablesResult& Result)
		{
			UE_LOG(LogTemp, Log, TEXT("Catalog successfully updated from JSON!"));
		});


	adminAPI->UpdateRandomResultTables(*Request, OnSuccess, OnError);
}

void SStoreManagerPanel::ShowDiffWindow(TSharedPtr<FJsonObject> Left, TSharedPtr<FJsonObject> Right)
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString("Compare Catalog Items"))
		.ClientSize(FVector2D(900, 600))
		.SupportsMinimize(false)
		.SupportsMaximize(true);

	Window->SetContent(
		SNew(SItemDiffWindow)
		.LeftItem(Left)
		.RightItem(Right)
		.OnResult([](TSharedPtr<FJsonObject> Merged)
			{
			})
	);

	FSlateApplication::Get().AddWindow(Window);
}

void SStoreManagerPanel::ShowDiffWindow_Test()
{
	const FString LeftJsonStr = TEXT(R"JSON(
    {
      "ItemId": "Item1",
      "ItemClass": "Weapon",
      "CatalogVersion": "Main",
      "DisplayName": "Old Sword",
      "Description": "Old description",
      "VirtualCurrencyPrices": {},
      "RealCurrencyPrices": {},
      "Tags": ["Old", "Basic"],
      "CustomData": null,
      "Consumable": {
        "UsageCount": 5,
        "UsagePeriod": 2629800,
        "UsagePeriodGroup": ""
      },
      "Container": null,
      "Bundle": null,
      "CanBecomeCharacter": false,
      "IsStackable": false,
      "IsTradable": false,
      "ItemImageUrl": null,
      "IsLimitedEdition": true,
      "InitialLimitedEditionCount": 100,
      "ActivatedMembership": null
    }
    )JSON");

	const FString RightJsonStr = TEXT(R"JSON(
    {
      "ItemId": "Item1",
      "ItemClass": "Weapon",
      "CatalogVersion": "Main",
      "DisplayName": "New Sword",
      "Description": "New, better description",
      "VirtualCurrencyPrices": {},
      "RealCurrencyPrices": {},
      "Tags": ["New", "Epic"],
      "CustomData": "{\"Damage\": 50}",
      "Consumable": {
        "UsageCount": 10,
        "UsagePeriod": 2629800,
        "UsagePeriodGroup": ""
      },
      "Container": null,
      "Bundle": null,
      "CanBecomeCharacter": false,
      "IsStackable": true,
      "IsTradable": true,
      "ItemImageUrl": null,
      "IsLimitedEdition": true,
      "InitialLimitedEditionCount": 200,
      "ActivatedMembership": null
    }
    )JSON");

	TSharedPtr<FJsonObject> LeftObj;
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(LeftJsonStr);
		if (!FJsonSerializer::Deserialize(Reader, LeftObj) || !LeftObj.IsValid())
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to parse Left JSON")));
			return;
		}
	}

	TSharedPtr<FJsonObject> RightObj;
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RightJsonStr);
		if (!FJsonSerializer::Deserialize(Reader, RightObj) || !RightObj.IsValid())
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to parse Right JSON")));
			return;
		}
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("Compare Catalog Items (Test)")))
		.ClientSize(FVector2D(900.f, 600.f))
		.SupportsMinimize(false)
		.SupportsMaximize(true);

	Window->SetContent(
		SNew(SItemDiffWindow)
		.LeftItem(LeftObj)
		.RightItem(RightObj)
		.OnResult([](TSharedPtr<FJsonObject> Merged)
			{
				FString OutStr;
				auto Writer = TJsonWriterFactory<>::Create(&OutStr);
				FJsonSerializer::Serialize(Merged.ToSharedRef(), Writer);

				UE_LOG(LogTemp, Log, TEXT("Merged JSON:\n%s"), *OutStr);
			})
	);

	FSlateApplication::Get().AddWindow(Window);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION