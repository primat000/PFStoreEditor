// MIT Licensed. Copyright (c) 2025 Olga Taranova

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FCompareDiffRow
{
    FString ItemId;
};
using FCompareDiffRowPtr = TSharedPtr<FCompareDiffRow>;

class SCompareAndMergePanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCompareAndMergePanel) {}
        SLATE_EVENT(FSimpleDelegate, OnCompareRequest)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    bool bShowDiffs = false;

    int32 CurrentTypeTabIndex = 0;

    TArray<FCompareDiffRowPtr> DiffRows;
    TSharedPtr<SListView<FCompareDiffRowPtr>> DiffListView;

    TArray<FCompareDiffRowPtr> AllDiffRows;
    TSharedPtr<SEditableTextBox> SearchTextBox;
    FString CurrentFilterText;

    FString SelectedEditorId;
    FString SelectedPlayFabId;

private:
    // UI helpers
    FReply OnShowDiffsClicked();
    TSharedRef<SWidget> BuildTypesTabs();
    TSharedRef<ITableRow> OnGenerateDiffRow(FCompareDiffRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
    void OnDiffRowSelected(FCompareDiffRowPtr Item, ESelectInfo::Type SelectInfo);
    FSimpleDelegate OnCompareRequest;
    void OnRowDoubleClicked(FCompareDiffRowPtr Item);

    EVisibility GetIntroVisibility() const;
    EVisibility GetDiffsVisibility() const;

    void ApplyFilter();

    TSharedRef<SWidget> MakeTypeTabButton(const FString& Label, int32 Index);
};