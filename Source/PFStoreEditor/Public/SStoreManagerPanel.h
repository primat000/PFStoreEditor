#pragma once

#include "CoreMinimal.h"
#include "StoreItemProvider.h"
#include "Widgets/SCompoundWidget.h"

class SStoreManagerPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SStoreManagerPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TArray<IStoreItemProvider*> InnerStoreItems;
    TSharedPtr<SVerticalBox> ItemsListContainer;

    FReply OnExportJsonClicked();
    FReply OnUploadToPlayFabClicked();
    FReply OnFindStoreItemsClicked();

    void ExportDataTableToJson();
    void UploadJsonToPlayFab();
    void FindStoreItemsClicked();
};

