// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "SFastAssetsWindow.h"
#include "SAssetTableRow.h"
#include "FAssetDragDropOp.h"
#include "FastAssetImporter.h"
#include "SFastAssetsSettingsDialog.h"
#include "FastAssetsSettings.h"
#include "FastAssetsDropHandler.h"
#include "FastAssets.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Editor.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "DesktopPlatformModule.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Async/Async.h"

#define LOCTEXT_NAMESPACE "FastAssets"

void SFastAssetsWindow::Construct(const FArguments& InArgs)
{
	// Load settings
	UFastAssetsSettings* Settings = UFastAssetsSettings::Get();

	// Set default view mode from settings
	CurrentViewMode = (Settings->DefaultViewMode == EFastAssetsDefaultView::Grid)
		? EFastAssetsViewMode::Grid
		: EFastAssetsViewMode::List;

	// Restore last path if enabled
	if (Settings->bRememberLastPath && !Settings->LastBrowsedPath.IsEmpty())
	{
		CurrentPath = Settings->LastBrowsedPath;
	}
	else
	{
		CurrentPath = FPaths::ProjectContentDir();
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			ConstructToolbar()
		]

		// Path Bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 2.0f)
		[
			ConstructPathBar()
		]

		// Search Bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 2.0f)
		[
			ConstructSearchBar()
		]

		// Content Area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(4.0f)
		[
			ConstructContentArea()
		]

		// Status Bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 2.0f)
		[
			ConstructStatusBar()
		]
	];
}

SFastAssetsWindow::~SFastAssetsWindow()
{
}

TSharedRef<SWidget> SFastAssetsWindow::ConstructToolbar()
{
	return SNew(SHorizontalBox)

		// Browse Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("BrowseButton", "Browse"))
			.ToolTipText(LOCTEXT("BrowseTooltip", "Select a folder to scan for assets"))
			.OnClicked(this, &SFastAssetsWindow::OnBrowseClicked)
		]

		// Refresh Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshButton", "Refresh"))
			.ToolTipText(LOCTEXT("RefreshTooltip", "Rescan current folder"))
			.OnClicked(this, &SFastAssetsWindow::OnRefreshClicked)
		]

		// Settings Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SettingsButton", "Settings"))
			.ToolTipText(LOCTEXT("SettingsTooltip", "Open settings"))
			.OnClicked(this, &SFastAssetsWindow::OnSettingsClicked)
		]

		// Spacer
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNullWidget::NullWidget
		]

		// Grid View Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("GridViewButton", "Grid"))
			.ToolTipText(LOCTEXT("GridViewTooltip", "Switch to grid view"))
			.OnClicked(this, &SFastAssetsWindow::OnGridViewClicked)
		]

		// List View Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ListViewButton", "List"))
			.ToolTipText(LOCTEXT("ListViewTooltip", "Switch to list view"))
			.OnClicked(this, &SFastAssetsWindow::OnListViewClicked)
		];
}

TSharedRef<SWidget> SFastAssetsWindow::ConstructPathBar()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PathLabel", "Path:"))
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SAssignNew(PathTextBox, SEditableTextBox)
			.Text(this, &SFastAssetsWindow::GetCurrentPathText)
			.OnTextCommitted(this, &SFastAssetsWindow::OnPathTextCommitted)
			.HintText(LOCTEXT("PathHint", "Enter or browse to a folder path..."))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("BrowsePathButton", "..."))
			.ToolTipText(LOCTEXT("BrowsePathTooltip", "Browse for folder"))
			.OnClicked(this, &SFastAssetsWindow::OnBrowseClicked)
		];
}

TSharedRef<SWidget> SFastAssetsWindow::ConstructSearchBar()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("SearchHint", "Search assets..."))
			.OnTextChanged(this, &SFastAssetsWindow::OnSearchTextChanged)
		];
}

