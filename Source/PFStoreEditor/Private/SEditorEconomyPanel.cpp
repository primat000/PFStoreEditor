// MIT Licensed. Copyright (c) 2025 Olga Taranova

#include "SEditorEconomyPanel.h"

#include "PFHelpers.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#include "Async/Async.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

#include "Core/PlayFabAdminAPI.h" 
#include "PlayFab.h"
#include "PlayFabAdminAPI.h"
#include "PlayFabAdminModels.h"


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

void SEditorEconomyPanel::Construct(const FArguments& InArgs)
{
	bShowLocalStore = false;
	CurrentTypeTabIndex = -1;

	ChildSlot
		[
			SNew(SBorder)
				.Padding(8)
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(0, 0, 0, 4)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Show local store")))
								.OnClicked(this, &SEditorEconomyPanel::OnShowLocalStoreClicked)
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.FillHeight(1.f)
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Center)
								[
									SNew(SBox)
										.Visibility(this, &SEditorEconomyPanel::GetIntroVisibility)
										[
											SNew(STextBlock)
												.Text(FText::FromString(
													TEXT("Press \"Show local store\" to display Editor Economy items.")))
										]
								]

								+ SVerticalBox::Slot()
								.FillHeight(1.f)
								[
									SNew(SBox)
										.Visibility(this, &SEditorEconomyPanel::GetListVisibility)
										[
											SNew(SVerticalBox)

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 4)
												[
													BuildTypesTabs()
												]

											+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 2, 0, 2)
												[
													SNew(SHorizontalBox)

														+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
														[
															SNew(STextBlock)
																.Text(FText::FromString(TEXT("Items list (Editor)")))
																.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
														]

														+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0, 0, 0)
														[
															SNew(STextBlock)
																.Text(FText::FromString(TEXT("▼")))
														]
												]

												// Header: ItemId | Name | Class
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 2)
												[
													SNew(SHorizontalBox)

														+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
														[
															SNew(SBox)
																.WidthOverride(160.f)
																[
																	SNew(STextBlock)
																		.Text(FText::FromString(TEXT("ItemId")))
																		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
																]
														]

														+ SHorizontalBox::Slot().AutoWidth().Padding(16, 0, 0, 0)
														[
															SNew(SBox)
																.WidthOverride(160.f)
																[
																	SNew(STextBlock)
																		.Text(FText::FromString(TEXT("Name")))
																		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
																]
														]

														+ SHorizontalBox::Slot().AutoWidth().Padding(16, 0, 0, 0)
														[
															SNew(SBox)
																.WidthOverride(160.f)
																[
																	SNew(STextBlock)
																		.Text(FText::FromString(TEXT("Class")))
																		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
																]
														]
												]

												+ SVerticalBox::Slot()
												.FillHeight(1.f)
												[
													SNew(SOverlay)

														+ SOverlay::Slot()
														[
															SAssignNew(ListView, SListView<FEditorStoreRowPtr>)
																.ListItemsSource(&Rows)
																.OnGenerateRow(this, &SEditorEconomyPanel::OnGenerateRow)
																.OnMouseButtonDoubleClick(this, &SEditorEconomyPanel::OnRowDoubleClicked)
																.SelectionMode(ESelectionMode::Single)
														]

														+ SOverlay::Slot()
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														[
															SNew(SBorder)
																.Visibility(this, &SEditorEconomyPanel::GetLoadingVisibility)
																.Padding(8)
																[
																	SNew(SHorizontalBox)
																		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
																		[
																			SNew(SThrobber)
																		]
																	+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0).VAlign(VAlign_Center)
																		[
																			SNew(STextBlock)
																				.Text(FText::FromString(TEXT("Loading editor items...")))
																		]
																]
														]
												]
										]
								]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 8, 0, 0)
						[
							SNew(SBorder)
								.Padding(8)
								[
									SNew(SGridPanel)
										.FillColumn(1, 1.0f)

										+ SGridPanel::Slot(0, 0).Padding(2).VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Choose path"))
										]

										+ SGridPanel::Slot(1, 0).Padding(2)
										[
											SNew(SHorizontalBox)

												+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
												[
													SAssignNew(ExtractPathTextBox, SEditableTextBox)
														.HintText(FText::FromString("Select folder..."))
												]

												+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
												[
													SNew(SButton)
														.Text(FText::FromString("Browse..."))
														.OnClicked(this, &SEditorEconomyPanel::OnExtractBrowseClicked)
												]

												+ SHorizontalBox::Slot().AutoWidth()
												[
													SNew(SButton)
														.Text(FText::FromString("Extract Editor Economy"))
														.OnClicked(this, &SEditorEconomyPanel::OnExtractClicked)
												]
										]
								]
						]
				]
		];
}

// ---------- Visibility ----------

