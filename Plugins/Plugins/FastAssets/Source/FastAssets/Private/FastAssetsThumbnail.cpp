// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FastAssetsThumbnail.h"
#include "FastAssetsMeshThumbnailGenerator.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleRegistry.h"

TUniquePtr<FFastAssetsThumbnail> FFastAssetsThumbnail::Instance = nullptr;

FFastAssetsThumbnail::FFastAssetsThumbnail()
{
	InitializeAssetTypeIcons();
	InitializeLoadingPlaceholder();
}

FFastAssetsThumbnail::~FFastAssetsThumbnail()
{
	ClearCache();
}

FFastAssetsThumbnail& FFastAssetsThumbnail::Get()
{
	if (!Instance.IsValid())
	{
		Instance = MakeUnique<FFastAssetsThumbnail>();
	}
	return *Instance;
}

void FFastAssetsThumbnail::InitializeAssetTypeIcons()
{
	// Create colored brushes for different asset types using FSlateColorBrush
	// which properly renders a solid color without needing an external texture

	// Mesh icon - Blue/Cyan
	{
		TSharedPtr<FSlateBrush> MeshBrush = MakeShared<FSlateBrush>();
		*MeshBrush = *FAppStyle::GetBrush("WhiteBrush");
		MeshBrush->TintColor = FSlateColor(FLinearColor(0.15f, 0.4f, 0.7f, 1.0f));
		MeshBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
		AssetTypeIcons.Add(TEXT("Mesh"), MeshBrush);
	}

	// Sound icon - Green
	{
		TSharedPtr<FSlateBrush> SoundBrush = MakeShared<FSlateBrush>();
		*SoundBrush = *FAppStyle::GetBrush("WhiteBrush");
		SoundBrush->TintColor = FSlateColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f));
		SoundBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
		AssetTypeIcons.Add(TEXT("Sound"), SoundBrush);
	}

	// Texture icon - Magenta/Pink
	{
		TSharedPtr<FSlateBrush> TextureBrush = MakeShared<FSlateBrush>();
		*TextureBrush = *FAppStyle::GetBrush("WhiteBrush");
		TextureBrush->TintColor = FSlateColor(FLinearColor(0.7f, 0.3f, 0.5f, 1.0f));
		TextureBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
		AssetTypeIcons.Add(TEXT("Texture"), TextureBrush);
	}

	// UAsset icon - Orange
	{
		TSharedPtr<FSlateBrush> UAssetBrush = MakeShared<FSlateBrush>();
		*UAssetBrush = *FAppStyle::GetBrush("WhiteBrush");
		UAssetBrush->TintColor = FSlateColor(FLinearColor(0.8f, 0.5f, 0.1f, 1.0f));
		UAssetBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
		AssetTypeIcons.Add(TEXT("UAsset"), UAssetBrush);
	}

	// Map icon - Purple
	{
		TSharedPtr<FSlateBrush> MapBrush = MakeShared<FSlateBrush>();
		*MapBrush = *FAppStyle::GetBrush("WhiteBrush");
		MapBrush->TintColor = FSlateColor(FLinearColor(0.5f, 0.2f, 0.7f, 1.0f));
		MapBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
		AssetTypeIcons.Add(TEXT("Map"), MapBrush);
	}

	// Data icon - Yellow
	{
		TSharedPtr<FSlateBrush> DataBrush = MakeShared<FSlateBrush>();
		*DataBrush = *FAppStyle::GetBrush("WhiteBrush");
		DataBrush->TintColor = FSlateColor(FLinearColor(0.8f, 0.8f, 0.2f, 1.0f));
		DataBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
		AssetTypeIcons.Add(TEXT("Data"), DataBrush);
	}

	// Default/Other icon - Gray
	{
		DefaultBrush = MakeShared<FSlateBrush>();
		*DefaultBrush = *FAppStyle::GetBrush("WhiteBrush");
		DefaultBrush->TintColor = FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
		DefaultBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
		AssetTypeIcons.Add(TEXT("Other"), DefaultBrush);
	}
}

void FFastAssetsThumbnail::InitializeLoadingPlaceholder()
{
	// Create a loading placeholder brush - dark gray with slight animation hint
	LoadingPlaceholderBrush = MakeShared<FSlateBrush>();
	*LoadingPlaceholderBrush = *FAppStyle::GetBrush("WhiteBrush");
	LoadingPlaceholderBrush->TintColor = FSlateColor(FLinearColor(0.2f, 0.25f, 0.3f, 1.0f));
	LoadingPlaceholderBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
}

