// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FFastAssetsDropHandler;
class FEditorViewportClient;

/**
 * Transparent overlay widget that sits on top of the level viewport
 * to intercept FastAssets drag-drop operations.
 */
class SFastAssetsViewportDropOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFastAssetsViewportDropOverlay)
	{}
		SLATE_ARGUMENT(TSharedPtr<FFastAssetsDropHandler>, DropHandler)
		SLATE_ARGUMENT(TWeakPtr<FEditorViewportClient>, ViewportClient)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// SWidget interface
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;

private:
	/** Calculate world location from screen position via ray cast */
	bool GetWorldLocationFromScreenPosition(const FVector2D& ScreenPosition, FVector& OutWorldLocation) const;

private:
	/** Reference to the drop handler */
	TSharedPtr<FFastAssetsDropHandler> DropHandler;

	/** Reference to the viewport client for deprojection */
	TWeakPtr<FEditorViewportClient> ViewportClient;

	/** Whether we're currently in a valid drag operation */
	bool bIsDragOver;
};
