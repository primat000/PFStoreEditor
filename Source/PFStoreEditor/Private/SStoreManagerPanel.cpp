#include "SStoreManagerPanel.h"
#include "StoreItem.h"
#include "SlateOptMacros.h"
#include "Engine/DataTable.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

// ���� ����������� PlayFab SDK:
#include "Core/PlayFabClientAPI.h" 
// �������� ��������, ��� ������ ���� � ����� ����� ���������� 
// � ����������� �� ��������� ������� PlayFab.

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SStoreManagerPanel::Construct(const FArguments& InArgs)
{
    ChildSlot
        [
            SNew(SVerticalBox)

                // ������ "Export JSON"
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SButton)
                        .Text(FText::FromString("Export to JSON"))
                        .OnClicked(this, &SStoreManagerPanel::OnExportJsonClicked)
                ]

                // ������ "Upload to PlayFab"
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SButton)
                        .Text(FText::FromString("Upload to PlayFab"))
                        .OnClicked(this, &SStoreManagerPanel::OnUploadToPlayFabClicked)
                ]
        ];
}

FReply SStoreManagerPanel::OnExportJsonClicked()
{
    ExportDataTableToJson();
    return FReply::Handled();
}

FReply SStoreManagerPanel::OnUploadToPlayFabClicked()
{
    UploadJsonToPlayFab();
    return FReply::Handled();
}

void SStoreManagerPanel::ExportDataTableToJson()
{
    // �����������, � ��� ���� DataTable '/Game/Store/StoreDataTable'
    static const FString DataTablePath = TEXT("/Game/Store/StoreDataTable.StoreDataTable");
    UDataTable* StoreDataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);

    if (!StoreDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load StoreDataTable at path: %s"), *DataTablePath);
        return;
    }

    TArray<FStoreItem*> StoreItems;
    StoreDataTable->GetAllRows<FStoreItem>(TEXT(""), StoreItems);

    // ������� JSON-������
    TArray<TSharedPtr<FJsonValue>> JsonItems;
    for (FStoreItem* Item : StoreItems)
    {
        TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
        JsonObj->SetStringField("ItemId", Item->ItemId);
        JsonObj->SetStringField("DisplayName", Item->DisplayName);
        JsonObj->SetStringField("Description", Item->Description);
        JsonObj->SetNumberField("Price", Item->Price);
        JsonObj->SetStringField("Currency", Item->Currency);

        JsonItems.Add(MakeShareable(new FJsonValueObject(JsonObj)));
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonItems, Writer);

    // ��������� JSON � ����
    const FString SavePath = FPaths::ProjectSavedDir() / TEXT("StoreData.json");
    if (FFileHelper::SaveStringToFile(OutputString, *SavePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Store data exported to %s"), *SavePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save StoreData.json"));
    }
}

void SStoreManagerPanel::UploadJsonToPlayFab()
{
    // ��������� JSON �� �����
    const FString FilePath = FPaths::ProjectSavedDir() / TEXT("StoreData.json");
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find StoreData.json"));
        return;
    }

     //������ ������ PlayFab API:
     //(������� �� ����, ����������� �� �� ClientAPI, AdminAPI � �.�.)

    //// ������-���, �.�. ������ �������� ������� ����� ��������:
    //PlayFab::ClientModels::FGetCatalogItemsRequest Request;
    //// ��������� ���� ��������, ��������:
    //Request.CatalogVersion = TEXT("MainStore");
    //Request.Catalog = JsonString; // ��� ������ ��� ��� ����� ��� PlayFab

    //// �������� API
    //PlayFab::UPlayFabClientAPI::SomeUpdateCatalog(Request,
    //    // OnSuccess
    //    [](const PlayFab::ClientModels::FSomeUpdateCatalogResult& Result)
    //    {
    //        UE_LOG(LogTemp, Log, TEXT("Successfully uploaded store to PlayFab!"));
    //    },
    //    // OnError
    //    [](const PlayFab::FPlayFabError& Error)
    //    {
    //        UE_LOG(LogTemp, Error, TEXT("Error uploading store to PlayFab: %s"), *Error.ErrorMessage);
    //    }
    //);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION