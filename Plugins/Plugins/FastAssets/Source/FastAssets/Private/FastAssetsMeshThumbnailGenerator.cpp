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
#include "Factories/FbxImportUI.h"
#include "Factories/FbxStaticMeshImportData.h"

TUniquePtr<FFastAssetsMeshThumbnailGenerator> FFastAssetsMeshThumbnailGenerator::Instance = nullptr;

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

	// Create unique name to avoid collisions
	FString UniqueSuffix = FGuid::NewGuid().ToString(EGuidFormats::Short);
	FString UniqueAssetName = AssetName + TEXT("_") + UniqueSuffix;

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Importing mesh to transient package: %s"), *UniqueAssetName);

	// Check if this is an FBX file
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	bool bIsFbx = (Extension == TEXT("fbx"));

	// Create a transient package - this won't be saved to disk
	FString TransientPackageName = FString::Printf(TEXT("/Temp/FastAssets/%s"), *UniqueAssetName);
	UPackage* TempPackage = CreatePackage(*TransientPackageName);
	if (!TempPackage)
	{
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to create transient package"));
		return nullptr;
	}
	TempPackage->SetFlags(RF_Transient);

	// Get asset tools
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Configure FBX import options
	UFbxImportUI* FbxImportUI = nullptr;
	if (bIsFbx)
	{
		FbxImportUI = NewObject<UFbxImportUI>();
		FbxImportUI->bImportMesh = true;
		FbxImportUI->bImportTextures = false;
		FbxImportUI->bImportMaterials = false;
		FbxImportUI->bImportAnimations = false;
		FbxImportUI->bIsObjImport = false;
		FbxImportUI->bAutomatedImportShouldDetectType = false;
		FbxImportUI->MeshTypeToImport = FBXIT_StaticMesh;

		// Configure static mesh import settings
		if (FbxImportUI->StaticMeshImportData)
		{
			// Auto-generate normals - this suppresses the smoothing group warning
			FbxImportUI->StaticMeshImportData->NormalImportMethod = FBXNIM_ComputeNormals;
			FbxImportUI->StaticMeshImportData->NormalGenerationMethod = EFBXNormalGenerationMethod::MikkTSpace;
			FbxImportUI->StaticMeshImportData->bCombineMeshes = true;
			FbxImportUI->StaticMeshImportData->bAutoGenerateCollision = false;
			FbxImportUI->StaticMeshImportData->bRemoveDegenerates = true;
			FbxImportUI->StaticMeshImportData->bBuildReversedIndexBuffer = false;
			FbxImportUI->StaticMeshImportData->bGenerateLightmapUVs = false;
		}
	}

	// Suppress dialogs during import
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	// Create import task for better control
	UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
	ImportTask->Filename = FilePath;
	ImportTask->DestinationPath = FPaths::GetPath(TransientPackageName);
	ImportTask->DestinationName = UniqueAssetName;
	ImportTask->bReplaceExisting = true;
	ImportTask->bAutomated = true;
	ImportTask->bSave = false;

	if (FbxImportUI)
	{
		ImportTask->Options = FbxImportUI;
	}

	// Import the asset
	TArray<UAssetImportTask*> ImportTasks;
	ImportTasks.Add(ImportTask);
	AssetTools.ImportAssetTasks(ImportTasks);

	// Find the static mesh in imported assets
	for (UObject* Asset : ImportTask->GetObjects())
	{
		if (UStaticMesh* Mesh = Cast<UStaticMesh>(Asset))
		{
			// Mark as transient so it won't be saved
			Mesh->SetFlags(RF_Transient);
			Mesh->GetPackage()->SetFlags(RF_Transient);
			UE_LOG(LogTemp, Log, TEXT("FastAssets: Successfully imported transient mesh: %s"), *Mesh->GetName());
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

	FString AssetName = Asset->GetName();
	UE_LOG(LogTemp, Log, TEXT("FastAssets: Cleaning up transient asset: %s"), *AssetName);

	// For transient objects, we just need to remove references and let GC handle it
	// The RF_Transient flag ensures it won't be saved to disk

	// Clear any references
	Asset->ClearFlags(RF_Standalone);
	Asset->MarkAsGarbage();

	// Also mark the package for cleanup
	if (UPackage* Package = Asset->GetPackage())
	{
		Package->ClearFlags(RF_Standalone);
		Package->MarkAsGarbage();
	}

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Transient asset marked for garbage collection"));
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
