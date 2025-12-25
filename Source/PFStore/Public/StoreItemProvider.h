// MIT Licensed. Copyright (c) 2025 Olga Taranova

#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StoreItemProvider.generated.h"

USTRUCT(BlueprintType)
struct PFSTORE_API FConsumableInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	int32 UsageCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	int32 UsagePeriod = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	FString UsagePeriodGroup;
};

USTRUCT(BlueprintType)
struct PFSTORE_API FBundleInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BundleInfo")
	TArray<FString> BundledItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BundleInfo")
	TArray<FString> BundledResultTables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BundleInfo")
	TMap<FString, int32> BundledVirtualCurrencies;
};

USTRUCT(BlueprintType)
struct PFSTORE_API FContainerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerInfo")
	FString KeyItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerInfo")
	TArray<FString> ItemContents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerInfo")
	TArray<FString> ResultTableContents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerInfo")
	TMap<FString, int32> VirtualCurrencyContents;
};

UINTERFACE(Blueprintable)
class PFSTORE_API UStoreItemProvider : public UInterface
{
	GENERATED_BODY()
};

class PFSTORE_API IStoreItemProvider
{
	GENERATED_BODY()

public:

	virtual FString GetItemId() const = 0;
	virtual FString GetDisplayName() const = 0;
	virtual FString GetItemClass() const = 0;
	virtual FString GetDescription() const = 0;
	virtual TMap<FString, int32> GetPrices() const = 0;
	virtual FString GetCustomData() const = 0;
	virtual TArray<FString> GetTags() const = 0;
	virtual bool GetIsLimitedEdition() const = 0;
	virtual bool GetIsTokenForCharacterCreation() const = 0;
	virtual bool GetIsTradable() const = 0;
	virtual bool GetIsStackable() const = 0;
	virtual FConsumableInfo GetConsumableInfo() const
	{
		return FConsumableInfo{};
	}
};

UINTERFACE(Blueprintable, meta = (CannotImplementInterfaceInBlueprint))
class PFSTORE_API UStoreContainerProvider : public UInterface
{
	GENERATED_BODY()
};

class PFSTORE_API IStoreContainerProvider
{
	GENERATED_BODY()

public:

	virtual FContainerInfo GetContainerInfo() const = 0;
};

UINTERFACE(Blueprintable, meta = (CannotImplementInterfaceInBlueprint))
class PFSTORE_API UStoreBundleProvider : public UInterface
{
	GENERATED_BODY()
};

class PFSTORE_API IStoreBundleProvider
{
	GENERATED_BODY()

public:

	virtual FBundleInfo GetBundleInfo() const = 0;
};
