// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STileView.h"

// Forward declarations
class SEditableTextBox;
class SWidgetSwitcher;
class STextBlock;

struct FExternalAssetItem
{
	FString FilePath;
	FString FileName;
	FString Extension;
	FString AssetType;
	int64 FileSize;
	FDateTime ModifiedTime;

	FExternalAssetItem()
		: FileSize(0)
	{
	}
};

enum class EFastAssetsViewMode : uint8
{
	Grid,
	List
};

class SFastAssetsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFastAssetsWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SFastAssetsWindow();

private:
	// UI Construction
	TSharedRef<SWidget> ConstructToolbar();
	TSharedRef<SWidget> ConstructPathBar();
	TSharedRef<SWidget> ConstructSearchBar();
	TSharedRef<SWidget> ConstructContentArea();
	TSharedRef<SWidget> ConstructStatusBar();

	// Toolbar Actions
	FReply OnBrowseClicked();
	FReply OnRefreshClicked();
	FReply OnSettingsClicked();
	FReply OnGridViewClicked();
	FReply OnListViewClicked();

	// Path Management
	void OnPathTextCommitted(const FText& NewText, ETextCommit::Type CommitType);
	FText GetCurrentPathText() const;

	// Search
	void OnSearchTextChanged(const FText& NewText);

	// Directory Scanning
	void ScanDirectory(const FString& Path);
	void ScanDirectoryAsync(const FString& Path);
	FString DetermineAssetType(const FString& Extension) const;
	FString FormatFileSize(int64 SizeInBytes) const;

	// List View
	TSharedRef<ITableRow> OnGenerateAssetRow(TSharedPtr<FExternalAssetItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	// Grid View
	TSharedRef<ITableRow> OnGenerateAssetTile(TSharedPtr<FExternalAssetItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	// Selection
	void OnAssetSelectionChanged(TSharedPtr<FExternalAssetItem> Item, ESelectInfo::Type SelectInfo);

	// Filtering
	void RefreshFilteredAssets();
	bool PassesFilter(const TSharedPtr<FExternalAssetItem>& Item) const;

	// Drag and Drop
	FReply OnAssetDragDetected(TSharedPtr<FExternalAssetItem> DraggedItem);
	TArray<TSharedPtr<FExternalAssetItem>> GetSelectedAssets() const;

	// Context Menu
	TSharedPtr<SWidget> OnContextMenuOpening();
	void OnImportToProject();
	void OnImportAndPlace();
	void OnOpenInExplorer();
	void OnCopyPath();

private:
	// Current directory path
	FString CurrentPath;

	// All scanned assets
	TArray<TSharedPtr<FExternalAssetItem>> AllAssets;

	// Filtered assets (after search/filter)
	TArray<TSharedPtr<FExternalAssetItem>> FilteredAssets;

	// Current view mode
	EFastAssetsViewMode CurrentViewMode;

	// Search text
	FString SearchText;

	// UI Elements
	TSharedPtr<SEditableTextBox> PathTextBox;
	TSharedPtr<SListView<TSharedPtr<FExternalAssetItem>>> AssetListView;
	TSharedPtr<STileView<TSharedPtr<FExternalAssetItem>>> AssetTileView;
	TSharedPtr<SWidgetSwitcher> ViewSwitcher;
	TSharedPtr<STextBlock> StatusText;

	// Selected assets
	TArray<TSharedPtr<FExternalAssetItem>> SelectedAssets;
};
