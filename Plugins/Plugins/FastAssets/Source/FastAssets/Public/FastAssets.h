// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class SDockTab;
class FSpawnTabArgs;
class FFastAssetsDropHandler;

class FFastAssetsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Opens the Fast Assets window */
	void OpenFastAssetsWindow();

	/** Get module instance */
	static FFastAssetsModule& Get();

	/** Check if module is loaded */
	static bool IsAvailable();

	/** Get the drop handler */
	FFastAssetsDropHandler& GetDropHandler();

private:
	/** Register menus and toolbar buttons */
	void RegisterMenus();

	/** Spawns the Fast Assets tab */
	TSharedRef<SDockTab> OnSpawnFastAssetsTab(const FSpawnTabArgs& SpawnTabArgs);

	/** This function will be bound to Command */
	void PluginButtonClicked();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<class SFastAssetsWindow> FastAssetsWindow;
	TSharedPtr<FFastAssetsDropHandler> DropHandler;

	/** Tab identifier */
	static const FName FastAssetsTabName;
};