TSharedRef<SWidget> SFastAssetsWindow::ConstructContentArea()
{
	// Create List View with header
	TSharedPtr<SHeaderRow> HeaderRow = SNew(SHeaderRow)
		+ SHeaderRow::Column(TEXT("Icon"))
		.DefaultLabel(FText::GetEmpty())
		.FixedWidth(30.0f)

		+ SHeaderRow::Column(TEXT("Name"))
		.DefaultLabel(LOCTEXT("ColumnName", "Name"))
		.FillWidth(0.4f)

		+ SHeaderRow::Column(TEXT("Type"))
		.DefaultLabel(LOCTEXT("ColumnType", "Type"))
		.FillWidth(0.15f)

		+ SHeaderRow::Column(TEXT("Size"))
		.DefaultLabel(LOCTEXT("ColumnSize", "Size"))
		.FillWidth(0.15f)

		+ SHeaderRow::Column(TEXT("Extension"))
		.DefaultLabel(LOCTEXT("ColumnExt", "Ext"))
		.FillWidth(0.1f);

	// Create List View
	SAssignNew(AssetListView, SListView<TSharedPtr<FExternalAssetItem>>)
		.ListItemsSource(&FilteredAssets)
		.OnGenerateRow(this, &SFastAssetsWindow::OnGenerateAssetRow)
		.OnSelectionChanged(this, &SFastAssetsWindow::OnAssetSelectionChanged)
		.OnContextMenuOpening(this, &SFastAssetsWindow::OnContextMenuOpening)
		.SelectionMode(ESelectionMode::Multi)
		.HeaderRow(HeaderRow);

	// Create Tile View
	SAssignNew(AssetTileView, STileView<TSharedPtr<FExternalAssetItem>>)
		.ListItemsSource(&FilteredAssets)
		.OnGenerateTile(this, &SFastAssetsWindow::OnGenerateAssetTile)
		.OnSelectionChanged(this, &SFastAssetsWindow::OnAssetSelectionChanged)
		.OnContextMenuOpening(this, &SFastAssetsWindow::OnContextMenuOpening)
		.SelectionMode(ESelectionMode::Multi)
		.ItemWidth(120)
		.ItemHeight(140);

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(4.0f)
		[
			SAssignNew(ViewSwitcher, SWidgetSwitcher)
			.WidgetIndex(CurrentViewMode == EFastAssetsViewMode::Grid ? 0 : 1)

			// Grid View (Index 0)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					AssetTileView.ToSharedRef()
				]
			]

			// List View (Index 1)
			+ SWidgetSwitcher::Slot()
			[
				AssetListView.ToSharedRef()
			]
		];
}

TSharedRef<SWidget> SFastAssetsWindow::ConstructStatusBar()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(4.0f)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(LOCTEXT("StatusReady", "Ready - Select a folder to scan. Drag assets to Viewport or Content Browser to import."))
		];
}

FReply SFastAssetsWindow::OnBrowseClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		FString SelectedFolder;
		const bool bOpened = DesktopPlatform->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared()),
			LOCTEXT("SelectFolderTitle", "Select Asset Folder").ToString(),
			CurrentPath,
			SelectedFolder
		);

		if (bOpened && !SelectedFolder.IsEmpty())
		{
			CurrentPath = SelectedFolder;
			if (PathTextBox.IsValid())
			{
				PathTextBox->SetText(FText::FromString(CurrentPath));
			}

			// Save to recent paths
			UFastAssetsSettings* Settings = UFastAssetsSettings::Get();
			if (Settings)
			{
				Settings->AddRecentPath(CurrentPath);
			}

			ScanDirectory(CurrentPath);
		}
	}

	return FReply::Handled();
}

FReply SFastAssetsWindow::OnRefreshClicked()
{
	if (!CurrentPath.IsEmpty())
	{
		ScanDirectory(CurrentPath);
	}
	return FReply::Handled();
}

FReply SFastAssetsWindow::OnSettingsClicked()
{
	SFastAssetsSettingsDialog::ShowDialog();
	return FReply::Handled();
}

FReply SFastAssetsWindow::OnGridViewClicked()
{
	CurrentViewMode = EFastAssetsViewMode::Grid;
	if (ViewSwitcher.IsValid())
	{
		ViewSwitcher->SetActiveWidgetIndex(0);
	}
	return FReply::Handled();
}

FReply SFastAssetsWindow::OnListViewClicked()
{
	CurrentViewMode = EFastAssetsViewMode::List;
	if (ViewSwitcher.IsValid())
	{
		ViewSwitcher->SetActiveWidgetIndex(1);
	}
	return FReply::Handled();
}

