#include "SStoreManagerPanel.h"
#include "StoreItem.h"
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

#include "Core/PlayFabAdminAPI.h" 
#include "PlayFab.h"
#include "PlayFabAdminAPI.h"
#include "PlayFabAdminModels.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

namespace MyPlayFabHelpers
{
	static PlayFab::AdminModels::FCatalogItemConsumableInfo ToPlayFabConsumableInfo(const FConsumableInfo& In)
	{
		PlayFab::AdminModels::FCatalogItemConsumableInfo Out;

		if (In.UsageCount > 0)
		{
			Out.UsageCount = PlayFab::Boxed<uint32>(static_cast<uint32>(In.UsageCount));
		}

		if (In.UsagePeriod > 0)
		{
			Out.UsagePeriod = PlayFab::Boxed<uint32>(static_cast<uint32>(In.UsagePeriod));
		}

		if (!In.UsagePeriodGroup.IsEmpty())
		{
			Out.UsagePeriodGroup = In.UsagePeriodGroup;
		}

		return Out;
	}

	bool ExportToCsv(
		const TArray<TWeakObjectPtr<UObject>>& Items,
		const FString& FilePath)
	{
		TArray<FString> Lines;

		Lines.Add(TEXT("ItemId,DisplayName,ItemClass,Description,CustomData,Tags,")
			TEXT("IsLimitedEdition,IsTokenForCharacterCreation,IsTradable,IsStackable,")
			TEXT("UsageCount,UsagePeriod,UsagePeriodGroup,")
			TEXT("BundledItems,BundledResultTables,BundledVirtualCurrencies,")
			TEXT("KeyItemId,ItemContents,ResultTableContents,VirtualCurrencyContents")
		);

		for (auto& ItemPtr : Items)
		{
			UObject* Obj = ItemPtr.Get();
			if (IStoreItemProvider* Provider = Cast<IStoreItemProvider>(Obj))
			{
				if (!Provider) continue;

				auto Escape = [&](const FString& In) {
					FString Out = In.Replace(TEXT("\""), TEXT("\"\""));
					return FString::Printf(TEXT("\"%s\""), *Out);
					};

				FString ItemId = Escape(Provider->GetItemId());
				FString Name = Escape(Provider->GetDisplayName());
				FString ClassName = Escape(Provider->GetItemClass());
				FString Desc = Escape(Provider->GetDescription());
				FString Custom = Escape(Provider->GetCustomData());

				FString Tags;
				{
					TArray<FString> RawTags = Provider->GetTags();
					Tags = Escape(FString::Join(RawTags, TEXT(";")));
				}

				auto BoolStr = [](bool b) { return b ? TEXT("TRUE") : TEXT("FALSE"); };
				FString IsLimited = BoolStr(Provider->GetIsLimitedEdition());
				FString IsToken = BoolStr(Provider->GetIsTokenForCharacterCreation());
				FString IsTradable = BoolStr(Provider->GetIsTradable());
				FString IsStackable = BoolStr(Provider->GetIsStackable());

				// Consumable
				FConsumableInfo CI = Provider->GetConsumableInfo();
				FString UsageCountStr = (CI.UsageCount > 0) ? FString::FromInt(CI.UsageCount) : TEXT("");
				FString UsagePeriodStr = (CI.UsagePeriod > 0) ? FString::FromInt(CI.UsagePeriod) : TEXT("");
				FString UsageGroupStr = Escape(CI.UsagePeriodGroup);

				// Bundle
				FString BundledItemsStr;
				FString BundledResultTablesStr;
				FString BundledVirtualCurrenciesStr;

				if (Obj->GetClass()->ImplementsInterface(UStoreBundleProvider::StaticClass()))
				{
					IStoreBundleProvider* BundleProvider = Cast<IStoreBundleProvider>(Obj);
					FBundleInfo BI = BundleProvider->GetBundleInfo();

					BundledItemsStr = Escape(FString::Join(BI.BundledItems, TEXT(";")));

					BundledResultTablesStr = Escape(FString::Join(BI.BundledResultTables, TEXT(";")));

					if (BI.BundledVirtualCurrencies.Num() > 0)
					{
						TArray<FString> Pairs;
						for (auto& KV : BI.BundledVirtualCurrencies)
						{
							Pairs.Add(KV.Key + TEXT(":") + FString::FromInt(KV.Value));
						}
						BundledVirtualCurrenciesStr = Escape(FString::Join(Pairs, TEXT(";")));
					}
					else
					{
						BundledVirtualCurrenciesStr = TEXT("");
					}
				}
				else
				{
					BundledItemsStr = BundledResultTablesStr = BundledVirtualCurrenciesStr = TEXT("");
				}


				// Container
				FString KeyItemIdStr;
				FString ItemContentsStr;
				FString ResultTableContentsStr;
				FString VirtualCurrencyContentsStr;
				if (Obj->GetClass()->ImplementsInterface(UStoreContainerProvider::StaticClass()))
				{
					IStoreContainerProvider* ContainerProvider = Cast<IStoreContainerProvider>(Obj);
					FContainerInfo ContainerInfo = ContainerProvider->GetContainerInfo();

					KeyItemIdStr = Escape(ContainerInfo.KeyItemId);

					ItemContentsStr = Escape(FString::Join(ContainerInfo.ItemContents, TEXT(";")));
					ResultTableContentsStr = Escape(FString::Join(ContainerInfo.ResultTableContents, TEXT(";")));

					if (ContainerInfo.VirtualCurrencyContents.Num() > 0)
					{
						TArray<FString> Pairs;
						for (auto& KV : ContainerInfo.VirtualCurrencyContents)
						{
							Pairs.Add(KV.Key + TEXT(":") + FString::FromInt(KV.Value));
						}
						VirtualCurrencyContentsStr = Escape(FString::Join(Pairs, TEXT(";")));
					}
					else
					{
						VirtualCurrencyContentsStr = TEXT("");
					}
				}
				else
				{
					KeyItemIdStr = ItemContentsStr = ResultTableContentsStr = VirtualCurrencyContentsStr = TEXT("");
				}

				FString Line = FString::Printf(
					TEXT("%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s"),
					*ItemId, *Name, *ClassName, *Desc, *Custom, *Tags,
					*IsLimited, *IsToken, *IsTradable, *IsStackable,
					*UsageCountStr, *UsagePeriodStr, *UsageGroupStr,
					*BundledItemsStr, *BundledResultTablesStr, *BundledVirtualCurrenciesStr,
					*KeyItemIdStr, *ItemContentsStr, *ResultTableContentsStr, *VirtualCurrencyContentsStr
				);

				Lines.Add(MoveTemp(Line));
			}
		}

		FString CsvContent = FString::Join(Lines, TEXT("\r\n"));
		return FFileHelper::SaveStringToFile(CsvContent, *FilePath);
	}

