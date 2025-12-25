// MIT Licensed. Copyright (c) 2025 Olga Taranova

#include "ItemDiffWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"

void SItemDiffWindow::Construct(const FArguments& InArgs)
{
    Left = InArgs._LeftItem;
    Right = InArgs._RightItem;
    OnResult = InArgs._OnResult;

    TSet<FString> AllKeys;
    if (Left.IsValid())
    {
        for (const auto& Kvp : Left->Values)
        {
            AllKeys.Add(Kvp.Key);
        }
    }
    if (Right.IsValid())
    {
        for (const auto& Kvp : Right->Values)
        {
            AllKeys.Add(Kvp.Key);
        }
    }

    for (const FString& Key : AllKeys)
    {
        FString LeftStr, RightStr;

        if (Left.IsValid())
        {
            const TSharedPtr<FJsonValue>* V = Left->Values.Find(Key);
            if (V && V->IsValid())
            {
                (*V)->TryGetString(LeftStr);
                if (LeftStr.IsEmpty())
                {
                    LeftStr = (*V)->AsString();
                }
            }
        }

        if (Right.IsValid())
        {
            const TSharedPtr<FJsonValue>* V = Right->Values.Find(Key);
            if (V && V->IsValid())
            {
                (*V)->TryGetString(RightStr);
                if (RightStr.IsEmpty())
                {
                    RightStr = (*V)->AsString();
                }
            }
        }

        const bool bDiff = !LeftStr.Equals(RightStr, ESearchCase::CaseSensitive);
        if (bDiff)
        {
            FFieldDiffRowPtr Row = MakeShared<FFieldDiffRow>();
            Row->FieldName = FName(*Key);
            Row->LeftValue = LeftStr;
            Row->RightValue = RightStr;
            Row->bDifferent = true;
            Row->Choice = EDiffChoice::Right;

            Rows.Add(Row);
        }
    }

    ChildSlot
        [
            SNew(SBorder)
                .Padding(8)
                [
                    SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 2)
                        [
                            SNew(SHorizontalBox)

                                // Field
                                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(160.f)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("Field")))
                                        ]
                                ]

                                // Left
                                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(260.f)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("Left")))
                                        ]
                                ]

                                // L
                                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(30.f)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("L")))
                                        ]
                                ]

                                // Right
                                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(260.f)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("Right")))
                                        ]
                                ]

                                // R
                                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(30.f)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("R")))
                                        ]
                                ]
                        ]

                        + SVerticalBox::Slot()
                        .FillHeight(0.5f)
                        [
                            SAssignNew(ListView, SListView<FFieldDiffRowPtr>)
                                .ListItemsSource(&Rows)
                                .OnGenerateRow(this, &SItemDiffWindow::OnGenerateRow)
                                .SelectionMode(ESelectionMode::None)
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 4, 0, 4)
                        .HAlign(HAlign_Center)
                        [
                            SNew(SHorizontalBox)

                                + SHorizontalBox::Slot().AutoWidth().Padding(2)
                                [
                                    SNew(SButton)
                                        .Text(FText::FromString("Use all Left"))
                                        .OnClicked(this, &SItemDiffWindow::OnUseAllLeftClicked)
                                ]

                                + SHorizontalBox::Slot().AutoWidth().Padding(2)
                                [
                                    SNew(SButton)
                                        .Text(FText::FromString("Use all Right"))
                                        .OnClicked(this, &SItemDiffWindow::OnUseAllRightClicked)
                                ]
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 2)
                        [
                            SNew(SHorizontalBox)

                                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(160.f)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("Field")))
                                        ]
                                ]

                                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(260.f)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(TEXT("Result")))
                                        ]
                                ]
                        ]

                        + SVerticalBox::Slot()
                        .FillHeight(0.4f)
                        [
                            SAssignNew(ResultListView, SListView<FFieldDiffRowPtr>)
                                .ListItemsSource(&Rows)
                                .OnGenerateRow(this, &SItemDiffWindow::OnGenerateResultRow)
                                .SelectionMode(ESelectionMode::None)
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 4, 0, 0)
                        .HAlign(HAlign_Center)
                        [
                            SNew(SButton)
                                .Text(FText::FromString("OK"))
                                .OnClicked(this, &SItemDiffWindow::OnOkClicked)
                        ]
                ]
        ];
}