EVisibility SEditorEconomyPanel::GetIntroVisibility() const
{
    return bShowLocalStore ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SEditorEconomyPanel::GetListVisibility() const
{
    return bShowLocalStore ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SEditorEconomyPanel::HandleLoadStoreTick(float DeltaTime)
{
	const int32 NumPerTick = 50;
	int32 Processed = 0;

	while (PendingAssetIndex < PendingAssets.Num() && Processed < NumPerTick)
	{
		const FAssetData& AssetData = PendingAssets[PendingAssetIndex++];
		++Processed;

		UObject* Asset = AssetData.GetAsset();
		if (!Asset)
		{
			continue;
		}

		IStoreItemProvider* Provider = Cast<IStoreItemProvider>(Asset);
		if (!Provider)
		{
			continue;
		}

		FEditorStoreRowPtr Row = MakeShared<FEditorStoreRow>();
		Row->ItemId = Provider->GetItemId();
		Row->Name = Provider->GetDisplayName();
		Row->ClassName = Provider->GetItemClass();
		Row->Asset = Asset;

		Rows.Add(Row);
	}

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}

	if (PendingAssetIndex < PendingAssets.Num())
	{
		return true;
	}

	bIsLoadingStore = false;
	return false;
}

EVisibility SEditorEconomyPanel::GetLoadingVisibility() const
{
	return bIsLoadingStore ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SEditorEconomyPanel::OnShowLocalStoreClicked()
{
	if (bIsLoadingStore)
	{
		return FReply::Handled();
	}

	bShowLocalStore = true;
	bIsLoadingStore = true;

	Rows.Empty();
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}

	PendingAssets.Empty();
	PendingAssetIndex = 0;

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UObject::StaticClass()->GetClassPathName());

	AssetRegistryModule.Get().GetAssets(Filter, PendingAssets);

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateSP(this, &SEditorEconomyPanel::HandleLoadStoreTick),
		0.0f
	);

	return FReply::Handled();
}


TSharedRef<SWidget> SEditorEconomyPanel::BuildTypesTabs()
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("Items"), 0)
        ]
    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("Bundles"), 1)
        ]
    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("Containers"), 2)
        ]
    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("DropTables"), 3)
        ];
}

TSharedRef<SWidget> SEditorEconomyPanel::MakeTypeTabButton(const FString& Label, int32 Index)
{
    return SNew(SButton)
        .Text(FText::FromString(Label))
        .OnClicked_Lambda([this, Index]()
            {
                CurrentTypeTabIndex = Index;
                LoadTestDataForCurrentType();
                return FReply::Handled();
            })
        .ButtonColorAndOpacity_Lambda([this, Index]()
            {
                return (CurrentTypeTabIndex == Index)
                    ? FLinearColor(0.25f, 0.25f, 0.9f, 1.f)
                    : FLinearColor(0.08f, 0.08f, 0.08f, 1.f);
            })
        .ForegroundColor_Lambda([this, Index]()
            {
                return (CurrentTypeTabIndex == Index)
                    ? FLinearColor::White
                    : FLinearColor(0.7f, 0.7f, 0.7f, 1.f);
            });
}

TSharedRef<ITableRow> SEditorEconomyPanel::OnGenerateRow(
    FEditorStoreRowPtr Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FEditorStoreRowPtr>, OwnerTable)
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
				[
					SNew(SBox)
						.WidthOverride(160.f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(Item->ItemId))
						]
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(16, 0, 0, 0)
				[
					SNew(SBox)
						.WidthOverride(160.f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(Item->Name))
						]
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(16, 0, 0, 0)
				[
					SNew(SBox)
						.WidthOverride(160.f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(Item->ClassName))
						]
				]
		];
}

bool SEditorEconomyPanel::PickFolderDialog(FString& OutFolder)
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

FReply SEditorEconomyPanel::OnExtractBrowseClicked()
{
    FString Folder;
    if (PickFolderDialog(Folder))
    {
        ExtractPathTextBox->SetText(FText::FromString(Folder));
    }
    return FReply::Handled();
}

FReply SEditorEconomyPanel::OnExtractClicked()
{
    const FString FolderPath = ExtractPathTextBox->GetText().ToString();
    if (FolderPath.IsEmpty())
    {
        return FReply::Handled();
    }
    const FString FullFilePath = FPaths::Combine(FolderPath, TEXT("StoreCatalog.csv"));
    TArray<TWeakObjectPtr<UObject>> TStoreItemProvider = FindAllStoreItemAssets<UStoreItemProvider>();
    PFHelpers::ExportToCsv(TStoreItemProvider, *FullFilePath);
    //ShowDiffWindow_Test();
    return FReply::Handled();
}

void SEditorEconomyPanel::OnRowDoubleClicked(FEditorStoreRowPtr Item)
{
	if (!Item.IsValid())
	{
		return;
	}

	UObject* AssetObj = nullptr;

	if (Item->Asset.IsValid())
	{
		AssetObj = Item->Asset.Get();
	}
	else if (Item->Asset.ToSoftObjectPath().IsValid())
	{
		AssetObj = Item->Asset.LoadSynchronous();
	}

	if (!AssetObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("SEditorEconomyPanel: Asset for item %s is null"),
			*Item->ItemId);
		return;
	}

	if (GEditor)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem =
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			AssetEditorSubsystem->OpenEditorForAsset(AssetObj);
		}
	}
}

void SEditorEconomyPanel::LoadTestDataForCurrentType()
{
    Rows.Empty();

    if (CurrentTypeTabIndex == 0) // Items
    {
        {
            FEditorStoreRowPtr Row = MakeShared<FEditorStoreRow>();
            Row->ItemId = TEXT("Item_Sword");
            Row->Name = TEXT("Sword");
            Row->ClassName = TEXT("Weapon");
            Rows.Add(Row);
        }
        {
            FEditorStoreRowPtr Row = MakeShared<FEditorStoreRow>();
            Row->ItemId = TEXT("Item_Axe");
            Row->Name = TEXT("Axe");
            Row->ClassName = TEXT("Weapon");
            Rows.Add(Row);
        }
    }

    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
}