bool FFastAssetsThumbnail::SupportsImageThumbnail(const FString& AssetType) const
{
	return AssetType == TEXT("Texture");
}

bool FFastAssetsThumbnail::SupportsMeshThumbnail(const FString& AssetType) const
{
	return AssetType == TEXT("Mesh");
}

const FSlateBrush* FFastAssetsThumbnail::GetLoadingPlaceholderBrush() const
{
	return LoadingPlaceholderBrush.Get();
}

bool FFastAssetsThumbnail::IsThumbnailPending(const FString& FilePath) const
{
	return PendingThumbnails.Contains(FilePath);
}

const FSlateBrush* FFastAssetsThumbnail::GetThumbnailBrush(const FString& FilePath, const FString& AssetType)
{
	UE_LOG(LogTemp, Verbose, TEXT("FastAssets: GetThumbnailBrush called for: %s (Type: %s)"), *FilePath, *AssetType);

	// Check cache first for any type
	if (TSharedPtr<FSlateBrush>* CachedBrush = ThumbnailCache.Find(FilePath))
	{
		if (CachedBrush->IsValid())
		{
			return CachedBrush->Get();
		}
	}

	// For textures, try to load actual image thumbnail
	if (SupportsImageThumbnail(AssetType))
	{
		// Try to load the image
		TSharedPtr<FSlateBrush> LoadedBrush = LoadImageAsBrush(FilePath);
		if (LoadedBrush.IsValid())
		{
			ThumbnailCache.Add(FilePath, LoadedBrush);
			return LoadedBrush.Get();
		}
	}

	// For meshes, request async thumbnail generation
	if (SupportsMeshThumbnail(AssetType))
	{
		FString Extension = FPaths::GetExtension(FilePath);
		if (FFastAssetsMeshThumbnailGenerator::Get().SupportsMeshThumbnail(Extension))
		{
			// Check if already pending
			if (!PendingThumbnails.Contains(FilePath))
			{
				PendingThumbnails.Add(FilePath);
				UE_LOG(LogTemp, Log, TEXT("FastAssets: Requesting mesh thumbnail for: %s"), *FilePath);

				// Request async thumbnail generation
				FFastAssetsMeshThumbnailGenerator::Get().RequestThumbnail(FilePath,
					[this, FilePath](UTexture2D* Thumbnail)
					{
						OnMeshThumbnailGenerated(FilePath, Thumbnail);
					});
			}

			// Return loading placeholder while generating
			return GetLoadingPlaceholderBrush();
		}
	}

	// Fall back to asset type icon
	return GetAssetTypeIcon(AssetType);
}

const FSlateBrush* FFastAssetsThumbnail::GetAssetTypeIcon(const FString& AssetType)
{
	if (TSharedPtr<FSlateBrush>* IconBrush = AssetTypeIcons.Find(AssetType))
	{
		return IconBrush->Get();
	}

	return DefaultBrush.Get();
}

