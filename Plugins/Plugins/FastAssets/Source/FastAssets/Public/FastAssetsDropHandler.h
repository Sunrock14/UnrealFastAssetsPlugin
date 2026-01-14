// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "FAssetDragDropOp.h"
#include "FastAssetImporter.h"

class FFastAssetsDropHandler
{
public:
	FFastAssetsDropHandler();
	~FFastAssetsDropHandler();

	/** Initialize and register drop handlers */
	void Initialize();

	/** Cleanup and unregister drop handlers */
	void Shutdown();

	/** Handle drop operation in viewport */
	bool HandleViewportDrop(const FGeometry& Geometry, const FDragDropEvent& DragDropEvent, const FVector& DropLocation);

	/** Handle drop operation in content browser */
	bool HandleContentBrowserDrop(const FString& DestinationPath, const FDragDropEvent& DragDropEvent);

	/** Check if the drag operation is a FastAsset drag */
	static bool IsFastAssetDrag(const FDragDropEvent& DragDropEvent);

	/** Get the importer instance */
	FFastAssetImporter& GetImporter() { return AssetImporter; }

private:
	/** Import and spawn assets at the given world location */
	bool ImportAndSpawnAssets(const TArray<TSharedPtr<FExternalAssetItem>>& Assets, UWorld* World, const FVector& Location);

	/** Import assets to the project */
	bool ImportAssetsToProject(const TArray<TSharedPtr<FExternalAssetItem>>& Assets, const FString& DestinationPath);

private:
	FFastAssetImporter AssetImporter;
	bool bIsInitialized;
};
