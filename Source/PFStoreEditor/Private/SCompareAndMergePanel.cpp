#include "SCompareAndMergePanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "ItemDiffWindow.h"

void SCompareAndMergePanel::Construct(const FArguments& InArgs)
{
    bShowDiffs = false;
    CurrentTypeTabIndex = -1;

    DiffRows.Empty();
    OnCompareRequest = InArgs._OnCompareRequest;

    ChildSlot
        [
            SNew(SBorder)
                .Padding(8)
                [
                    SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(0, 0, 0, 4)
                        [
                            SNew(SButton)
                                .Text(FText::FromString(TEXT("Show Diffs")))
                                .OnClicked(this, &SCompareAndMergePanel::OnShowDiffsClicked)
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 4)
                        [
                            BuildTypesTabs()
                        ]

                    + SVerticalBox::Slot()
                        .FillHeight(1.f)
                        [
                            SNew(SVerticalBox)

                                + SVerticalBox::Slot()
                                .FillHeight(1.f)
                                .VAlign(VAlign_Center)
                                .HAlign(HAlign_Center)
                                .Padding(0, 8, 0, 0)
                                [
                                    SNew(SBox)
                                        .Visibility(this, &SCompareAndMergePanel::GetIntroVisibility)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("Press Show Diffs to compare Editor and PlayFab Economy")))
                                        ]
                                ]

                                + SVerticalBox::Slot()
                                .FillHeight(1.f)
                                .Padding(0, 4, 0, 0)
                                [
                                    SNew(SBox)
                                        .Visibility(this, &SCompareAndMergePanel::GetDiffsVisibility)
                                        [
                                            SNew(SVerticalBox)

                                                + SVerticalBox::Slot()
                                                .AutoHeight()
                                                .Padding(0, 0, 0, 2)
                                                [
                                                    SNew(SHorizontalBox)

                                                        // "Editor"
                                                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
                                                        [
                                                            SNew(STextBlock)
                                                                .Text(FText::FromString(TEXT("ID:")))
                                                                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                                                        ]

                                                        + SHorizontalBox::Slot().FillWidth(1.f)

                                                        // "Search:"
                                                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                                        [
                                                            SNew(STextBlock)
                                                                .Text(FText::FromString(TEXT("Search:")))
                                                        ]

                                                        + SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
                                                        [
                                                            SAssignNew(SearchTextBox, SEditableTextBox)
                                                                .MinDesiredWidth(200.f)
                                                                .OnTextChanged_Lambda([this](const FText& NewText)
                                                                    {
                                                                        CurrentFilterText = NewText.ToString();
                                                                        ApplyFilter();
                                                                    })
                                                        ]
                                                ]

                                            + SVerticalBox::Slot()
                                                .FillHeight(1.f)
                                                .Padding(0, 2, 0, 4)
                                                [
                                                    SAssignNew(DiffListView, SListView<FCompareDiffRowPtr>)
                                                        .ListItemsSource(&DiffRows)
                                                        .OnGenerateRow(this, &SCompareAndMergePanel::OnGenerateDiffRow)
                                                        .OnSelectionChanged(this, &SCompareAndMergePanel::OnDiffRowSelected)
                                                        .OnMouseButtonDoubleClick(this, &SCompareAndMergePanel::OnRowDoubleClicked)
                                                        .SelectionMode(ESelectionMode::Single)
                                                ]

                                                + SVerticalBox::Slot()
                                                .AutoHeight()
                                                .Padding(0, 4, 0, 4)
                                                [
                                                    SNew(SHorizontalBox)
                                                        // ... Selected: Editor[...] PlayFab[...] + [Compare selected] ...
                                                ]

                                            // Save to Editor
                                            + SVerticalBox::Slot()
                                                .AutoHeight()
                                                .Padding(0, 4, 0, 0)
                                                .HAlign(HAlign_Center)
                                                [
                                                    SNew(SButton)
                                                        .Text(FText::FromString(TEXT("Save to Editor")))
                                                ]
                                        ]
                                ]
                        ]
                ]
        ];
}

FReply SCompareAndMergePanel::OnShowDiffsClicked()
{
    bShowDiffs = true;
    AllDiffRows.Empty();
    {
        FCompareDiffRowPtr Row1 = MakeShared<FCompareDiffRow>();
        Row1->ItemId = TEXT("Item_Sword");
        AllDiffRows.Add(Row1);

        FCompareDiffRowPtr Row2 = MakeShared<FCompareDiffRow>();
        Row2->ItemId = TEXT("Item_Axe");
        AllDiffRows.Add(Row2);

        FCompareDiffRowPtr Row3 = MakeShared<FCompareDiffRow>();
        Row3->ItemId = TEXT("Item_Shld");
        AllDiffRows.Add(Row3);
    }

    ApplyFilter();

    return FReply::Handled();
}

TSharedRef<ITableRow> SCompareAndMergePanel::OnGenerateDiffRow(
    FCompareDiffRowPtr Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<FCompareDiffRowPtr>, OwnerTable)
        [
            SNew(STextBlock)
                .Text(FText::FromString(Item->ItemId))
        ];
}

void SCompareAndMergePanel::OnDiffRowSelected(FCompareDiffRowPtr Item, ESelectInfo::Type SelectInfo)
{
    if (Item.IsValid())
    {
        SelectedEditorId = Item->ItemId;
        SelectedPlayFabId = Item->ItemId;
    }
    else
    {
        SelectedEditorId.Empty();
        SelectedPlayFabId.Empty();
    }
}

