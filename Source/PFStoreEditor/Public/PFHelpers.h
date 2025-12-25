// MIT Licensed. Copyright (c) 2025 Olga Taranova

#pragma once

#include "CoreMinimal.h"
#include "StoreItemProvider.h"

#include "PlayFabAdminDataModels.h"

namespace PFHelpers
{
	PFSTOREEDITOR_API PlayFab::AdminModels::FCatalogItemConsumableInfo
		ToPlayFabConsumableInfo(const FConsumableInfo& In);

	PFSTOREEDITOR_API bool ExportToCsv(
		const TArray<TWeakObjectPtr<UObject>>& Items,
		const FString& FilePath);

	PFSTOREEDITOR_API void ParseCsvLine(
		const FString& Line,
		TArray<FString>& OutFields);

	PFSTOREEDITOR_API FString Unescape(const FString& In);

	PFSTOREEDITOR_API bool ImportItemsFromCsv(
		const FString& FilePath,
		TArray<PlayFab::AdminModels::FCatalogItem>& OutItems);

	PFSTOREEDITOR_API bool ImportDropTablesFromCsv(
		const FString& FilePath,
		TArray<PlayFab::AdminModels::FRandomResultTable>& OutItems);
}