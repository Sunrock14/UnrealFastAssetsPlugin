// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FAssetDragDropOp.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FastAssets"

TSharedRef<FFastAssetDragDropOp> FFastAssetDragDropOp::New(const TArray<TSharedPtr<FExternalAssetItem>>& InAssets)
{
	TSharedRef<FFastAssetDragDropOp> Operation = MakeShared<FFastAssetDragDropOp>();
	Operation->DraggedAssets = InAssets;
	Operation->Construct();
	return Operation;
}

FText FFastAssetDragDropOp::GetDecoratorText() const
{
	if (DraggedAssets.Num() == 0)
	{
		return LOCTEXT("DragNoAssets", "No assets");
	}
	else if (DraggedAssets.Num() == 1)
	{
		return FText::FromString(DraggedAssets[0]->FileName);
	}
	else
	{
		return FText::Format(LOCTEXT("DragMultipleAssets", "{0} assets"), FText::AsNumber(DraggedAssets.Num()));
	}
}

void FFastAssetDragDropOp::Construct()
{
	FDecoratedDragDropOp::Construct();

	CurrentHoverText = GetDecoratorText();
	CurrentIconBrush = FAppStyle::GetBrush("ContentBrowser.AssetDragDropTooltipBackground");
}

TSharedPtr<SWidget> FFastAssetDragDropOp::GetDefaultDecorator() const
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ContentBrowser.AssetDragDropTooltipBackground"))
		.Padding(8.0f)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("ContentBrowser.ColumnViewAssetIcon"))
			]

			// Text
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(GetDecoratorText())
				.Font(FAppStyle::GetFontStyle("ContentBrowser.DraggedAssetCount"))
			]
		];
}

#undef LOCTEXT_NAMESPACE