EVisibility SCompareAndMergePanel::GetIntroVisibility() const
{
    return bShowDiffs ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SCompareAndMergePanel::GetDiffsVisibility() const
{
    return bShowDiffs ? EVisibility::Visible : EVisibility::Collapsed;
}

void SCompareAndMergePanel::ApplyFilter()
{
    DiffRows.Empty();

    for (const FCompareDiffRowPtr& Row : AllDiffRows)
    {
        if (CurrentFilterText.IsEmpty()
            || Row->ItemId.Contains(CurrentFilterText, ESearchCase::IgnoreCase))
        {
            DiffRows.Add(Row);
        }
    }

    if (DiffListView.IsValid())
    {
        DiffListView->RequestListRefresh();
    }
}

TSharedRef<SWidget> SCompareAndMergePanel::BuildTypesTabs()
{
    return SNew(SHorizontalBox)

        + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("Items"), 0)
        ]
    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("Bundles"), 1)
        ]
    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("Containers"), 2)
        ]
    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
        [
            MakeTypeTabButton(TEXT("DropTables"), 3)
        ];
}

TSharedRef<SWidget> SCompareAndMergePanel::MakeTypeTabButton(const FString& Label, int32 Index)
{
    return SNew(SButton)
        .Text(FText::FromString(Label))
        .OnClicked_Lambda([this, Index]()
            {
                CurrentTypeTabIndex = Index;
                // TODO: reload DiffRows for selected type
                if (DiffListView.IsValid())
                {
                    DiffListView->RequestListRefresh();
                }
                return FReply::Handled();
            })
        .ButtonColorAndOpacity_Lambda([this, Index]()
            {
                return (CurrentTypeTabIndex == Index)
                    ? FLinearColor(0.25f, 0.25f, 0.9f, 1.f)
                    : FLinearColor(0.08f, 0.08f, 0.08f, 1.f);
            })
        .ForegroundColor_Lambda([this, Index]()
            {
                return (CurrentTypeTabIndex == Index)
                    ? FLinearColor::White
                    : FLinearColor(0.7f, 0.7f, 0.7f, 1.f);
            });
}

void SCompareAndMergePanel::OnRowDoubleClicked(FCompareDiffRowPtr Item)
{
    UE_LOG(LogTemp, Log, TEXT("Double clicked on diff row: %s"),
        Item.IsValid() ? *Item->ItemId : TEXT("<none>"));

    const FString LeftJsonStr = TEXT(R"JSON(
    {
      "ItemId": "Item1",
      "ItemClass": "Weapon",
      "CatalogVersion": "Main",
      "DisplayName": "Old Sword",
      "Description": "Old description",
      "VirtualCurrencyPrices": {},
      "RealCurrencyPrices": {},
      "Tags": ["Old", "Basic"],
      "CustomData": null,
      "Consumable": {
        "UsageCount": 5,
        "UsagePeriod": 2629800,
        "UsagePeriodGroup": ""
      },
      "Container": null,
      "Bundle": null,
      "CanBecomeCharacter": false,
      "IsStackable": false,
      "IsTradable": false,
      "ItemImageUrl": null,
      "IsLimitedEdition": true,
      "InitialLimitedEditionCount": 100,
      "ActivatedMembership": null
    }
    )JSON");

    const FString RightJsonStr = TEXT(R"JSON(
    {
      "ItemId": "Item1",
      "ItemClass": "Weapon",
      "CatalogVersion": "Main",
      "DisplayName": "New Sword",
      "Description": "New, better description",
      "VirtualCurrencyPrices": {},
      "RealCurrencyPrices": {},
      "Tags": ["New", "Epic"],
      "CustomData": "{\"Damage\": 50}",
      "Consumable": {
        "UsageCount": 10,
        "UsagePeriod": 2629800,
        "UsagePeriodGroup": ""
      },
      "Container": null,
      "Bundle": null,
      "CanBecomeCharacter": false,
      "IsStackable": true,
      "IsTradable": true,
      "ItemImageUrl": null,
      "IsLimitedEdition": true,
      "InitialLimitedEditionCount": 200,
      "ActivatedMembership": null
    }
    )JSON");

    TSharedPtr<FJsonObject> LeftObj;
    {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(LeftJsonStr);
        if (!FJsonSerializer::Deserialize(Reader, LeftObj) || !LeftObj.IsValid())
        {
            FMessageDialog::Open(EAppMsgType::Ok,
                FText::FromString(TEXT("Failed to parse Left JSON")));
            return;
        }
    }

    TSharedPtr<FJsonObject> RightObj;
    {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RightJsonStr);
        if (!FJsonSerializer::Deserialize(Reader, RightObj) || !RightObj.IsValid())
        {
            FMessageDialog::Open(EAppMsgType::Ok,
                FText::FromString(TEXT("Failed to parse Right JSON")));
            return;
        }
    }

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(FText::FromString(TEXT("Compare Catalog Items (Test)")))
        .ClientSize(FVector2D(900.f, 600.f))
        .SupportsMinimize(false)
        .SupportsMaximize(true);

    Window->SetContent(
        SNew(SItemDiffWindow)
        .LeftItem(LeftObj)
        .RightItem(RightObj)
        .OnResult([](TSharedPtr<FJsonObject> Merged)
            {
                FString OutStr;
                const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutStr);
                FJsonSerializer::Serialize(Merged.ToSharedRef(), Writer);

                UE_LOG(LogTemp, Log, TEXT("Merged JSON:\n%s"), *OutStr);
            })
    );

    FSlateApplication::Get().AddWindow(Window);
}