// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FastAssetsMeshThumbnailGenerator.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetImportTask.h"
#include "ObjectTools.h"
#include "Misc/ObjectThumbnail.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "HAL/FileManager.h"
#include "Containers/Ticker.h"

TUniquePtr<FFastAssetsMeshThumbnailGenerator> FFastAssetsMeshThumbnailGenerator::Instance = nullptr;
const FString FFastAssetsMeshThumbnailGenerator::TempFolderPath = TEXT("/Game/_FastAssetsTemp");

FFastAssetsMeshThumbnailGenerator::FFastAssetsMeshThumbnailGenerator()
	: bIsProcessing(false)
{
}

FFastAssetsMeshThumbnailGenerator::~FFastAssetsMeshThumbnailGenerator()
{
	CancelPendingRequests();
	ClearCache();
}

FFastAssetsMeshThumbnailGenerator& FFastAssetsMeshThumbnailGenerator::Get()
{
	if (!Instance.IsValid())
	{
		Instance = MakeUnique<FFastAssetsMeshThumbnailGenerator>();
	}
	return *Instance;
}

bool FFastAssetsMeshThumbnailGenerator::SupportsMeshThumbnail(const FString& Extension) const
{
	FString LowerExt = Extension.ToLower();
	return LowerExt == TEXT("fbx") ||
		   LowerExt == TEXT("obj") ||
		   LowerExt == TEXT("gltf") ||
		   LowerExt == TEXT("glb");
}

bool FFastAssetsMeshThumbnailGenerator::IsPendingRequest(const FString& FilePath) const
{
	return PendingRequests.Contains(FilePath);
}

void FFastAssetsMeshThumbnailGenerator::RequestThumbnail(const FString& FilePath, TFunction<void(UTexture2D*)> OnComplete)
{
	// Check cache first
	if (TWeakObjectPtr<UTexture2D>* CachedTexture = ThumbnailCache.Find(FilePath))
	{
		if (CachedTexture->IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("FastAssets: Returning cached mesh thumbnail for: %s"), *FilePath);
			if (OnComplete)
			{
				OnComplete(CachedTexture->Get());
			}
			return;
		}
	}

	// Check if already pending
	if (PendingRequests.Contains(FilePath))
	{
		UE_LOG(LogTemp, Verbose, TEXT("FastAssets: Thumbnail request already pending for: %s"), *FilePath);
		return;
	}

	// Add to pending and queue
	{
		FScopeLock Lock(&RequestLock);
		PendingRequests.Add(FilePath);
		RequestQueue.Add(TPair<FString, TFunction<void(UTexture2D*)>>(FilePath, OnComplete));
	}

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Queued mesh thumbnail request for: %s"), *FilePath);

	// Process on game thread if not already processing
	if (!bIsProcessing)
	{
		bIsProcessing = true;
		if (!TickerHandle.IsValid())
		{
			TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateRaw(this, &FFastAssetsMeshThumbnailGenerator::Tick));
		}
	}
}

bool FFastAssetsMeshThumbnailGenerator::Tick(float DeltaTime)
{
	const bool bProcessedRequest = ProcessNextRequest();
	if (bProcessedRequest)
	{
		return true;
	}

	bIsProcessing = false;
	TickerHandle.Reset();
	return false;
}

bool FFastAssetsMeshThumbnailGenerator::ProcessNextRequest()
{
	FString FilePath;
	TFunction<void(UTexture2D*)> Callback;

	// Get next request from queue
	{
		FScopeLock Lock(&RequestLock);
		if (RequestQueue.Num() == 0)
		{
			return false;
		}

		TPair<FString, TFunction<void(UTexture2D*)>> Request = RequestQueue[0];
		RequestQueue.RemoveAt(0);
		FilePath = Request.Key;
		Callback = Request.Value;
	}

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Processing mesh thumbnail for: %s"), *FilePath);

	// Import mesh to temp package
	UStaticMesh* TempMesh = ImportToTempPackage(FilePath);
	UTexture2D* Thumbnail = nullptr;

	if (TempMesh)
	{
		// Capture thumbnail
		Thumbnail = CaptureThumbnailFromMesh(TempMesh);

		// Delete temp asset
		DeleteTempAsset(TempMesh);
	}

	// Cache the result
	if (Thumbnail)
	{
		ThumbnailCache.Add(FilePath, Thumbnail);
		UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully generated mesh thumbnail for: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to generate mesh thumbnail for: %s"), *FilePath);
	}

	// Remove from pending
	{
		FScopeLock Lock(&RequestLock);
		PendingRequests.Remove(FilePath);
	}

	// Call callback on game thread
	if (Callback)
	{
		Callback(Thumbnail);
	}

	return true;
}

