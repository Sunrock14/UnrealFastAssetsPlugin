// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "SFastAssetsWindow.h"

DECLARE_DELEGATE_RetVal_OneParam(FReply, FOnAssetDragDetected, TSharedPtr<FExternalAssetItem>);

/**
 * Custom table row widget with drag support for list view
 */
class SAssetListRow : public SMultiColumnTableRow<TSharedPtr<FExternalAssetItem>>
{
public:
	SLATE_BEGIN_ARGS(SAssetListRow) {}
		SLATE_ARGUMENT(TSharedPtr<FExternalAssetItem>, AssetItem)
		SLATE_EVENT(FOnAssetDragDetected, OnDragDetected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable);
	virtual ~SAssetListRow();

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	FString FormatFileSize(int64 SizeInBytes) const;
	void OnThumbnailUpdated(const FString& FilePath);

private:
	TSharedPtr<FExternalAssetItem> AssetItem;
	FOnAssetDragDetected OnDragDetectedDelegate;
	FDelegateHandle ThumbnailReadyHandle;
};

/**
 * Custom tile widget with drag support for grid view
 */
class SAssetTile : public STableRow<TSharedPtr<FExternalAssetItem>>
{
public:
	SLATE_BEGIN_ARGS(SAssetTile) {}
		SLATE_ARGUMENT(TSharedPtr<FExternalAssetItem>, AssetItem)
		SLATE_EVENT(FOnAssetDragDetected, OnDragDetected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable);
	virtual ~SAssetTile();

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	void OnThumbnailUpdated(const FString& FilePath);
	void RebuildTileContent();

private:
	TSharedPtr<FExternalAssetItem> AssetItem;
	FOnAssetDragDetected OnDragDetectedDelegate;
	FDelegateHandle ThumbnailReadyHandle;
	TSharedPtr<STableViewBase> OwnerTable;
};
