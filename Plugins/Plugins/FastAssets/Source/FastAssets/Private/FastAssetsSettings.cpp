// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FastAssetsSettings.h"

#define LOCTEXT_NAMESPACE "FastAssets"

UFastAssetsSettings::UFastAssetsSettings()
{
	// General defaults
	DefaultViewMode = EFastAssetsDefaultView::Grid;
	ThumbnailSize = EFastAssetsThumbnailSize::Medium;
	bDisableThumbnails = false;
	bRememberLastPath = true;
	LastBrowsedPath = TEXT("");
	RecentPathsCount = 10;

	// Import defaults
	bAutoImportOnDrop = true;
	bShowImportDialog = false;
	DuplicateHandling = EFastAssetsDuplicateHandling::Ask;

	// Organization defaults
	bAutoCreateFolders = true;
	BaseImportPath = TEXT("/Game/Assets");

	// Default folder mappings
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Mesh"), TEXT("Meshes")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Sound"), TEXT("Sounds")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Texture"), TEXT("Textures")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Material"), TEXT("Materials")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Animation"), TEXT("Animations")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Blueprint"), TEXT("Blueprints")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Data"), TEXT("Data")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Other"), TEXT("Misc")));

	// Scanning defaults
	bRecursiveScan = true;
	MaxScanDepth = 0; // Unlimited
}

UFastAssetsSettings* UFastAssetsSettings::Get()
{
	return GetMutableDefault<UFastAssetsSettings>();
}

int32 UFastAssetsSettings::GetThumbnailSizePixels() const
{
	switch (ThumbnailSize)
	{
	case EFastAssetsThumbnailSize::Small:
		return 64;
	case EFastAssetsThumbnailSize::Medium:
		return 128;
	case EFastAssetsThumbnailSize::Large:
		return 256;
	default:
		return 128;
	}
}

void UFastAssetsSettings::AddRecentPath(const FString& Path)
{
	if (Path.IsEmpty())
	{
		return;
	}

	// Remove if already exists
	RecentPaths.Remove(Path);

	// Add to the beginning
	RecentPaths.Insert(Path, 0);

	// Trim to max count
	while (RecentPaths.Num() > RecentPathsCount)
	{
		RecentPaths.RemoveAt(RecentPaths.Num() - 1);
	}

	// Update last browsed path
	LastBrowsedPath = Path;

	// Save config
	SaveConfig();
}

FString UFastAssetsSettings::GetTargetFolder(const FString& AssetType) const
{
	for (const FFastAssetsFolderMapping& Mapping : FolderMappings)
	{
		if (Mapping.AssetType.Equals(AssetType, ESearchCase::IgnoreCase))
		{
			return BaseImportPath / Mapping.TargetFolder;
		}
	}

	// Default to Misc
	return BaseImportPath / TEXT("Misc");
}

void UFastAssetsSettings::ResetToDefaults()
{
	// General
	DefaultViewMode = EFastAssetsDefaultView::Grid;
	ThumbnailSize = EFastAssetsThumbnailSize::Medium;
	bDisableThumbnails = false;
	bRememberLastPath = true;
	RecentPathsCount = 10;
	RecentPaths.Empty();
	LastBrowsedPath = TEXT("");

	// Import
	bAutoImportOnDrop = true;
	bShowImportDialog = false;
	DuplicateHandling = EFastAssetsDuplicateHandling::Ask;

	// Organization
	bAutoCreateFolders = true;
	BaseImportPath = TEXT("/Game/Assets");

	FolderMappings.Empty();
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Mesh"), TEXT("Meshes")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Sound"), TEXT("Sounds")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Texture"), TEXT("Textures")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Material"), TEXT("Materials")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Animation"), TEXT("Animations")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Blueprint"), TEXT("Blueprints")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Data"), TEXT("Data")));
	FolderMappings.Add(FFastAssetsFolderMapping(TEXT("Other"), TEXT("Misc")));

	// Scanning
	bRecursiveScan = true;
	MaxScanDepth = 0;
	IncludeExtensions.Empty();
	ExcludeExtensions.Empty();

	SaveConfig();
}

#undef LOCTEXT_NAMESPACE
