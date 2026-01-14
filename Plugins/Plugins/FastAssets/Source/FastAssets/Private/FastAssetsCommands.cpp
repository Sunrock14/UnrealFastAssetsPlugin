// Copyright Epic Games, Inc. All Rights Reserved.

#include "FastAssetsCommands.h"

#define LOCTEXT_NAMESPACE "FFastAssetsModule"

void FFastAssetsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "FastAssets", "Execute FastAssets action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
