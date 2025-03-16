#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SStoreManagerPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SStoreManagerPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    FReply OnExportJsonClicked();
    FReply OnUploadToPlayFabClicked();

    void ExportDataTableToJson();
    void UploadJsonToPlayFab();
};

