// MIT Licensed. Copyright (c) 2025 Olga Taranova

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PFStoreEditorSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PF Store Editor"))
class PFSTOREEDITOR_API UPFStoreEditorSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPFStoreEditorSettings();

#if WITH_EDITOR
    virtual FName GetContainerName() const override
    {
        return TEXT("Project");
    }

    virtual FName GetCategoryName() const override
    {
        return TEXT("Plugins");
    }

    virtual FName GetSectionName() const override
    {
        return TEXT("PF Store Editor");
    }
#endif

    UPROPERTY(EditAnywhere, config, Category = "PFStoreEditorSettings")
    FString DefaultCatalogVersion;
};