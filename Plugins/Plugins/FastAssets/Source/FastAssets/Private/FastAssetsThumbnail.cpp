// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FastAssetsThumbnail.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleRegistry.h"
// For 3D model thumbnail rendering
#include "ThumbnailRendering/ThumbnailManager.h"
#include "ObjectTools.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxStaticMeshImportData.h"
#include "UObject/Package.h"
#include "ImageUtils.h"
#include "Misc/ObjectThumbnail.h"
#include "HAL/PlatformFileManager.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

TUniquePtr<FFastAssetsThumbnail> FFastAssetsThumbnail::Instance = nullptr;

FFastAssetsThumbnail::FFastAssetsThumbnail()
{
	InitializeAssetTypeIcons();
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

bool FFastAssetsThumbnail::SupportsImageThumbnail(const FString& AssetType) const
{
	return AssetType == TEXT("Texture");
}

bool FFastAssetsThumbnail::Supports3DModelThumbnail(const FString& AssetType) const
{
	return AssetType == TEXT("Mesh");
}

bool FFastAssetsThumbnail::IsSupportedMeshFormat(const FString& Extension) const
{
	FString LowerExt = Extension.ToLower();
	return LowerExt == TEXT("fbx") ||
		   LowerExt == TEXT("obj") ||
		   LowerExt == TEXT("gltf") ||
		   LowerExt == TEXT("glb");
}

const FSlateBrush* FFastAssetsThumbnail::GetThumbnailBrush(const FString& FilePath, const FString& AssetType)
{
	UE_LOG(LogTemp, Verbose, TEXT("FastAssets: GetThumbnailBrush called for: %s (Type: %s)"), *FilePath, *AssetType);

	// Check cache first (applies to all types)
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

	// For 3D models, try to render thumbnail
	if (Supports3DModelThumbnail(AssetType))
	{
		FString Extension = FPaths::GetExtension(FilePath);
		if (IsSupportedMeshFormat(Extension))
		{
			TSharedPtr<FSlateBrush> MeshBrush = Load3DModelAsBrush(FilePath);
			if (MeshBrush.IsValid())
			{
				ThumbnailCache.Add(FilePath, MeshBrush);
				return MeshBrush.Get();
			}
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

TSharedPtr<FSlateBrush> FFastAssetsThumbnail::Load3DModelAsBrush(const FString& FilePath)
{
	UE_LOG(LogTemp, Log, TEXT("FastAssets: Loading 3D model thumbnail for: %s"), *FilePath);

	// Check file size limit
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	int64 FileSize = PlatformFile.FileSize(*FilePath);

	if (FileSize < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Cannot get file size for: %s"), *FilePath);
		return nullptr;
	}

	if (FileSize > MaxMeshFileSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: File too large for thumbnail (%.1f MB > 100 MB): %s"),
			FileSize / (1024.0 * 1024.0), *FilePath);
		return nullptr;
	}

	// Import to transient mesh
	UStaticMesh* TransientMesh = ImportToTransientMesh(FilePath);
	if (!TransientMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to import mesh: %s"), *FilePath);
		return nullptr;
	}

	// Render thumbnail
	TSharedPtr<FSlateBrush> ResultBrush = RenderMeshThumbnail(TransientMesh, FilePath);

	// Cleanup transient mesh
	CleanupTransientMesh(TransientMesh);

	return ResultBrush;
}

UStaticMesh* FFastAssetsThumbnail::ImportToTransientMesh(const FString& FilePath)
{
	UE_LOG(LogTemp, Log, TEXT("FastAssets: Importing to transient mesh: %s"), *FilePath);

	FString Extension = FPaths::GetExtension(FilePath).ToLower();

	// Create unique package name for this import
	FString FileName = FPaths::GetBaseFilename(FilePath);
	FString UniquePackageName = FString::Printf(TEXT("/Temp/FastAssets/%s_%d"), *FileName, FMath::Rand());

	// Create transient package
	UPackage* TransientPackage = CreatePackage(*UniquePackageName);
	if (!TransientPackage)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to create transient package"));
		return nullptr;
	}

	TransientPackage->SetFlags(RF_Transient);

	UStaticMesh* ResultMesh = nullptr;

	if (Extension == TEXT("fbx") || Extension == TEXT("obj"))
	{
		// Use FbxFactory for FBX and OBJ files
		UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
		FbxFactory->AddToRoot(); // Prevent GC during import

		// Configure import settings for minimal import (just geometry, no materials/textures)
		UFbxImportUI* ImportUI = FbxFactory->ImportUI;
		if (ImportUI)
		{
			ImportUI->bImportMaterials = false;
			ImportUI->bImportAnimations = false;
			ImportUI->bImportAsSkeletal = false;
			ImportUI->bAutomatedImportShouldDetectType = false;
			ImportUI->MeshTypeToImport = FBXIT_StaticMesh;
			ImportUI->bIsObjImport = (Extension == TEXT("obj"));

			// Configure static mesh import data
			if (ImportUI->StaticMeshImportData)
			{
				ImportUI->StaticMeshImportData->bCombineMeshes = true;
				ImportUI->StaticMeshImportData->bAutoGenerateCollision = false;
				ImportUI->StaticMeshImportData->bRemoveDegenerates = true;
				ImportUI->StaticMeshImportData->bBuildReversedIndexBuffer = false;
			}
		}

		// Perform import
		bool bCancelled = false;
		UObject* ImportedObject = FbxFactory->ImportObject(
			UStaticMesh::StaticClass(),
			TransientPackage,
			FName(*FileName),
			RF_Transient | RF_Public,
			FilePath,
			nullptr,
			bCancelled
		);

		FbxFactory->RemoveFromRoot();

		ResultMesh = Cast<UStaticMesh>(ImportedObject);
	}
	// Note: GLTF/GLB support may require InterchangeFramework module - not all UE versions have built-in GLTF factory

	if (ResultMesh)
	{
		// Mark as transient
		ResultMesh->SetFlags(RF_Transient);

		// Build mesh for rendering (required for thumbnail)
		ResultMesh->Build(false);

		UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully imported mesh: %s"), *FilePath);
	}

	return ResultMesh;
}

TSharedPtr<FSlateBrush> FFastAssetsThumbnail::RenderMeshThumbnail(UStaticMesh* Mesh, const FString& FilePath)
{
	if (!Mesh)
	{
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Rendering thumbnail for mesh"));

	// Create thumbnail data
	FObjectThumbnail NewThumbnail;

	// Use ThumbnailTools to render the thumbnail
	ThumbnailTools::RenderThumbnail(
		Mesh,
		MeshThumbnailSize,
		MeshThumbnailSize,
		ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
		nullptr,
		&NewThumbnail
	);

	if (NewThumbnail.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Rendered thumbnail is empty"));
		return nullptr;
	}

	// Convert thumbnail to texture
	UTexture2D* ThumbnailTexture = ConvertThumbnailToTexture(NewThumbnail);
	if (!ThumbnailTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to convert thumbnail to texture"));
		return nullptr;
	}

	// Store texture reference
	TextureCache.Add(FilePath, ThumbnailTexture);

	// Create brush from texture
	TSharedPtr<FSlateBrush> NewBrush = MakeShared<FSlateBrush>();
	NewBrush->SetResourceObject(ThumbnailTexture);
	NewBrush->ImageSize = FVector2D(MeshThumbnailSize, MeshThumbnailSize);
	NewBrush->DrawAs = ESlateBrushDrawType::Image;
	NewBrush->Tiling = ESlateBrushTileType::NoTile;

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully created mesh thumbnail brush"));

	return NewBrush;
}

UTexture2D* FFastAssetsThumbnail::ConvertThumbnailToTexture(const FObjectThumbnail& Thumbnail)
{
	if (Thumbnail.IsEmpty())
	{
		return nullptr;
	}

	int32 Width = Thumbnail.GetImageWidth();
	int32 Height = Thumbnail.GetImageHeight();

	if (Width <= 0 || Height <= 0)
	{
		return nullptr;
	}

	// Get uncompressed image data
	const TArray<uint8>& UncompressedData = Thumbnail.GetUncompressedImageData();

	if (UncompressedData.Num() == 0)
	{
		return nullptr;
	}

	// Create texture
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture)
	{
		return nullptr;
	}

	// Prevent garbage collection
	Texture->AddToRoot();

	// Copy pixel data
	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, UncompressedData.GetData(), UncompressedData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update resource
	Texture->UpdateResource();

	return Texture;
}

void FFastAssetsThumbnail::CleanupTransientMesh(UStaticMesh* Mesh)
{
	if (!Mesh)
	{
		return;
	}

	UE_LOG(LogTemp, Verbose, TEXT("FastAssets: Cleaning up transient mesh"));

	// Mark for garbage collection
	Mesh->ClearFlags(RF_Public | RF_Standalone);
	Mesh->SetFlags(RF_Transient);
	Mesh->MarkAsGarbage();

	// Track in pending cleanup list
	PendingCleanupMeshes.Add(Mesh);

	// Periodically clean up the pending list
	for (int32 i = PendingCleanupMeshes.Num() - 1; i >= 0; --i)
	{
		if (!PendingCleanupMeshes[i].IsValid())
		{
			PendingCleanupMeshes.RemoveAt(i);
		}
	}
}

void FFastAssetsThumbnail::ClearCache()
{
	ThumbnailCache.Empty();

	// Remove textures from root before clearing
	for (auto& Pair : TextureCache)
	{
		if (Pair.Value && Pair.Value->IsValidLowLevel())
		{
			Pair.Value->RemoveFromRoot();
		}
	}
	TextureCache.Empty();

	// Clear pending cleanup meshes
	PendingCleanupMeshes.Empty();
}

void FFastAssetsThumbnail::PreCacheThumbnails(const TArray<FString>& FilePaths, const TArray<FString>& AssetTypes)
{
	// Pre-cache thumbnails for faster display
	for (int32 i = 0; i < FilePaths.Num() && i < AssetTypes.Num(); i++)
	{
		if (SupportsImageThumbnail(AssetTypes[i]) || Supports3DModelThumbnail(AssetTypes[i]))
		{
			GetThumbnailBrush(FilePaths[i], AssetTypes[i]);
		}
	}
}
