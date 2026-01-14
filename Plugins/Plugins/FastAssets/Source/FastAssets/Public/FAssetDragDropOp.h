// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "SFastAssetsWindow.h"

class FFastAssetDragDropOp : public FDecoratedDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FFastAssetDragDropOp, FDecoratedDragDropOp)

	/** The external assets being dragged */
	TArray<TSharedPtr<FExternalAssetItem>> DraggedAssets;

	/** Creates a new drag operation */
	static TSharedRef<FFastAssetDragDropOp> New(const TArray<TSharedPtr<FExternalAssetItem>>& InAssets);

	/** Get the assets being dragged */
	const TArray<TSharedPtr<FExternalAssetItem>>& GetAssets() const { return DraggedAssets; }

	/** Get display text for the drag operation */
	FText GetDecoratorText() const;

public:
	virtual void Construct() override;
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
};
