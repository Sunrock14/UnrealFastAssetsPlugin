// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FastAssets.h"
#include "FastAssetsStyle.h"
#include "FastAssetsCommands.h"
#include "SFastAssetsWindow.h"
#include "FastAssetsDropHandler.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Application/SlateApplication.h"

const FName FFastAssetsModule::FastAssetsTabName("FastAssetsTab");

#define LOCTEXT_NAMESPACE "FFastAssetsModule"

void FFastAssetsModule::StartupModule()
{
	// Initialize style
	FFastAssetsStyle::Initialize();
	FFastAssetsStyle::ReloadTextures();

	// Register commands
	FFastAssetsCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FFastAssetsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FFastAssetsModule::PluginButtonClicked),
		FCanExecuteAction());

	// Register menus
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFastAssetsModule::RegisterMenus));

	// Register the tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FastAssetsTabName,
		FOnSpawnTab::CreateRaw(this, &FFastAssetsModule::OnSpawnFastAssetsTab))
		.SetDisplayName(LOCTEXT("FastAssetsTabTitle", "Fast Assets"))
		.SetTooltipText(LOCTEXT("FastAssetsTabTooltip", "Open the Fast Assets browser"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FFastAssetsStyle::GetStyleSetName(), "FastAssets.PluginAction"));

	// Initialize drop handler
	DropHandler = MakeShareable(new FFastAssetsDropHandler());
	DropHandler->Initialize();
}

void FFastAssetsModule::ShutdownModule()
{
	// Shutdown drop handler
	if (DropHandler.IsValid())
	{
		DropHandler->Shutdown();
		DropHandler.Reset();
	}

	// Unregister tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FastAssetsTabName);

	// Unregister menus
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	// Shutdown style and commands
	FFastAssetsStyle::Shutdown();
	FFastAssetsCommands::Unregister();
}

FFastAssetsModule& FFastAssetsModule::Get()
{
	return FModuleManager::LoadModuleChecked<FFastAssetsModule>("FastAssets");
}

bool FFastAssetsModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded("FastAssets");
}

FFastAssetsDropHandler& FFastAssetsModule::GetDropHandler()
{
	check(DropHandler.IsValid());
	return *DropHandler;
}

void FFastAssetsModule::OpenFastAssetsWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FastAssetsTabName);
}

TSharedRef<SDockTab> FFastAssetsModule::OnSpawnFastAssetsTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SAssignNew(FastAssetsWindow, SFastAssetsWindow)
		];
}

void FFastAssetsModule::PluginButtonClicked()
{
	OpenFastAssetsWindow();
}

void FFastAssetsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add to Window menu
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FFastAssetsCommands::Get().PluginAction, PluginCommands);
		}
	}

	// Add to toolbar
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FFastAssetsCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFastAssetsModule, FastAssets)