void SFastAssetsWindow::OnPathTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		FString NewPath = NewText.ToString();
		if (FPaths::DirectoryExists(NewPath))
		{
			CurrentPath = NewPath;
			ScanDirectory(CurrentPath);
		}
	}
}

FText SFastAssetsWindow::GetCurrentPathText() const
{
	return FText::FromString(CurrentPath);
}

void SFastAssetsWindow::OnSearchTextChanged(const FText& NewText)
{
	SearchText = NewText.ToString();
	RefreshFilteredAssets();
}

void SFastAssetsWindow::ScanDirectory(const FString& Path)
{
	AllAssets.Empty();
	FilteredAssets.Empty();

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(LOCTEXT("StatusScanning", "Scanning: {0}"), FText::FromString(Path)));
	}

	// Supported extensions
	TArray<FString> SupportedExtensions = {
		// 3D Models
		TEXT("fbx"), TEXT("obj"), TEXT("gltf"), TEXT("glb"), TEXT("usd"), TEXT("usda"), TEXT("usdc"), TEXT("abc"),
		// Audio
		TEXT("wav"), TEXT("mp3"), TEXT("ogg"), TEXT("flac"), TEXT("aiff"),
		// Images/Textures
		TEXT("png"), TEXT("jpg"), TEXT("jpeg"), TEXT("tga"), TEXT("bmp"), TEXT("exr"), TEXT("hdr"), TEXT("psd"), TEXT("tiff"),
		// Unreal
		TEXT("uasset"), TEXT("umap"),
		// Data
		TEXT("json"), TEXT("csv")
	};

	// Find all files recursively
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFilesRecursive(FoundFiles, *Path, TEXT("*.*"), true, false);

	for (const FString& FilePath : FoundFiles)
	{
		FString Extension = FPaths::GetExtension(FilePath).ToLower();

		if (SupportedExtensions.Contains(Extension))
		{
			TSharedPtr<FExternalAssetItem> NewItem = MakeShared<FExternalAssetItem>();
			NewItem->FilePath = FilePath;
			NewItem->FileName = FPaths::GetBaseFilename(FilePath);
			NewItem->Extension = Extension;
			NewItem->AssetType = DetermineAssetType(Extension);
			NewItem->FileSize = IFileManager::Get().FileSize(*FilePath);
			NewItem->ModifiedTime = IFileManager::Get().GetTimeStamp(*FilePath);

			AllAssets.Add(NewItem);
		}
	}

	RefreshFilteredAssets();

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("StatusComplete", "{0} assets found | {1} selected | Drag to import"),
			FText::AsNumber(FilteredAssets.Num()),
			FText::AsNumber(SelectedAssets.Num())
		));
	}
}

void SFastAssetsWindow::ScanDirectoryAsync(const FString& Path)
{
	// TODO: Implement async scanning for large directories
	ScanDirectory(Path);
}

FString SFastAssetsWindow::DetermineAssetType(const FString& Extension) const
{
	// 3D Models
	if (Extension == TEXT("fbx") || Extension == TEXT("obj") ||
		Extension == TEXT("gltf") || Extension == TEXT("glb") ||
		Extension == TEXT("usd") || Extension == TEXT("usda") ||
		Extension == TEXT("usdc") || Extension == TEXT("abc"))
	{
		return TEXT("Mesh");
	}

	// Audio
	if (Extension == TEXT("wav") || Extension == TEXT("mp3") ||
		Extension == TEXT("ogg") || Extension == TEXT("flac") ||
		Extension == TEXT("aiff"))
	{
		return TEXT("Sound");
	}

	// Textures
	if (Extension == TEXT("png") || Extension == TEXT("jpg") ||
		Extension == TEXT("jpeg") || Extension == TEXT("tga") ||
		Extension == TEXT("bmp") || Extension == TEXT("exr") ||
		Extension == TEXT("hdr") || Extension == TEXT("psd") ||
		Extension == TEXT("tiff"))
	{
		return TEXT("Texture");
	}

	// Unreal Assets
	if (Extension == TEXT("uasset"))
	{
		return TEXT("UAsset");
	}

	if (Extension == TEXT("umap"))
	{
		return TEXT("Map");
	}

	// Data
	if (Extension == TEXT("json") || Extension == TEXT("csv"))
	{
		return TEXT("Data");
	}

	return TEXT("Other");
}

