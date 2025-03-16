// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "PFStoreEditorStyle.h"

class FPFStoreEditorCommands : public TCommands<FPFStoreEditorCommands>
{
public:

	FPFStoreEditorCommands()
		: TCommands<FPFStoreEditorCommands>(TEXT("PFStoreEditor"), NSLOCTEXT("Contexts", "PFStoreEditor", "PFStoreEditor Plugin"), NAME_None, FPFStoreEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};