TSharedPtr<FSlateBrush> FFastAssetsThumbnail::LoadImageAsBrush(const FString& FilePath)
{
	// Load image data from file
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to load file: %s"), *FilePath);
		return nullptr;
	}

	// Determine image format from extension
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	EImageFormat ImageFormat = EImageFormat::Invalid;

	if (Extension == TEXT("png"))
	{
		ImageFormat = EImageFormat::PNG;
	}
	else if (Extension == TEXT("jpg") || Extension == TEXT("jpeg"))
	{
		ImageFormat = EImageFormat::JPEG;
	}
	else if (Extension == TEXT("bmp"))
	{
		ImageFormat = EImageFormat::BMP;
	}
	else if (Extension == TEXT("exr"))
	{
		ImageFormat = EImageFormat::EXR;
	}
	else if (Extension == TEXT("tga"))
	{
		ImageFormat = EImageFormat::TGA;
	}
	else if (Extension == TEXT("hdr"))
	{
		ImageFormat = EImageFormat::HDR;
	}

	if (ImageFormat == EImageFormat::Invalid)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Unsupported image format for: %s"), *FilePath);
		return nullptr;
	}

	// Get image wrapper module
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

	if (!ImageWrapper.IsValid())
	{
		return nullptr;
	}

	// Decompress the image
	if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to decompress image: %s"), *FilePath);
		return nullptr;
	}

	// Get raw data
	TArray<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to get raw data from image: %s"), *FilePath);
		return nullptr;
	}

	// Create texture
	int32 Width = ImageWrapper->GetWidth();
	int32 Height = ImageWrapper->GetHeight();

	// Calculate thumbnail size (maintain aspect ratio)
	int32 ThumbWidth = ThumbnailSize;
	int32 ThumbHeight = ThumbnailSize;

	if (Width > Height)
	{
		ThumbHeight = FMath::RoundToInt((float)ThumbnailSize * Height / Width);
	}
	else
	{
		ThumbWidth = FMath::RoundToInt((float)ThumbnailSize * Width / Height);
	}

	// Create texture
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to create transient texture for: %s"), *FilePath);
		return nullptr;
	}

	// Prevent garbage collection
	Texture->AddToRoot();

	// Lock and copy data
	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update resource
	Texture->UpdateResource();

	// Store texture reference
	TextureCache.Add(FilePath, Texture);

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully loaded thumbnail for: %s (%dx%d)"), *FilePath, Width, Height);

	// Create brush from texture
	TSharedPtr<FSlateBrush> NewBrush = MakeShared<FSlateBrush>();
	NewBrush->SetResourceObject(Texture);
	NewBrush->ImageSize = FVector2D(ThumbWidth, ThumbHeight);
	NewBrush->DrawAs = ESlateBrushDrawType::Image;
	NewBrush->Tiling = ESlateBrushTileType::NoTile;

	return NewBrush;
}

UTexture2D* FFastAssetsThumbnail::CreateTextureFromImage(const FString& FilePath)
{
	// This is a simplified version - the actual loading is in LoadImageAsBrush
	return nullptr;
}

void FFastAssetsThumbnail::OnMeshThumbnailGenerated(const FString& FilePath, UTexture2D* Thumbnail)
{
	// Remove from pending
	PendingThumbnails.Remove(FilePath);

	if (Thumbnail)
	{
		// Create brush from texture and cache it
		TSharedPtr<FSlateBrush> NewBrush = CreateBrushFromTexture(Thumbnail);
		if (NewBrush.IsValid())
		{
			ThumbnailCache.Add(FilePath, NewBrush);
			TextureCache.Add(FilePath, Thumbnail);

			UE_LOG(LogTemp, Log, TEXT("FastAssets: Mesh thumbnail ready for: %s"), *FilePath);

			// Broadcast that thumbnail is ready
			OnThumbnailReady.Broadcast(FilePath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to generate mesh thumbnail for: %s"), *FilePath);
	}
}

TSharedPtr<FSlateBrush> FFastAssetsThumbnail::CreateBrushFromTexture(UTexture2D* Texture)
{
	if (!Texture)
	{
		return nullptr;
	}

	TSharedPtr<FSlateBrush> NewBrush = MakeShared<FSlateBrush>();
	NewBrush->SetResourceObject(Texture);
	NewBrush->ImageSize = FVector2D(ThumbnailSize, ThumbnailSize);
	NewBrush->DrawAs = ESlateBrushDrawType::Image;
	NewBrush->Tiling = ESlateBrushTileType::NoTile;

	return NewBrush;
}

void FFastAssetsThumbnail::ClearCache()
{
	ThumbnailCache.Empty();
	PendingThumbnails.Empty();

	// Remove textures from root before clearing
	for (auto& Pair : TextureCache)
	{
		if (Pair.Value && Pair.Value->IsValidLowLevel())
		{
			Pair.Value->RemoveFromRoot();
		}
	}
	TextureCache.Empty();

	// Also clear mesh thumbnail generator cache
	FFastAssetsMeshThumbnailGenerator::Get().ClearCache();
}

void FFastAssetsThumbnail::PreCacheThumbnails(const TArray<FString>& FilePaths, const TArray<FString>& AssetTypes)
{
	// Pre-cache thumbnails for faster display
	for (int32 i = 0; i < FilePaths.Num() && i < AssetTypes.Num(); i++)
	{
		if (SupportsImageThumbnail(AssetTypes[i]))
		{
			GetThumbnailBrush(FilePaths[i], AssetTypes[i]);
		}
	}
}