FString SFastAssetsWindow::FormatFileSize(int64 SizeInBytes) const
{
	if (SizeInBytes < 1024)
	{
		return FString::Printf(TEXT("%lld B"), SizeInBytes);
	}
	else if (SizeInBytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f KB"), SizeInBytes / 1024.0);
	}
	else if (SizeInBytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f MB"), SizeInBytes / (1024.0 * 1024.0));
	}
	else
	{
		return FString::Printf(TEXT("%.1f GB"), SizeInBytes / (1024.0 * 1024.0 * 1024.0));
	}
}

TSharedRef<ITableRow> SFastAssetsWindow::OnGenerateAssetRow(TSharedPtr<FExternalAssetItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SAssetListRow, OwnerTable)
		.AssetItem(Item)
		.OnDragDetected(FOnAssetDragDetected::CreateSP(this, &SFastAssetsWindow::OnAssetDragDetected));
}

TSharedRef<ITableRow> SFastAssetsWindow::OnGenerateAssetTile(TSharedPtr<FExternalAssetItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SAssetTile, OwnerTable)
		.AssetItem(Item)
		.OnDragDetected(FOnAssetDragDetected::CreateSP(this, &SFastAssetsWindow::OnAssetDragDetected));
}

void SFastAssetsWindow::OnAssetSelectionChanged(TSharedPtr<FExternalAssetItem> Item, ESelectInfo::Type SelectInfo)
{
	SelectedAssets = GetSelectedAssets();

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("StatusWithSelection", "{0} assets found | {1} selected | Drag to import"),
			FText::AsNumber(FilteredAssets.Num()),
			FText::AsNumber(SelectedAssets.Num())
		));
	}
}

void SFastAssetsWindow::RefreshFilteredAssets()
{
	FilteredAssets.Empty();

	for (const TSharedPtr<FExternalAssetItem>& Item : AllAssets)
	{
		if (PassesFilter(Item))
		{
			FilteredAssets.Add(Item);
		}
	}

	if (AssetListView.IsValid())
	{
		AssetListView->RequestListRefresh();
	}

	if (AssetTileView.IsValid())
	{
		AssetTileView->RequestListRefresh();
	}
}

bool SFastAssetsWindow::PassesFilter(const TSharedPtr<FExternalAssetItem>& Item) const
{
	if (SearchText.IsEmpty())
	{
		return true;
	}

	// Search in filename and asset type
	return Item->FileName.Contains(SearchText, ESearchCase::IgnoreCase) ||
		   Item->AssetType.Contains(SearchText, ESearchCase::IgnoreCase) ||
		   Item->Extension.Contains(SearchText, ESearchCase::IgnoreCase);
}

FReply SFastAssetsWindow::OnAssetDragDetected(TSharedPtr<FExternalAssetItem> DraggedItem)
{
	TArray<TSharedPtr<FExternalAssetItem>> AssetsToDrag = GetSelectedAssets();

	// If the dragged item is not in selection, just drag that single item
	if (AssetsToDrag.Num() == 0 || !AssetsToDrag.Contains(DraggedItem))
	{
		AssetsToDrag.Empty();
		AssetsToDrag.Add(DraggedItem);
	}

	if (AssetsToDrag.Num() > 0)
	{
		TSharedRef<FFastAssetDragDropOp> DragOp = FFastAssetDragDropOp::New(AssetsToDrag);
		return FReply::Handled().BeginDragDrop(DragOp);
	}

	return FReply::Unhandled();
}

TArray<TSharedPtr<FExternalAssetItem>> SFastAssetsWindow::GetSelectedAssets() const
{
	TArray<TSharedPtr<FExternalAssetItem>> Selected;

	if (CurrentViewMode == EFastAssetsViewMode::List && AssetListView.IsValid())
	{
		Selected = AssetListView->GetSelectedItems();
	}
	else if (CurrentViewMode == EFastAssetsViewMode::Grid && AssetTileView.IsValid())
	{
		Selected = AssetTileView->GetSelectedItems();
	}

	return Selected;
}

