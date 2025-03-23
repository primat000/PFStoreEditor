#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StoreItem.h"
#include "StoreItemProvider.generated.h"

UINTERFACE(Blueprintable)
class UStoreItemProvider : public UInterface
{
    GENERATED_BODY()
};

class IStoreItemProvider
{
    GENERATED_BODY()

public:

    virtual FString GetItemId() const = 0;
    virtual FString GetDisplayName() const = 0;
    virtual FString GetDescription() const = 0;
    virtual FString GetPrice() const = 0;
    virtual FString GetCurrency() const = 0;
};

