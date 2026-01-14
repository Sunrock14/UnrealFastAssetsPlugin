// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "FastAssetsStyle.h"

class FFastAssetsCommands : public TCommands<FFastAssetsCommands>
{
public:

	FFastAssetsCommands()
		: TCommands<FFastAssetsCommands>(TEXT("FastAssets"), NSLOCTEXT("Contexts", "FastAssets", "FastAssets Plugin"), NAME_None, FFastAssetsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