TSharedRef<ITableRow> SItemDiffWindow::OnGenerateRow(
    FFieldDiffRowPtr Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<FFieldDiffRowPtr>, OwnerTable)
        [
            SNew(SHorizontalBox)

                // Field
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                [
                    SNew(SBox)
                        .WidthOverride(160.f)
                        [
                            SNew(STextBlock)
                                .Text(FText::FromName(Item->FieldName))
                        ]
                ]

                // Left value
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                [
                    SNew(SBox)
                        .WidthOverride(260.f)
                        [
                            SNew(STextBlock)
                                .Text(FText::FromString(Item->LeftValue))
                                .ColorAndOpacity_Lambda([Item]()
                                    {
                                        if (Item->bDifferent && Item->Choice == EDiffChoice::Right)
                                            return FSlateColor(FLinearColor(0.25f, 0.25f, 0.25f));
                                        return FSlateColor(FLinearColor::White);
                                    })
                        ]
                ]

                // Radio Left
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                [
                    SNew(SBox)
                        .WidthOverride(30.f)
                        [
                            SNew(SCheckBox)
                                .Style(FCoreStyle::Get(), "RadioButton")
                                .IsChecked_Lambda([Item]()
                                    {
                                        return Item->Choice == EDiffChoice::Left
                                            ? ECheckBoxState::Checked
                                            : ECheckBoxState::Unchecked;
                                    })
                                .OnCheckStateChanged_Lambda([Item, this](ECheckBoxState NewState)
                                    {
                                        if (NewState == ECheckBoxState::Checked)
                                        {
                                            Item->Choice = EDiffChoice::Left;
                                            if (ResultListView.IsValid())
                                            {
                                                ResultListView->RequestListRefresh();
                                            }
                                        }
                                    })
                        ]
                ]

                // Right value
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                [
                    SNew(SBox)
                        .WidthOverride(260.f)
                        [
                            SNew(STextBlock)
                                .Text(FText::FromString(Item->RightValue))
                                .ColorAndOpacity_Lambda([Item]()
                                    {
                                        if (Item->bDifferent && Item->Choice == EDiffChoice::Left)
                                            return FSlateColor(FLinearColor(0.25f, 0.25f, 0.25f));
                                        return FSlateColor(FLinearColor::White);
                                    })
                        ]
                ]

                // Radio Right
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                [
                    SNew(SBox)
                        .WidthOverride(30.f)
                        [
                            SNew(SCheckBox)
                                .Style(FCoreStyle::Get(), "RadioButton")
                                .IsChecked_Lambda([Item]()
                                    {
                                        return Item->Choice == EDiffChoice::Right
                                            ? ECheckBoxState::Checked
                                            : ECheckBoxState::Unchecked;
                                    })
                                .OnCheckStateChanged_Lambda([Item, this](ECheckBoxState NewState)
                                    {
                                        if (NewState == ECheckBoxState::Checked)
                                        {
                                            Item->Choice = EDiffChoice::Right;
                                            if (ResultListView.IsValid())
                                            {
                                                ResultListView->RequestListRefresh();
                                            }
                                        }
                                    })
                        ]
                ]
        ];
}




FReply SItemDiffWindow::OnUseAllLeftClicked()
{
    for (auto& Row : Rows)
    {
        Row->Choice = EDiffChoice::Left;
    }
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
    if (ResultListView.IsValid())
    {
        ResultListView->RequestListRefresh();
    }
    return FReply::Handled();
}

FReply SItemDiffWindow::OnUseAllRightClicked()
{
    for (auto& Row : Rows)
    {
        Row->Choice = EDiffChoice::Right;
    }
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
    if (ResultListView.IsValid())
    {
        ResultListView->RequestListRefresh();
    }
    return FReply::Handled();
}

FReply SItemDiffWindow::OnOkClicked()
{
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    for (const FFieldDiffRowPtr& Row : Rows)
    {
        const FString Key = Row->FieldName.ToString();
        const FString& ValueStr =
            (Row->Choice == EDiffChoice::Left) ? Row->LeftValue : Row->RightValue;

        Result->SetStringField(Key, ValueStr);
    }

    if (OnResult)
    {
        OnResult(Result);
    }

    TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
    if (Window.IsValid())
    {
        Window->RequestDestroyWindow();
    }

    return FReply::Handled();
}

TSharedRef<ITableRow> SItemDiffWindow::OnGenerateResultRow(
    FFieldDiffRowPtr Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<FFieldDiffRowPtr>, OwnerTable)
        [
            SNew(SHorizontalBox)

                // Field
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                [
                    SNew(SBox)
                        .WidthOverride(160.f)
                        [
                            SNew(STextBlock)
                                .Text(FText::FromName(Item->FieldName))
                                .ColorAndOpacity_Lambda([Item]()
                                    {
                                        return (Item->Choice == EDiffChoice::Left)
                                            ? FSlateColor(FLinearColor(0.7f, 0.9f, 1.f))
                                            : FSlateColor(FLinearColor::White);
                                    })
                        ]
                ]

                // Result value
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                [
                    SNew(SBox)
                        .WidthOverride(260.f)
                        [
                            SNew(STextBlock)
                                .Text_Lambda([Item]()
                                    {
                                        const FString& ResultValue =
                                            (Item->Choice == EDiffChoice::Left)
                                            ? Item->LeftValue
                                            : Item->RightValue;

                                        return FText::FromString(ResultValue);
                                    })
                                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
                        ]
                ]
        ];
}