	void ParseCsvLine(const FString& Line, TArray<FString>& OutFields)
	{
		OutFields.Empty();

		FString Current;
		bool bInQuotes = false;

		const int32 Len = Line.Len();
		for (int32 i = 0; i < Len; ++i)
		{
			TCHAR C = Line[i];

			if (C == '"')
			{
				if (bInQuotes && i + 1 < Len && Line[i + 1] == '"')
				{
					Current += '"';
					++i;
				}
				else
				{
					bInQuotes = !bInQuotes;
				}
			}
			else if (C == ',' && !bInQuotes)
			{
				FString Field = Current.TrimStartAndEnd();

				if (Field.StartsWith("\"") && Field.EndsWith("\"") && Field.Len() >= 2)
				{
					Field = Field.Mid(1, Field.Len() - 2);
				}

				OutFields.Add(Field);
				Current.Empty();
			}
			else
			{
				Current += C;
			}
		}

		{
			FString Field = Current.TrimStartAndEnd();
			if (Field.StartsWith("\"") && Field.EndsWith("\"") && Field.Len() >= 2)
			{
				Field = Field.Mid(1, Field.Len() - 2);
			}
			OutFields.Add(Field);
		}
	}

	FString Unescape(const FString& In)
	{
		return In;
	}

	bool ImportItemsFromCsv(
		const FString& FilePath,
		TArray<PlayFab::AdminModels::FCatalogItem>& OutItems)
	{
		OutItems.Empty();

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load CSV: %s"), *FilePath);
			return false;
		}
		if (Lines.Num() < 2)
		{
			UE_LOG(LogTemp, Error, TEXT("CSV has no data lines."));
			return false;
		}

