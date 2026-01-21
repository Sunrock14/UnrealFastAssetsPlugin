// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

class UTexture2D;

/**
 * Delegate called when a thumbnail becomes ready for a file
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnThumbnailReady, const FString& /*FilePath*/);

/**
 * Manages thumbnail loading and caching for external assets
 */
class FASTASSETS_API FFastAssetsThumbnail
{
public:
	FFastAssetsThumbnail();
	~FFastAssetsThumbnail();

	/** Get singleton instance */
	static FFastAssetsThumbnail& Get();

	/** Load or get cached thumbnail for an asset */
	const FSlateBrush* GetThumbnailBrush(const FString& FilePath, const FString& AssetType);

	/** Get icon brush for asset type (for non-previewable assets) */
	const FSlateBrush* GetAssetTypeIcon(const FString& AssetType);

	/** Check if asset type supports image thumbnail */
	bool SupportsImageThumbnail(const FString& AssetType) const;

	/** Check if asset type supports mesh thumbnail (3D preview) */
	bool SupportsMeshThumbnail(const FString& AssetType) const;

	/** Get the loading placeholder brush (shown while mesh thumbnails generate) */
	const FSlateBrush* GetLoadingPlaceholderBrush() const;

	/** Clear thumbnail cache */
	void ClearCache();

	/** Pre-cache thumbnails for a list of files */
	void PreCacheThumbnails(const TArray<FString>& FilePaths, const TArray<FString>& AssetTypes);

	/** Check if a thumbnail is currently being generated for this file */
	bool IsThumbnailPending(const FString& FilePath) const;

	/** Delegate broadcast when a thumbnail becomes ready */
	FOnThumbnailReady OnThumbnailReady;

private:
	/** Load image file as brush */
	TSharedPtr<FSlateBrush> LoadImageAsBrush(const FString& FilePath);

	/** Create texture from image data */
	UTexture2D* CreateTextureFromImage(const FString& FilePath);

	/** Initialize asset type icons */
	void InitializeAssetTypeIcons();

	/** Initialize loading placeholder brush */
	void InitializeLoadingPlaceholder();

	/** Called when a mesh thumbnail has been generated */
	void OnMeshThumbnailGenerated(const FString& FilePath, UTexture2D* Thumbnail);

	/** Create brush from texture */
	TSharedPtr<FSlateBrush> CreateBrushFromTexture(UTexture2D* Texture);

private:
	/** Cached thumbnail brushes (FilePath -> Brush) */
	TMap<FString, TSharedPtr<FSlateBrush>> ThumbnailCache;

	/** Cached textures (need to hold references) */
	TMap<FString, UTexture2D*> TextureCache;

	/** Asset type icon brushes */
	TMap<FString, TSharedPtr<FSlateBrush>> AssetTypeIcons;

	/** Default brush for unknown types */
	TSharedPtr<FSlateBrush> DefaultBrush;

	/** Loading placeholder brush (shown while mesh thumbnails generate) */
	TSharedPtr<FSlateBrush> LoadingPlaceholderBrush;

	/** Files currently pending thumbnail generation */
	TSet<FString> PendingThumbnails;

	/** Thumbnail size */
	static const int32 ThumbnailSize = 128;

	/** Singleton instance */
	static TUniquePtr<FFastAssetsThumbnail> Instance;
};
