// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "SAssetTableRow.h"
#include "FAssetDragDropOp.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FastAssets"

// ============================================================================
// SAssetListRow Implementation
// ============================================================================

void SAssetListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	AssetItem = InArgs._AssetItem;
	OnDragDetectedDelegate = InArgs._OnDragDetected;

	SMultiColumnTableRow<TSharedPtr<FExternalAssetItem>>::Construct(
		FSuperRowType::FArguments()
			.Padding(FMargin(0.0f, 2.0f)),
		InOwnerTable
	);
}

TSharedRef<SWidget> SAssetListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (!AssetItem.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	if (ColumnName == TEXT("Icon"))
	{
		return SNew(SBox)
			.WidthOverride(20)
			.HeightOverride(20)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ContentBrowser.ColumnViewAssetIcon"))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(AssetItem->AssetType.Left(1).ToUpper()))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			];
	}
	else if (ColumnName == TEXT("Name"))
	{
		return SNew(STextBlock)
			.Text(FText::FromString(AssetItem->FileName));
	}
	else if (ColumnName == TEXT("Type"))
	{
		return SNew(STextBlock)
			.Text(FText::FromString(AssetItem->AssetType));
	}
	else if (ColumnName == TEXT("Size"))
	{
		return SNew(STextBlock)
			.Text(FText::FromString(FormatFileSize(AssetItem->FileSize)));
	}
	else if (ColumnName == TEXT("Extension"))
	{
		return SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT(".%s"), *AssetItem->Extension)));
	}

	return SNullWidget::NullWidget;
}

FReply SAssetListRow::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

FReply SAssetListRow::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (AssetItem.IsValid() && OnDragDetectedDelegate.IsBound())
	{
		return OnDragDetectedDelegate.Execute(AssetItem);
	}

	return FReply::Unhandled();
}

FString SAssetListRow::FormatFileSize(int64 SizeInBytes) const
{
	if (SizeInBytes < 1024)
	{
		return FString::Printf(TEXT("%lld B"), SizeInBytes);
	}
	else if (SizeInBytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f KB"), SizeInBytes / 1024.0);
	}
	else if (SizeInBytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f MB"), SizeInBytes / (1024.0 * 1024.0));
	}
	else
	{
		return FString::Printf(TEXT("%.1f GB"), SizeInBytes / (1024.0 * 1024.0 * 1024.0));
	}
}

// ============================================================================
// SAssetTile Implementation
// ============================================================================

void SAssetTile::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	AssetItem = InArgs._AssetItem;
	OnDragDetectedDelegate = InArgs._OnDragDetected;

	STableRow<TSharedPtr<FExternalAssetItem>>::Construct(
		STableRow<TSharedPtr<FExternalAssetItem>>::FArguments()
			.Padding(FMargin(4.0f)),
		InOwnerTable
	);

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(110)
		.HeightOverride(130)
		[
			SNew(SVerticalBox)

			// Thumbnail placeholder
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(100)
				.HeightOverride(100)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ContentBrowser.AssetTileItem.AssetBackground"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(AssetItem.IsValid() ? FText::FromString(AssetItem->AssetType.Left(1).ToUpper()) : FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					]
				]
			]

			// File name
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(AssetItem.IsValid() ? FText::FromString(AssetItem->FileName) : FText::GetEmpty())
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]
		]
	];
}

FReply SAssetTile::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	return STableRow<TSharedPtr<FExternalAssetItem>>::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SAssetTile::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (AssetItem.IsValid() && OnDragDetectedDelegate.IsBound())
	{
		return OnDragDetectedDelegate.Execute(AssetItem);
	}

	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE
