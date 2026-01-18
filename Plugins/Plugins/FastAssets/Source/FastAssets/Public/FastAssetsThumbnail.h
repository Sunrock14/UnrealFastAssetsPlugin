// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

class UTexture2D;

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

	/** Clear thumbnail cache */
	void ClearCache();

	/** Pre-cache thumbnails for a list of files */
	void PreCacheThumbnails(const TArray<FString>& FilePaths, const TArray<FString>& AssetTypes);

private:
	/** Load image file as brush */
	TSharedPtr<FSlateBrush> LoadImageAsBrush(const FString& FilePath);

	/** Create texture from image data */
	UTexture2D* CreateTextureFromImage(const FString& FilePath);

	/** Initialize asset type icons */
	void InitializeAssetTypeIcons();

private:
	/** Cached thumbnail brushes (FilePath -> Brush) */
	TMap<FString, TSharedPtr<FSlateBrush>> ThumbnailCache;

	/** Cached textures (need to hold references) */
	TMap<FString, UTexture2D*> TextureCache;

	/** Asset type icon brushes */
	TMap<FString, TSharedPtr<FSlateBrush>> AssetTypeIcons;

	/** Default brush for unknown types */
	TSharedPtr<FSlateBrush> DefaultBrush;

	/** Thumbnail size */
	static const int32 ThumbnailSize = 128;

	/** Singleton instance */
	static TUniquePtr<FFastAssetsThumbnail> Instance;
};
