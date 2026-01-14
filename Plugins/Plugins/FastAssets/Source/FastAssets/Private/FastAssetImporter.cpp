// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FastAssetImporter.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundWave.h"

#define LOCTEXT_NAMESPACE "FastAssets"

FFastAssetImporter::FFastAssetImporter()
{
	// Default base path
	BaseContentPath = TEXT("/Game/Assets");

	// Setup asset type to folder mapping
	AssetTypeToFolder.Add(TEXT("Mesh"), TEXT("Meshes"));
	AssetTypeToFolder.Add(TEXT("Sound"), TEXT("Sounds"));
	AssetTypeToFolder.Add(TEXT("Texture"), TEXT("Textures"));
	AssetTypeToFolder.Add(TEXT("Material"), TEXT("Materials"));
	AssetTypeToFolder.Add(TEXT("Animation"), TEXT("Animations"));
	AssetTypeToFolder.Add(TEXT("Blueprint"), TEXT("Blueprints"));
	AssetTypeToFolder.Add(TEXT("Data"), TEXT("Data"));
	AssetTypeToFolder.Add(TEXT("UAsset"), TEXT("Misc"));
	AssetTypeToFolder.Add(TEXT("Map"), TEXT("Maps"));
	AssetTypeToFolder.Add(TEXT("Other"), TEXT("Misc"));
}

FFastAssetImporter::~FFastAssetImporter()
{
}

void FFastAssetImporter::SetBaseContentPath(const FString& InPath)
{
	BaseContentPath = InPath;
}

FString FFastAssetImporter::GetDestinationPath(const FString& AssetType) const
{
	const FString* SubFolder = AssetTypeToFolder.Find(AssetType);
	if (SubFolder)
	{
		return BaseContentPath / *SubFolder;
	}
	return BaseContentPath / TEXT("Misc");
}

bool FFastAssetImporter::CanImportFileType(const FString& Extension) const
{
	// Supported extensions
	static TArray<FString> SupportedExtensions = {
		// 3D Models
		TEXT("fbx"), TEXT("obj"), TEXT("gltf"), TEXT("glb"),
		// Audio
		TEXT("wav"), TEXT("mp3"), TEXT("ogg"), TEXT("flac"),
		// Textures
		TEXT("png"), TEXT("jpg"), TEXT("jpeg"), TEXT("tga"), TEXT("bmp"), TEXT("exr"), TEXT("hdr"), TEXT("psd"),
		// Data
		TEXT("json"), TEXT("csv")
	};

	return SupportedExtensions.Contains(Extension.ToLower());
}

FImportResult FFastAssetImporter::ImportAsset(const TSharedPtr<FExternalAssetItem>& AssetItem)
{
	FImportResult Result;

	if (!AssetItem.IsValid())
	{
		Result.ErrorMessage = TEXT("Invalid asset item");
		return Result;
	}

	// Get destination path based on asset type
	FString DestPath = GetDestinationPath(AssetItem->AssetType);

	// Ensure directory exists
	if (!EnsureDirectoryExists(DestPath))
	{
		Result.ErrorMessage = FString::Printf(TEXT("Failed to create directory: %s"), *DestPath);
		return Result;
	}

	// Generate unique name
	FString UniqueAssetName = GenerateUniqueAssetName(DestPath, AssetItem->FileName);

	// Import based on asset type
	if (AssetItem->AssetType == TEXT("Mesh"))
	{
		Result = Import3DModel(AssetItem->FilePath, DestPath, UniqueAssetName);
	}
	else if (AssetItem->AssetType == TEXT("Sound"))
	{
		Result = ImportAudio(AssetItem->FilePath, DestPath, UniqueAssetName);
	}
	else if (AssetItem->AssetType == TEXT("Texture"))
	{
		Result = ImportTexture(AssetItem->FilePath, DestPath, UniqueAssetName);
	}
	else
	{
		Result = ImportGeneric(AssetItem->FilePath, DestPath, UniqueAssetName);
	}

	return Result;
}

