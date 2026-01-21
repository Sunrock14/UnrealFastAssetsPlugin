// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

class UStaticMesh;
class UTexture2D;

/**
 * Delegate called when a mesh thumbnail has been generated
 */
DECLARE_DELEGATE_TwoParams(FOnMeshThumbnailGenerated, const FString& /*FilePath*/, UTexture2D* /*Thumbnail*/);

/**
 * Generates thumbnails for 3D mesh files (FBX, OBJ, GLTF, GLB)
 * Uses temporary import to UE, captures thumbnail, then cleans up
 */
class FASTASSETS_API FFastAssetsMeshThumbnailGenerator
{
public:
	FFastAssetsMeshThumbnailGenerator();
	~FFastAssetsMeshThumbnailGenerator();

	/** Get singleton instance */
	static FFastAssetsMeshThumbnailGenerator& Get();

	/** Check if the file extension is supported for mesh thumbnail generation */
	bool SupportsMeshThumbnail(const FString& Extension) const;

	/**
	 * Request thumbnail generation for a mesh file (non-blocking)
	 * @param FilePath - Full path to the external mesh file
	 * @param OnComplete - Callback when thumbnail is ready (may be called on game thread)
	 */
	void RequestThumbnail(const FString& FilePath, TFunction<void(UTexture2D*)> OnComplete);

	/** Check if a thumbnail request is already pending for this file */
	bool IsPendingRequest(const FString& FilePath) const;

	/** Clear all cached thumbnails */
	void ClearCache();

	/** Cancel any pending thumbnail requests */
	void CancelPendingRequests();

private:
	/**
	 * Import mesh file to a temporary package
	 * @return The imported static mesh, or nullptr if import failed
	 */
	UStaticMesh* ImportToTempPackage(const FString& FilePath);

	/**
	 * Capture thumbnail from a static mesh using UE's thumbnail system
	 * @return Generated texture, or nullptr if capture failed
	 */
	UTexture2D* CaptureThumbnailFromMesh(UStaticMesh* Mesh);

	/**
	 * Delete the temporary asset and clean up
	 */
	void DeleteTempAsset(UObject* Asset);

	/**
	 * Process pending thumbnail requests (called on game thread)
	 */
	bool Tick(float DeltaTime);

	/**
	 * Process one pending thumbnail request (called on game thread)
	 * @return true if a request was processed
	 */
	bool ProcessNextRequest();

private:
	/** Cached thumbnail textures (FilePath -> Texture) */
	TMap<FString, TWeakObjectPtr<UTexture2D>> ThumbnailCache;

	/** Pending thumbnail requests */
	TSet<FString> PendingRequests;

	/** Queue of requests to process */
	TArray<TPair<FString, TFunction<void(UTexture2D*)>>> RequestQueue;

	/** Lock for thread-safe access */
	FCriticalSection RequestLock;

	/** Whether we're currently processing requests */
	bool bIsProcessing;

	/** Ticker handle for processing requests on the game thread */
	FTSTicker::FDelegateHandle TickerHandle;

	/** Thumbnail size */
	static const int32 ThumbnailSize = 128;

	/** Temp folder path for mesh imports */
	static const FString TempFolderPath;

	/** Singleton instance */
	static TUniquePtr<FFastAssetsMeshThumbnailGenerator> Instance;
};
