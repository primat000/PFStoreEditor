#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

#include "StoreItemProvider.h"

struct FEditorStoreRow
{
    FString ItemId;
    FString Name;
    FString ClassName;

    TSoftObjectPtr<UObject> Asset;
};
using FEditorStoreRowPtr = TSharedPtr<FEditorStoreRow>;

class SEditorEconomyPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SEditorEconomyPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TArray<FAssetData> PendingAssets;
    int32 PendingAssetIndex = 0;

    bool bShowLocalStore = false;
    int32 CurrentTypeTabIndex = -1;

    TArray<FEditorStoreRowPtr> Rows;
    TSharedPtr<SListView<FEditorStoreRowPtr>> ListView;
    bool bIsLoadingStore = false;

    TSharedPtr<SEditableTextBox> ExtractPathTextBox;

private:
    // UI

    EVisibility GetLoadingVisibility() const;
    FReply OnShowLocalStoreClicked();
    TSharedRef<SWidget> BuildTypesTabs();
    TSharedRef<SWidget> MakeTypeTabButton(const FString& Label, int32 Index);

    bool PickFolderDialog(FString& OutFolder);

    FReply OnExtractBrowseClicked();
    FReply OnExtractClicked();
    void OnRowDoubleClicked(FEditorStoreRowPtr Item);

    TSharedRef<ITableRow> OnGenerateRow(FEditorStoreRowPtr Item,
        const TSharedRef<STableViewBase>& OwnerTable);

    EVisibility GetIntroVisibility() const;
    EVisibility GetListVisibility() const;
    bool HandleLoadStoreTick(float DeltaTime);

    void LoadTestDataForCurrentType();
};