UStaticMesh* FFastAssetsMeshThumbnailGenerator::ImportToTempPackage(const FString& FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: File does not exist: %s"), *FilePath);
		return nullptr;
	}

	FString AssetName = FPaths::GetBaseFilename(FilePath);
	// Sanitize asset name
	AssetName = ObjectTools::SanitizeObjectName(AssetName);

	// Create unique path to avoid collisions
	FString UniqueSuffix = FGuid::NewGuid().ToString(EGuidFormats::Short);
	FString TempPath = TempFolderPath / AssetName + TEXT("_") + UniqueSuffix;

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Importing mesh to temp path: %s"), *TempPath);

	// Get asset tools
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Create import data
	UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();
	ImportData->Filenames.Add(FilePath);
	ImportData->DestinationPath = TempPath;
	ImportData->bReplaceExisting = true;
	ImportData->bSkipReadOnly = true;

	// Suppress dialogs during import
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	// Import the asset
	TArray<UObject*> ImportedAssets = AssetTools.ImportAssetsAutomated(ImportData);

	// Find the static mesh in imported assets
	for (UObject* Asset : ImportedAssets)
	{
		if (UStaticMesh* Mesh = Cast<UStaticMesh>(Asset))
		{
			UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully imported mesh: %s"), *Mesh->GetName());
			return Mesh;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("FastAssets: No static mesh found in imported assets for: %s"), *FilePath);
	return nullptr;
}

UTexture2D* FFastAssetsMeshThumbnailGenerator::CaptureThumbnailFromMesh(UStaticMesh* Mesh)
{
	if (!Mesh)
	{
		return nullptr;
	}

	const int32 ThumbSize = ThumbnailSize;

	FObjectThumbnail ObjectThumbnail;
	ThumbnailTools::RenderThumbnail(
		Mesh,
		ThumbSize,
		ThumbSize,
		ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
		nullptr,
		&ObjectThumbnail);

	if (ObjectThumbnail.IsEmpty() || !ObjectThumbnail.HasValidImageData())
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Thumbnail render failed for mesh: %s"), *Mesh->GetName());
		return nullptr;
	}

	const int32 Width = ObjectThumbnail.GetImageWidth();
	const int32 Height = ObjectThumbnail.GetImageHeight();
	if (Width <= 0 || Height <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Thumbnail has invalid size for mesh: %s"), *Mesh->GetName());
		return nullptr;
	}

	const TArray<uint8>& RawData = ObjectThumbnail.GetUncompressedImageData();
	const int32 ExpectedSize = Width * Height * sizeof(FColor);
	if (RawData.Num() < ExpectedSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Thumbnail data size mismatch for mesh: %s"), *Mesh->GetName());
		return nullptr;
	}

	UTexture2D* ThumbnailTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!ThumbnailTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to create thumbnail texture for mesh: %s"), *Mesh->GetName());
		return nullptr;
	}

	ThumbnailTexture->AddToRoot();

	void* TextureData = ThumbnailTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), ExpectedSize);
	ThumbnailTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	ThumbnailTexture->UpdateResource();

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully captured thumbnail (%dx%d)"), Width, Height);

	return ThumbnailTexture;
}

void FFastAssetsMeshThumbnailGenerator::DeleteTempAsset(UObject* Asset)
{
	if (!Asset)
	{
		return;
	}

	UPackage* Package = Asset->GetOutermost();
	FString PackageName = Package->GetName();

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Deleting temp asset: %s"), *PackageName);

	// Collect objects to delete
	TArray<UObject*> ObjectsToDelete;
	ObjectsToDelete.Add(Asset);

	// Delete the objects
	int32 NumDeleted = ObjectTools::ForceDeleteObjects(ObjectsToDelete, false);

	if (NumDeleted > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully deleted temp asset"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to delete temp asset: %s"), *PackageName);
	}

	// Also try to delete the package file from disk if it exists
	FString PackagePath = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	if (FPaths::FileExists(PackagePath))
	{
		IFileManager::Get().Delete(*PackagePath);
	}
}

void FFastAssetsMeshThumbnailGenerator::ClearCache()
{
	// Remove textures from root
	for (auto& Pair : ThumbnailCache)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->RemoveFromRoot();
		}
	}
	ThumbnailCache.Empty();
}

void FFastAssetsMeshThumbnailGenerator::CancelPendingRequests()
{
	{
		FScopeLock Lock(&RequestLock);
		PendingRequests.Empty();
		RequestQueue.Empty();
	}
	bIsProcessing = false;
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
}
