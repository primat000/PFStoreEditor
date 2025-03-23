#include "SStoreManagerPanel.h"
#include "StoreItem.h"
#include "SlateOptMacros.h"
#include "Engine/DataTable.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Core/PlayFabClientAPI.h" 

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


TArray<IStoreItemProvider*> FindAllStoreItemAssets()
{
    TArray<IStoreItemProvider*> StoreItemAssets;

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
        if (Asset && Asset->Implements<UStoreItemProvider>())
        {
            StoreItemAssets.Add(Cast<IStoreItemProvider>(Asset));
        }
    }

    return StoreItemAssets;
}

bool SaveStoreItemsToJson(const TArray<IStoreItemProvider*>& StoreItemProviders)
{
    TArray<TSharedPtr<FJsonValue>> JsonItems;

    for (const IStoreItemProvider* Provider : StoreItemProviders)
    {
        if (Provider)
        {
            TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

            JsonObject->SetStringField("ItemId", Provider->GetItemId());
            JsonObject->SetStringField("DisplayName", Provider->GetDisplayName());
            JsonObject->SetStringField("Description", Provider->GetDescription());
            JsonObject->SetStringField("Price", Provider->GetPrice());
            JsonObject->SetStringField("Currency", Provider->GetCurrency());

            JsonItems.Add(MakeShareable(new FJsonValueObject(JsonObject)));
        }
    }

    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
    RootObject->SetArrayField("Items", JsonItems);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

    if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
    {
        FString FilePath = FPaths::ProjectSavedDir() / TEXT("StoreItems.json");
        return FFileHelper::SaveStringToFile(OutputString, *FilePath);
    }

    return false;
}

void ExportAllStoreItems()
{
    TArray<IStoreItemProvider*> StoreItemProviders = FindAllStoreItemAssets();

    if (SaveStoreItemsToJson(StoreItemProviders))
    {
        UE_LOG(LogTemp, Log, TEXT("Store items exported successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to export store items."));
    }
}

void SStoreManagerPanel::Construct(const FArguments& InArgs)
{
    ChildSlot
        [
            SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SButton)
                        .Text(FText::FromString("Export to JSON"))
                        .OnClicked(this, &SStoreManagerPanel::OnExportJsonClicked)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SButton)
                        .Text(FText::FromString("Upload to PlayFab"))
                        .OnClicked(this, &SStoreManagerPanel::OnUploadToPlayFabClicked)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SButton)
                        .Text(FText::FromString("Find all items"))
                        .OnClicked(this, &SStoreManagerPanel::OnFindStoreItemsClicked)
                ]

                // Контейнер для списка предметов
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SAssignNew(ItemsListContainer, SVerticalBox)
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

FReply SStoreManagerPanel::OnFindStoreItemsClicked()
{
    FindStoreItemsClicked();
    return FReply::Handled();
}

void SStoreManagerPanel::ExportDataTableToJson()
{
    // Trest DataTable '/Game/Store/StoreDataTable'
    static const FString DataTablePath = TEXT("/Game/Store/StoreDataTable.StoreDataTable");
    UDataTable* StoreDataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);

    if (!StoreDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load StoreDataTable at path: %s"), *DataTablePath);
        return;
    }

    TArray<FStoreItem*> StoreItems;
    StoreDataTable->GetAllRows<FStoreItem>(TEXT(""), StoreItems);

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
    const FString FilePath = FPaths::ProjectSavedDir() / TEXT("StoreData.json");
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find StoreData.json"));
        return;
    }

     //TODO: playfab request
}

void SStoreManagerPanel::FindStoreItemsClicked()
{
    ItemsListContainer->ClearChildren();

    InnerStoreItems = FindAllStoreItemAssets();

    if (InnerStoreItems.Num() == 0)
    {
        ItemsListContainer->AddSlot()
            .AutoHeight()
            [
                SNew(STextBlock)
                    .Text(FText::FromString("No items found."))
            ];
        return;
    }

    for (const IStoreItemProvider* Provider : InnerStoreItems)
    {
        if (Provider)
        {
            ItemsListContainer->AddSlot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                        .Text(FText::Format(
                            FText::FromString("ID: {0}, Name: {1}"),
                            FText::FromString(Provider->GetItemId()),
                            FText::FromString(Provider->GetDisplayName())
                        ))
                ];
        }
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION