// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "SAssetTableRow.h"
#include "FAssetDragDropOp.h"
#include "FastAssetsThumbnail.h"
#include "FastAssetsSettings.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FastAssets"

namespace FastAssetsColors
{
	FLinearColor GetColorForAssetType(const FString& AssetType)
	{
		if (AssetType == TEXT("Mesh"))
		{
			return FLinearColor(0.15f, 0.4f, 0.75f, 1.0f); // Blue
		}
		else if (AssetType == TEXT("Sound"))
		{
			return FLinearColor(0.2f, 0.65f, 0.2f, 1.0f); // Green
		}
		else if (AssetType == TEXT("Texture"))
		{
			return FLinearColor(0.75f, 0.3f, 0.55f, 1.0f); // Pink/Magenta
		}
		else if (AssetType == TEXT("UAsset"))
		{
			return FLinearColor(0.85f, 0.55f, 0.1f, 1.0f); // Orange
		}
		else if (AssetType == TEXT("Map"))
		{
			return FLinearColor(0.55f, 0.25f, 0.75f, 1.0f); // Purple
		}
		else if (AssetType == TEXT("Data"))
		{
			return FLinearColor(0.85f, 0.85f, 0.2f, 1.0f); // Yellow
		}
		else
		{
			return FLinearColor(0.4f, 0.4f, 0.4f, 1.0f); // Gray
		}
	}
}

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

	// Subscribe to thumbnail ready events for mesh types
	if (AssetItem.IsValid() && AssetItem->AssetType == TEXT("Mesh"))
	{
		ThumbnailReadyHandle = FFastAssetsThumbnail::Get().OnThumbnailReady.AddRaw(
			this, &SAssetListRow::OnThumbnailUpdated);
	}
}

SAssetListRow::~SAssetListRow()
{
	if (ThumbnailReadyHandle.IsValid())
	{
		FFastAssetsThumbnail::Get().OnThumbnailReady.Remove(ThumbnailReadyHandle);
	}
}

TSharedRef<SWidget> SAssetListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (!AssetItem.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	if (ColumnName == TEXT("Icon"))
	{
		FLinearColor IconColor = FastAssetsColors::GetColorForAssetType(AssetItem->AssetType);

		// Check if thumbnails are disabled in settings
		bool bThumbnailsDisabled = UFastAssetsSettings::Get()->bDisableThumbnails;
		bool bHasThumbnail = false;

		if (!bThumbnailsDisabled)
		{
			// Lazy load thumbnail if not already loaded
			if (AssetItem->ThumbnailBrush == nullptr)
			{
				UE_LOG(LogTemp, Log, TEXT("FastAssets: Lazy loading thumbnail for list row: %s"), *AssetItem->FilePath);
				AssetItem->ThumbnailBrush = FFastAssetsThumbnail::Get().GetThumbnailBrush(
					AssetItem->FilePath,
					AssetItem->AssetType
				);
				UE_LOG(LogTemp, Log, TEXT("FastAssets: Thumbnail loaded, brush is %s"), AssetItem->ThumbnailBrush ? TEXT("valid") : TEXT("null"));
			}

			// Check if we have a real thumbnail (for texture files or mesh files with generated thumbnail)
			bHasThumbnail = (AssetItem->ThumbnailBrush != nullptr) &&
				(AssetItem->AssetType == TEXT("Texture") ||
				 (AssetItem->AssetType == TEXT("Mesh") && !FFastAssetsThumbnail::Get().IsThumbnailPending(AssetItem->FilePath)));
		}

		if (bHasThumbnail)
		{
			return SNew(SBox)
				.WidthOverride(24)
				.HeightOverride(24)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(AssetItem->ThumbnailBrush)
				];
		}

		// Use colored background with type letter
		return SNew(SBox)
			.WidthOverride(24)
			.HeightOverride(24)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(IconColor)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(AssetItem->AssetType.Left(1).ToUpper()))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
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

void SAssetListRow::OnThumbnailUpdated(const FString& FilePath)
{
	if (AssetItem.IsValid() && AssetItem->FilePath == FilePath)
	{
		// Update the cached brush
		AssetItem->ThumbnailBrush = FFastAssetsThumbnail::Get().GetThumbnailBrush(
			AssetItem->FilePath, AssetItem->AssetType);

		// Request widget refresh
		Invalidate(EInvalidateWidget::Paint);

		UE_LOG(LogTemp, Log, TEXT("FastAssets: List row thumbnail updated for: %s"), *FilePath);
	}
}

