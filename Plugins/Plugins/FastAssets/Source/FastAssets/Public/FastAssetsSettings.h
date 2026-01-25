// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FastAssetsSettings.generated.h"

UENUM(BlueprintType)
enum class EFastAssetsDefaultView : uint8
{
	Grid UMETA(DisplayName = "Grid View"),
	List UMETA(DisplayName = "List View")
};

UENUM(BlueprintType)
enum class EFastAssetsDuplicateHandling : uint8
{
	Ask UMETA(DisplayName = "Ask Every Time"),
	Skip UMETA(DisplayName = "Skip Duplicates"),
	Replace UMETA(DisplayName = "Replace Existing"),
	Rename UMETA(DisplayName = "Auto Rename")
};

UENUM(BlueprintType)
enum class EFastAssetsThumbnailSize : uint8
{
	Small UMETA(DisplayName = "Small (64px)"),
	Medium UMETA(DisplayName = "Medium (128px)"),
	Large UMETA(DisplayName = "Large (256px)")
};

USTRUCT(BlueprintType)
struct FFastAssetsFolderMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Folder Mapping")
	FString AssetType;

	UPROPERTY(EditAnywhere, Category = "Folder Mapping")
	FString TargetFolder;

	FFastAssetsFolderMapping()
		: AssetType(TEXT(""))
		, TargetFolder(TEXT(""))
	{
	}

	FFastAssetsFolderMapping(const FString& InType, const FString& InFolder)
		: AssetType(InType)
		, TargetFolder(InFolder)
	{
	}
};

/**
 * Settings for Fast Assets plugin
 * These settings appear in Project Settings > Plugins > Fast Assets
 */
UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "Fast Assets"))
class FASTASSETS_API UFastAssetsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFastAssetsSettings();

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
	virtual FName GetSectionName() const override { return FName(TEXT("Fast Assets")); }

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return NSLOCTEXT("FastAssets", "SettingsSection", "Fast Assets"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("FastAssets", "SettingsDesc", "Configure Fast Assets plugin settings"); }
#endif

	/** Get the settings instance */
	static UFastAssetsSettings* Get();

	// ========== General Settings ==========

	/** Default view mode when opening the Fast Assets window */
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Default View Mode"))
	EFastAssetsDefaultView DefaultViewMode;

	/** Thumbnail size in grid view */
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Thumbnail Size"))
	EFastAssetsThumbnailSize ThumbnailSize;

	/** Disable thumbnail generation for faster list performance */
	UPROPERTY(config, EditAnywhere, Category = "Performance", meta = (DisplayName = "Disable Thumbnails"))
	bool bDisableThumbnails;

	/** Remember and restore the last browsed folder path */
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Remember Last Path"))
	bool bRememberLastPath;

	/** Last browsed folder path (auto-saved) */
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Last Path"))
	FString LastBrowsedPath;

	/** Number of recent paths to remember */
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Recent Paths Count", ClampMin = "0", ClampMax = "20"))
	int32 RecentPathsCount;

	/** List of recent paths */
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Recent Paths"))
	TArray<FString> RecentPaths;

	// ========== Import Settings ==========

	/** Automatically import assets when dropped (without showing import dialog) */
	UPROPERTY(config, EditAnywhere, Category = "Import", meta = (DisplayName = "Auto Import on Drop"))
	bool bAutoImportOnDrop;

	/** Show the import settings dialog before importing */
	UPROPERTY(config, EditAnywhere, Category = "Import", meta = (DisplayName = "Show Import Dialog"))
	bool bShowImportDialog;

	/** How to handle duplicate assets */
	UPROPERTY(config, EditAnywhere, Category = "Import", meta = (DisplayName = "Duplicate Handling"))
	EFastAssetsDuplicateHandling DuplicateHandling;

	// ========== Organization Settings ==========

	/** Automatically create folders if they don't exist */
	UPROPERTY(config, EditAnywhere, Category = "Organization", meta = (DisplayName = "Auto Create Folders"))
	bool bAutoCreateFolders;

	/** Base path for imported assets */
	UPROPERTY(config, EditAnywhere, Category = "Organization", meta = (DisplayName = "Base Import Path"))
	FString BaseImportPath;

	/** Custom folder mappings (asset type -> folder) */
	UPROPERTY(config, EditAnywhere, Category = "Organization", meta = (DisplayName = "Folder Mappings"))
	TArray<FFastAssetsFolderMapping> FolderMappings;

	// ========== Scanning Settings ==========

	/** Scan subfolders recursively */
	UPROPERTY(config, EditAnywhere, Category = "Scanning", meta = (DisplayName = "Recursive Scan"))
	bool bRecursiveScan;

	/** Maximum folder depth for recursive scanning (0 = unlimited) */
	UPROPERTY(config, EditAnywhere, Category = "Scanning", meta = (DisplayName = "Max Scan Depth", ClampMin = "0", ClampMax = "100"))
	int32 MaxScanDepth;

	/** File extensions to include (empty = all supported) */
	UPROPERTY(config, EditAnywhere, Category = "Scanning", meta = (DisplayName = "Include Extensions"))
	TArray<FString> IncludeExtensions;

	/** File extensions to exclude */
	UPROPERTY(config, EditAnywhere, Category = "Scanning", meta = (DisplayName = "Exclude Extensions"))
	TArray<FString> ExcludeExtensions;

public:
	/** Get thumbnail size in pixels */
	int32 GetThumbnailSizePixels() const;

	/** Add a path to recent paths */
	void AddRecentPath(const FString& Path);

	/** Get the target folder for an asset type */
	FString GetTargetFolder(const FString& AssetType) const;

	/** Reset to default settings */
	void ResetToDefaults();
};
