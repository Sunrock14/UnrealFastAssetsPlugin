// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SFastAssetsWindow.h"

struct FImportResult
{
	bool bSuccess;
	FString AssetPath;
	UObject* ImportedAsset;
	FString ErrorMessage;

	FImportResult()
		: bSuccess(false)
		, ImportedAsset(nullptr)
	{
	}
};

class FFastAssetImporter
{
public:
	FFastAssetImporter();
	~FFastAssetImporter();

	/** Import a single external asset into the project */
	FImportResult ImportAsset(const TSharedPtr<FExternalAssetItem>& AssetItem);

	/** Import multiple assets */
	TArray<FImportResult> ImportAssets(const TArray<TSharedPtr<FExternalAssetItem>>& AssetItems);

	/** Get the destination path for an asset based on its type */
	FString GetDestinationPath(const FString& AssetType) const;

	/** Set the base content path for imports */
	void SetBaseContentPath(const FString& InPath);

	/** Get the base content path */
	FString GetBaseContentPath() const { return BaseContentPath; }

	/** Check if a file type can be imported */
	bool CanImportFileType(const FString& Extension) const;

private:
	/** Create destination directory if it doesn't exist */
	bool EnsureDirectoryExists(const FString& Path);

	/** Generate unique asset name to avoid conflicts */
	FString GenerateUniqueAssetName(const FString& DestPath, const FString& AssetName);

	/** Import 3D model files (FBX, OBJ, etc.) */
	FImportResult Import3DModel(const FString& SourcePath, const FString& DestPath, const FString& AssetName);

	/** Import audio files (WAV, MP3, etc.) */
	FImportResult ImportAudio(const FString& SourcePath, const FString& DestPath, const FString& AssetName);

	/** Import texture files (PNG, JPG, etc.) */
	FImportResult ImportTexture(const FString& SourcePath, const FString& DestPath, const FString& AssetName);

	/** Import generic file using asset tools */
	FImportResult ImportGeneric(const FString& SourcePath, const FString& DestPath, const FString& AssetName);

private:
	/** Base path for imported assets (default: /Game/Assets/) */
	FString BaseContentPath;

	/** Mapping from asset type to subfolder */
	TMap<FString, FString> AssetTypeToFolder;
};
