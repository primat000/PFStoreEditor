#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StoreDropTableProvider.generated.h"

USTRUCT(BlueprintType)
struct FDropTableNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropTable")
    FString ResultItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropTable")
    FString ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropTable")
    int32 Weight = 0;
};

USTRUCT(BlueprintType)
struct FDropTableInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropTable")
    FString TableId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropTable")
    TArray<FDropTableNode> Nodes;
};

UINTERFACE(Blueprintable, meta = (CannotImplementInterfaceInBlueprint))
class UStoreDropTableProvider : public UInterface
{
    GENERATED_BODY()
};

class IStoreDropTableProvider
{
    GENERATED_BODY()

public:

    virtual FDropTableInfo GetDropTable() const = 0;
};