		for (int32 i = 1; i < Lines.Num(); ++i)
		{
			const FString& Line = Lines[i].TrimStartAndEnd();
			if (Line.IsEmpty()) continue;

			TArray<FString> Fields;
			ParseCsvLine(Line, Fields);
			if (Fields.Num() < 20) continue;

			PlayFab::AdminModels::FCatalogItem Item;

			Item.ItemId = Fields[0];
			Item.DisplayName = Fields[1];
			Item.ItemClass = Fields[2];
			Item.Description = Fields[3];
			Item.CustomData = Fields[4];

			// Tags
			{
				TArray<FString> Tags;
				Fields[5].ParseIntoArray(Tags, TEXT(";"), true);
				Item.Tags = Tags;
			}

			Item.IsLimitedEdition = Fields[6].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			Item.CanBecomeCharacter = Fields[7].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			Item.IsTradable = Fields[8].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			Item.IsStackable = Fields[9].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);

			// Consumable
			{
				auto CI = MakeShared<PlayFab::AdminModels::FCatalogItemConsumableInfo>();
				const FString& SC = Fields[10];
				const FString& SP = Fields[11];
				if (!SC.IsEmpty()) CI->UsageCount = PlayFab::Boxed<uint32>(FCString::Atoi(*SC));
				if (!SP.IsEmpty()) CI->UsagePeriod = PlayFab::Boxed<uint32>(FCString::Atoi(*SP));
				CI->UsagePeriodGroup = Fields[12];
				Item.Consumable = CI;
			}

			// Bundle
			{
				auto BI = MakeShared<PlayFab::AdminModels::FCatalogItemBundleInfo>();
				TArray<FString> Arr;
				Fields[13].ParseIntoArray(Arr, TEXT(";"), true);
				BI->BundledItems = Arr;

				Arr.Empty();
				Fields[14].ParseIntoArray(Arr, TEXT(";"), true);
				BI->BundledResultTables = Arr;

				// VirtualCurrencies: пары Key:Value
				if (!Fields[15].IsEmpty())
				{
					TArray<FString> Pairs;
					Fields[15].ParseIntoArray(Pairs, TEXT(";"), true);
					for (auto& P : Pairs)
					{
						FString Key, Val;
						if (P.Split(TEXT(":"), &Key, &Val))
							BI->BundledVirtualCurrencies.Add(Key, FCString::Atoi(*Val));
					}
				}
				Item.Bundle = BI;
			}

			// Container
			{
				auto CI = MakeShared<PlayFab::AdminModels::FCatalogItemContainerInfo>();
				CI->KeyItemId = Fields[16];

				TArray<FString> Arr;
				Fields[17].ParseIntoArray(Arr, TEXT(";"), true);
				CI->ItemContents = Arr;

				Arr.Empty();
				Fields[18].ParseIntoArray(Arr, TEXT(";"), true);
				CI->ResultTableContents = Arr;

				if (!Fields[19].IsEmpty())
				{
					TArray<FString> Pairs;
					Fields[19].ParseIntoArray(Pairs, TEXT(";"), true);
					for (auto& P : Pairs)
					{
						FString Key, Val;
						if (P.Split(TEXT(":"), &Key, &Val))
							CI->VirtualCurrencyContents.Add(Key, FCString::Atoi(*Val));
					}
				}
				Item.Container = CI;
			}

			Item.CatalogVersion = TEXT("Main");
			Item.ItemImageUrl = FString();
			Item.InitialLimitedEditionCount = 0;
			Item.RealCurrencyPrices.Empty();

			OutItems.Add(Item);
		}

		return true;
	}

	bool ImportDropTablesFromCsv(
		const FString& FilePath,
		TArray<PlayFab::AdminModels::FRandomResultTable>& OutItems)
	{
		OutItems.Empty();

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load CSV: %s"), *FilePath);
			return false;
		}
		if (Lines.Num() < 2)
		{
			UE_LOG(LogTemp, Error, TEXT("CSV has no data lines."));
			return false;
		}

		for (int32 i = 1; i < Lines.Num(); ++i)
		{
			const FString& Line = Lines[i].TrimStartAndEnd();
			if (Line.IsEmpty()) continue;

			PlayFab::AdminModels::FRandomResultTable Item;
			OutItems.Add(Item);
		}

		return true;
	}
}


template<class UInterfaceClass>
TArray<TWeakObjectPtr<UObject>> FindAllStoreItemAssets()
{
	TArray<TWeakObjectPtr<UObject>> StoreItemAssets;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UObject::StaticClass()->GetClassPathName());

	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UObject* Asset = AssetData.GetAsset();
		if (Asset && Asset->Implements<UInterfaceClass>())
		{
			StoreItemAssets.Add(Cast<UObject>(Asset));
		}
	}

	return StoreItemAssets;
}