// ============================================================================
// SAssetTile Implementation
// ============================================================================

void SAssetTile::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	AssetItem = InArgs._AssetItem;
	OnDragDetectedDelegate = InArgs._OnDragDetected;
	OwnerTable = InOwnerTable;

	STableRow<TSharedPtr<FExternalAssetItem>>::Construct(
		STableRow<TSharedPtr<FExternalAssetItem>>::FArguments()
			.Padding(FMargin(4.0f)),
		InOwnerTable
	);

	// Subscribe to thumbnail ready events for mesh types
	if (AssetItem.IsValid() && AssetItem->AssetType == TEXT("Mesh"))
	{
		ThumbnailReadyHandle = FFastAssetsThumbnail::Get().OnThumbnailReady.AddRaw(
			this, &SAssetTile::OnThumbnailUpdated);
	}

	RebuildTileContent();
}

SAssetTile::~SAssetTile()
{
	if (ThumbnailReadyHandle.IsValid())
	{
		FFastAssetsThumbnail::Get().OnThumbnailReady.Remove(ThumbnailReadyHandle);
	}
}

void SAssetTile::OnThumbnailUpdated(const FString& FilePath)
{
	if (AssetItem.IsValid() && AssetItem->FilePath == FilePath)
	{
		// Update the cached brush
		AssetItem->ThumbnailBrush = FFastAssetsThumbnail::Get().GetThumbnailBrush(
			AssetItem->FilePath, AssetItem->AssetType);

		// Rebuild the tile content with the new thumbnail
		RebuildTileContent();

		UE_LOG(LogTemp, Log, TEXT("FastAssets: Tile thumbnail updated for: %s"), *FilePath);
	}
}

void SAssetTile::RebuildTileContent()
{
	// Get color for this asset type
	FLinearColor TileColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
	FString TypeLetter = TEXT("?");
	bool bHasThumbnail = false;

	// Check if thumbnails are disabled in settings
	bool bThumbnailsDisabled = UFastAssetsSettings::Get()->bDisableThumbnails;

	if (AssetItem.IsValid())
	{
		TileColor = FastAssetsColors::GetColorForAssetType(AssetItem->AssetType);
		TypeLetter = AssetItem->AssetType.Left(1).ToUpper();

		if (!bThumbnailsDisabled)
		{
			// Lazy load thumbnail if not already loaded
			if (AssetItem->ThumbnailBrush == nullptr)
			{
				UE_LOG(LogTemp, Log, TEXT("FastAssets: Lazy loading thumbnail for tile: %s"), *AssetItem->FilePath);
				AssetItem->ThumbnailBrush = FFastAssetsThumbnail::Get().GetThumbnailBrush(
					AssetItem->FilePath,
					AssetItem->AssetType
				);
				UE_LOG(LogTemp, Log, TEXT("FastAssets: Tile thumbnail loaded, brush is %s"), AssetItem->ThumbnailBrush ? TEXT("valid") : TEXT("null"));
			}

			// Check if we have a real thumbnail (for texture files or mesh files with generated thumbnail)
			bHasThumbnail = (AssetItem->ThumbnailBrush != nullptr) &&
				(AssetItem->AssetType == TEXT("Texture") ||
				 (AssetItem->AssetType == TEXT("Mesh") && !FFastAssetsThumbnail::Get().IsThumbnailPending(AssetItem->FilePath)));

			UE_LOG(LogTemp, Log, TEXT("FastAssets: bHasThumbnail = %s for %s"), bHasThumbnail ? TEXT("true") : TEXT("false"), *AssetItem->FileName);
		}
	}

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(110)
		.HeightOverride(130)
		[
			SNew(SVerticalBox)

			// Thumbnail
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(100)
				.HeightOverride(100)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(TileColor)
					.Padding(4.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						bHasThumbnail ?
						// Real thumbnail for textures and meshes
						StaticCastSharedRef<SWidget>(
							SNew(SImage)
							.Image(AssetItem->ThumbnailBrush)
						)
						:
						// Type letter for other assets or loading state
						StaticCastSharedRef<SWidget>(
							SNew(STextBlock)
							.Text(FText::FromString(TypeLetter))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
							.ColorAndOpacity(FSlateColor(FLinearColor::White))
						)
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
