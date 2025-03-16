// Copyright Epic Games, Inc. All Rights Reserved.

#include "PFStoreEditorCommands.h"

#define LOCTEXT_NAMESPACE "FPFStoreEditorModule"

void FPFStoreEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "PFStoreEditor", "Bring up PFStoreEditor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
