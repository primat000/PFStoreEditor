// MIT Licensed. Copyright (c) 2025 Olga Taranova

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DiffChoice.h"

class SItemDiffWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SItemDiffWindow) {}
        SLATE_ARGUMENT(TSharedPtr<FJsonObject>, LeftItem)
        SLATE_ARGUMENT(TSharedPtr<FJsonObject>, RightItem)
        SLATE_ARGUMENT(TFunction<void(TSharedPtr<FJsonObject>)>, OnResult)
    SLATE_END_ARGS()

        void Construct(const FArguments& InArgs);

private:
    TArray<FFieldDiffRowPtr> Rows;
    TSharedPtr<SListView<FFieldDiffRowPtr>> ListView;
    TSharedPtr<SListView<FFieldDiffRowPtr>> ResultListView;

    TSharedPtr<FJsonObject> Left;
    TSharedPtr<FJsonObject> Right;
    TFunction<void(TSharedPtr<FJsonObject>)> OnResult;


    TSharedRef<ITableRow> OnGenerateRow(FFieldDiffRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<ITableRow> OnGenerateResultRow(FFieldDiffRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

    FReply OnUseAllLeftClicked();
    FReply OnUseAllRightClicked();
    FReply OnOkClicked();
};