void SStoreManagerPanel::Construct(const FArguments& InArgs)
{
	CurrentTabIndex = 0;

	ChildSlot
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
						[
							SNew(SButton)
								.Text(FText::FromString("Items"))
								.OnClicked_Lambda([this]() { CurrentTabIndex = 0; return FReply::Handled(); })
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
						[
							SNew(SButton)
								.Text(FText::FromString("DropTables"))
								.OnClicked_Lambda([this]() { CurrentTabIndex = 1; return FReply::Handled(); })
						]
				]

				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SWidgetSwitcher)
						.WidgetIndex_Lambda([this]() { return CurrentTabIndex; })

						+ SWidgetSwitcher::Slot()
						[
							BuildSplitterPanel()
						]

					+ SWidgetSwitcher::Slot()
						[
							BuildSplitterPanel()
						]
				]
		];
}

TSharedRef<SWidget> SStoreManagerPanel::BuildSplitterPanel()
{
	return SNew(SSplitter)

		// --- Extract ---
		+ SSplitter::Slot().Value(0.5f)
		[
			SNew(SBorder).Padding(8)
				[
					SNew(SGridPanel).FillColumn(1, 1.0f)

						// Label
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
									SAssignNew(ExtractPathTextBox, SEditableTextBox)
										.HintText(FText::FromString("Select folder..."))
								]

								// Browse button
								+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
								[
									SNew(SButton)
										.Text(FText::FromString("Browse..."))
										.OnClicked(this, &SStoreManagerPanel::OnExtractBrowseClicked)
								]

								// Extract button
								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(SButton)
										.Text(FText::FromString("Extract"))
										.OnClicked(this, &SStoreManagerPanel::OnExtractClicked)
								]
						]
				]
		]

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
										.HintText(FText::FromString("Select folder..."))
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
						+ SGridPanel::Slot(0, 1).Padding(2).VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString("Catalog name"))
						]
						+ SGridPanel::Slot(1, 1).Padding(2)
						[
							SNew(SEditableTextBox)
								.HintText(FText::FromString("e.g. Main"))
						]

						// Upload button
						+ SGridPanel::Slot(1, 2).Padding(2).HAlign(HAlign_Left)
						[
							SNew(SButton)
								.Text(FText::FromString("Upload"))
								.OnClicked(this, &SStoreManagerPanel::OnUploadClicked)
						]
				]
		];
}

FReply SStoreManagerPanel::OnExtractBrowseClicked()
{
	FString Folder;
	if (PickFolderDialog(Folder))
	{
		ExtractPathTextBox->SetText(FText::FromString(Folder));
	}
	return FReply::Handled();
}

FReply SStoreManagerPanel::OnExtractClicked()
{
	const FString FolderPath = ExtractPathTextBox->GetText().ToString();
	if (FolderPath.IsEmpty())
	{
		return FReply::Handled();
	}
	const FString FullFilePath = FPaths::Combine(FolderPath, TEXT("StoreCatalog.csv"));
	TArray<TWeakObjectPtr<UObject>> TStoreItemProvider = FindAllStoreItemAssets<UStoreItemProvider>();
	MyPlayFabHelpers::ExportToCsv(TStoreItemProvider, *FullFilePath);
	return FReply::Handled();
}

FReply SStoreManagerPanel::OnUploadClicked()
{
	const FString Path = UploadPathTextBox->GetText().ToString();
	UploadCatalogItemsToPlayFab(Path);
	return FReply::Handled();
}

bool SStoreManagerPanel::PickFolderDialog(FString& OutFolder)
{
	if (FDesktopPlatformModule::Get())
	{
		void* ParentWindowHandle = const_cast<void*>(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr));
		return FDesktopPlatformModule::Get()->OpenDirectoryDialog(
			ParentWindowHandle,
			TEXT("Choose Folder"),
			FPaths::ProjectDir(),
			OutFolder
		);
	}
	return false;
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
	MyPlayFabHelpers::ImportItemsFromCsv(File, OutItems);

	Request->Catalog = OutItems;
	Request->SetAsDefaultCatalog = true;
	Request->CatalogVersion = "Main";

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
	MyPlayFabHelpers::ImportDropTablesFromCsv(File, OutTables);

	Request->Tables = OutTables;
	Request->CatalogVersion = "Main";

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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION