// MIT Licensed. Copyright (c) 2025 Olga Taranova

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EDiffChoice : uint8
{
	Left,
	Right
};

struct FFieldDiffRow
{
    FName       FieldName;
    FString     LeftValue;
    FString     RightValue;
    bool        bDifferent;
    EDiffChoice Choice = EDiffChoice::Right;   // by default
};

typedef TSharedPtr<FFieldDiffRow> FFieldDiffRowPtr;

class PFSTOREEDITOR_API DiffChoice
{
public:
	DiffChoice();
	~DiffChoice();
};
