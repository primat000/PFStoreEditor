#pragma once

#include "CoreMinimal.h"
#include "StoreItemProvider.h"
#include "StoreDropTableProvider.h"
#include "Widgets/SCompoundWidget.h"

class SStoreManagerPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStoreManagerPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	int32 CurrentTabIndex;

	TArray<TSharedPtr<UObject>> StoreItemProvider;
	TArray<TSharedPtr<UObject>> StoreDropTableProvider;

	TSharedPtr<SEditableTextBox> ExtractPathTextBox;
	TSharedPtr<SEditableTextBox> UploadPathTextBox;
	TSharedPtr<SEditableTextBox> CatalogNameTextBox;

	TSharedRef<SWidget> BuildSplitterPanel();

	FReply OnExtractBrowseClicked();
	FReply OnExtractClicked();

	FReply OnUploadBrowseClicked();
	FReply OnUploadClicked();

	bool PickFolderDialog(FString& OutFolder);
	bool PickFileDialog(const FString& Title, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, FString& OutFile);

	void UploadCatalogItemsToPlayFab(const FString& File);
	void UploadCatalogDropTablesToPlayFab(const FString& File);
};

