// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "FAssetDragDropOp.h"
#include "FastAssetImporter.h"

class ILevelEditor;
class SWidget;

class FFastAssetsDropHandler : public TSharedFromThis<FFastAssetsDropHandler>
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
	/** Register the viewport drop overlay widget */
	void RegisterViewportDropHandler();

	/** Unregister the viewport drop overlay widget */
	void UnregisterViewportDropHandler();

	/** Register content browser drop handler */
	void RegisterContentBrowserDropHandler();

	/** Unregister content browser drop handler */
	void UnregisterContentBrowserDropHandler();

	/** Called when a level editor is created */
	void OnLevelEditorCreated(TSharedPtr<ILevelEditor> LevelEditor);

private:
	FFastAssetImporter AssetImporter;
	bool bIsInitialized;

	/** Handle for level editor created delegate */
	FDelegateHandle LevelEditorCreatedHandle;

	/** Handle for content browser drag-drop delegate */
	FDelegateHandle ContentBrowserDropHandle;

	/** Weak reference to the viewport overlay widget */
	TWeakPtr<SWidget> ViewportOverlayWidget;
};
