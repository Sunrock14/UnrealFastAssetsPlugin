// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

class UTexture2D;
class UStaticMesh;

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

	/** Check if asset type supports 3D model thumbnail */
	bool Supports3DModelThumbnail(const FString& AssetType) const;

	/** Clear thumbnail cache */
	void ClearCache();

	/** Pre-cache thumbnails for a list of files */
	void PreCacheThumbnails(const TArray<FString>& FilePaths, const TArray<FString>& AssetTypes);

private:
	/** Load image file as brush */
	TSharedPtr<FSlateBrush> LoadImageAsBrush(const FString& FilePath);

	/** Load 3D model file and render thumbnail as brush */
	TSharedPtr<FSlateBrush> Load3DModelAsBrush(const FString& FilePath);

	/** Import FBX/OBJ to a transient UStaticMesh */
	UStaticMesh* ImportToTransientMesh(const FString& FilePath);

	/** Render mesh thumbnail and return as brush */
	TSharedPtr<FSlateBrush> RenderMeshThumbnail(UStaticMesh* Mesh, const FString& FilePath);

	/** Convert FObjectThumbnail to UTexture2D */
	UTexture2D* ConvertThumbnailToTexture(const FObjectThumbnail& Thumbnail);

	/** Cleanup transient mesh after rendering */
	void CleanupTransientMesh(UStaticMesh* Mesh);

	/** Create texture from image data */
	UTexture2D* CreateTextureFromImage(const FString& FilePath);

	/** Initialize asset type icons */
	void InitializeAssetTypeIcons();

	/** Check if file extension is a supported 3D model format */
	bool IsSupportedMeshFormat(const FString& Extension) const;

private:
	/** Cached thumbnail brushes (FilePath -> Brush) */
	TMap<FString, TSharedPtr<FSlateBrush>> ThumbnailCache;

	/** Cached textures (need to hold references) */
	TMap<FString, UTexture2D*> TextureCache;

	/** Asset type icon brushes */
	TMap<FString, TSharedPtr<FSlateBrush>> AssetTypeIcons;

	/** Default brush for unknown types */
	TSharedPtr<FSlateBrush> DefaultBrush;

	/** Pending meshes to cleanup */
	TArray<TWeakObjectPtr<UStaticMesh>> PendingCleanupMeshes;

	/** Thumbnail size for images */
	static const int32 ThumbnailSize = 128;

	/** Thumbnail size for 3D model rendering (higher quality) */
	static const int32 MeshThumbnailSize = 256;

	/** Maximum file size for 3D model import (100 MB) */
	static const int64 MaxMeshFileSize = 100 * 1024 * 1024;

	/** Singleton instance */
	static TUniquePtr<FFastAssetsThumbnail> Instance;
};