TArray<FImportResult> FFastAssetImporter::ImportAssets(const TArray<TSharedPtr<FExternalAssetItem>>& AssetItems)
{
	TArray<FImportResult> Results;

	for (const TSharedPtr<FExternalAssetItem>& Item : AssetItems)
	{
		Results.Add(ImportAsset(Item));
	}

	return Results;
}

bool FFastAssetImporter::EnsureDirectoryExists(const FString& Path)
{
	// Convert game path to filesystem path
	FString FilesystemPath;
	if (FPackageName::TryConvertLongPackageNameToFilename(Path, FilesystemPath))
	{
		// Create directory if it doesn't exist
		if (!IFileManager::Get().DirectoryExists(*FilesystemPath))
		{
			return IFileManager::Get().MakeDirectory(*FilesystemPath, true);
		}
		return true;
	}

	return false;
}

FString FFastAssetImporter::GenerateUniqueAssetName(const FString& DestPath, const FString& AssetName)
{
	FString CleanName = AssetName;
	// Remove invalid characters
	CleanName = FPaths::MakeValidFileName(CleanName);

	// Check if asset already exists
	FString FullPath = DestPath / CleanName;

	if (!UEditorAssetLibrary::DoesAssetExist(FullPath))
	{
		return CleanName;
	}

	// Generate unique name with suffix
	int32 Suffix = 1;
	FString UniqueName;
	do
	{
		UniqueName = FString::Printf(TEXT("%s_%d"), *CleanName, Suffix);
		FullPath = DestPath / UniqueName;
		Suffix++;
	} while (UEditorAssetLibrary::DoesAssetExist(FullPath) && Suffix < 1000);

	return UniqueName;
}

FImportResult FFastAssetImporter::Import3DModel(const FString& SourcePath, const FString& DestPath, const FString& AssetName)
{
	FImportResult Result;

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Import the file
	TArray<FString> FilesToImport;
	FilesToImport.Add(SourcePath);

	TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(FilesToImport, DestPath);

	if (ImportedAssets.Num() > 0 && ImportedAssets[0] != nullptr)
	{
		Result.bSuccess = true;
		Result.ImportedAsset = ImportedAssets[0];
		Result.AssetPath = DestPath / AssetName;
	}
	else
	{
		Result.ErrorMessage = TEXT("Failed to import 3D model");
	}

	return Result;
}

FImportResult FFastAssetImporter::ImportAudio(const FString& SourcePath, const FString& DestPath, const FString& AssetName)
{
	FImportResult Result;

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TArray<FString> FilesToImport;
	FilesToImport.Add(SourcePath);

	TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(FilesToImport, DestPath);

	if (ImportedAssets.Num() > 0 && ImportedAssets[0] != nullptr)
	{
		Result.bSuccess = true;
		Result.ImportedAsset = ImportedAssets[0];
		Result.AssetPath = DestPath / AssetName;
	}
	else
	{
		Result.ErrorMessage = TEXT("Failed to import audio file");
	}

	return Result;
}

FImportResult FFastAssetImporter::ImportTexture(const FString& SourcePath, const FString& DestPath, const FString& AssetName)
{
	FImportResult Result;

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TArray<FString> FilesToImport;
	FilesToImport.Add(SourcePath);

	TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(FilesToImport, DestPath);

	if (ImportedAssets.Num() > 0 && ImportedAssets[0] != nullptr)
	{
		Result.bSuccess = true;
		Result.ImportedAsset = ImportedAssets[0];
		Result.AssetPath = DestPath / AssetName;
	}
	else
	{
		Result.ErrorMessage = TEXT("Failed to import texture");
	}

	return Result;
}

FImportResult FFastAssetImporter::ImportGeneric(const FString& SourcePath, const FString& DestPath, const FString& AssetName)
{
	FImportResult Result;

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TArray<FString> FilesToImport;
	FilesToImport.Add(SourcePath);

	TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(FilesToImport, DestPath);

	if (ImportedAssets.Num() > 0 && ImportedAssets[0] != nullptr)
	{
		Result.bSuccess = true;
		Result.ImportedAsset = ImportedAssets[0];
		Result.AssetPath = DestPath / AssetName;
	}
	else
	{
		Result.ErrorMessage = TEXT("Failed to import asset");
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