TSharedPtr<SWidget> SFastAssetsWindow::OnContextMenuOpening()
{
	TArray<TSharedPtr<FExternalAssetItem>> Selected = GetSelectedAssets();
	if (Selected.Num() == 0)
	{
		return nullptr;
	}

	FMenuBuilder MenuBuilder(true, nullptr);

	// Import Section
	MenuBuilder.BeginSection("Import", LOCTEXT("ImportSection", "Import"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ImportToProject", "Import to Project"),
			LOCTEXT("ImportToProjectTooltip", "Import selected assets to the project"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SFastAssetsWindow::OnImportToProject))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ImportAndPlace", "Import and Place in Level"),
			LOCTEXT("ImportAndPlaceTooltip", "Import and place selected assets in the current level"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SFastAssetsWindow::OnImportAndPlace))
		);
	}
	MenuBuilder.EndSection();

	// File Section
	MenuBuilder.BeginSection("File", LOCTEXT("FileSection", "File"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("OpenInExplorer", "Open in Explorer"),
			LOCTEXT("OpenInExplorerTooltip", "Open the file location in file explorer"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SFastAssetsWindow::OnOpenInExplorer))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("CopyPath", "Copy Path"),
			LOCTEXT("CopyPathTooltip", "Copy the file path to clipboard"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SFastAssetsWindow::OnCopyPath))
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SFastAssetsWindow::OnImportToProject()
{
	TArray<TSharedPtr<FExternalAssetItem>> Selected = GetSelectedAssets();
	if (Selected.Num() == 0)
	{
		return;
	}

	FFastAssetsDropHandler& DropHandler = FFastAssetsModule::Get().GetDropHandler();
	FFastAssetImporter& Importer = DropHandler.GetImporter();

	int32 SuccessCount = 0;
	for (const TSharedPtr<FExternalAssetItem>& Item : Selected)
	{
		FImportResult Result = Importer.ImportAsset(Item);
		if (Result.bSuccess)
		{
			SuccessCount++;
		}
	}

	// Update status
	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("ImportComplete", "Imported {0} of {1} assets"),
			FText::AsNumber(SuccessCount),
			FText::AsNumber(Selected.Num())
		));
	}
}

void SFastAssetsWindow::OnImportAndPlace()
{
	TArray<TSharedPtr<FExternalAssetItem>> Selected = GetSelectedAssets();
	if (Selected.Num() == 0)
	{
		return;
	}

	FFastAssetsDropHandler& DropHandler = FFastAssetsModule::Get().GetDropHandler();

	// Get center of viewport for placement
	FVector DropLocation = FVector::ZeroVector;
	if (GEditor)
	{
		// Use world origin as default placement location
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World)
		{
			DropLocation = FVector(0, 0, 100); // Slightly above ground
		}
	}

	// Create a fake drag drop event (we'll handle it directly)
	FFastAssetImporter& Importer = DropHandler.GetImporter();

	int32 SuccessCount = 0;
	for (const TSharedPtr<FExternalAssetItem>& Item : Selected)
	{
		FImportResult Result = Importer.ImportAsset(Item);
		if (Result.bSuccess)
		{
			SuccessCount++;
			// Note: Actual spawning in level would require more complex handling
			// For now, just import the assets
		}
	}

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("ImportPlaceComplete", "Imported {0} of {1} assets to project"),
			FText::AsNumber(SuccessCount),
			FText::AsNumber(Selected.Num())
		));
	}
}

void SFastAssetsWindow::OnOpenInExplorer()
{
	TArray<TSharedPtr<FExternalAssetItem>> Selected = GetSelectedAssets();
	if (Selected.Num() == 0)
	{
		return;
	}

	// Open the first selected item's folder
	FString FolderPath = FPaths::GetPath(Selected[0]->FilePath);
	FPlatformProcess::ExploreFolder(*FolderPath);
}

void SFastAssetsWindow::OnCopyPath()
{
	TArray<TSharedPtr<FExternalAssetItem>> Selected = GetSelectedAssets();
	if (Selected.Num() == 0)
	{
		return;
	}

	// Copy all selected paths
	FString AllPaths;
	for (int32 i = 0; i < Selected.Num(); i++)
	{
		if (i > 0)
		{
			AllPaths += TEXT("\n");
		}
		AllPaths += Selected[i]->FilePath;
	}

	FPlatformApplicationMisc::ClipboardCopy(*AllPaths);

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("PathCopied", "Copied {0} path(s) to clipboard"),
			FText::AsNumber(Selected.Num())
		));
	}
}

#undef LOCTEXT_NAMESPACE
