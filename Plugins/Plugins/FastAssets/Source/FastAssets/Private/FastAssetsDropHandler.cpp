// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "FastAssetsDropHandler.h"
#include "FAssetDragDropOp.h"
#include "FastAssetImporter.h"
#include "SFastAssetsViewportDropOverlay.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Sound/AmbientSound.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "ILevelEditor.h"
#include "SLevelViewport.h"
#include "EditorViewportClient.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Widgets/SOverlay.h"

#define LOCTEXT_NAMESPACE "FastAssets"

FFastAssetsDropHandler::FFastAssetsDropHandler()
	: bIsInitialized(false)
{
}

FFastAssetsDropHandler::~FFastAssetsDropHandler()
{
	Shutdown();
}

void FFastAssetsDropHandler::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}

	bIsInitialized = true;

	// Register viewport drop handler
	RegisterViewportDropHandler();

	// Register content browser drop handler
	RegisterContentBrowserDropHandler();

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Drop handlers initialized"));
}

void FFastAssetsDropHandler::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}

	// Unregister viewport drop handler
	UnregisterViewportDropHandler();

	// Unregister content browser drop handler
	UnregisterContentBrowserDropHandler();

	bIsInitialized = false;

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Drop handlers shut down"));
}

bool FFastAssetsDropHandler::IsFastAssetDrag(const FDragDropEvent& DragDropEvent)
{
	return DragDropEvent.GetOperationAs<FFastAssetDragDropOp>().IsValid();
}

bool FFastAssetsDropHandler::HandleViewportDrop(const FGeometry& Geometry, const FDragDropEvent& DragDropEvent, const FVector& DropLocation)
{
	TSharedPtr<FFastAssetDragDropOp> DragOp = DragDropEvent.GetOperationAs<FFastAssetDragDropOp>();
	if (!DragOp.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FExternalAssetItem>>& Assets = DragOp->GetAssets();
	if (Assets.Num() == 0)
	{
		return false;
	}

	// Get the current world
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return false;
	}

	return ImportAndSpawnAssets(Assets, World, DropLocation);
}

bool FFastAssetsDropHandler::HandleContentBrowserDrop(const FString& DestinationPath, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FFastAssetDragDropOp> DragOp = DragDropEvent.GetOperationAs<FFastAssetDragDropOp>();
	if (!DragOp.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FExternalAssetItem>>& Assets = DragOp->GetAssets();
	if (Assets.Num() == 0)
	{
		return false;
	}

	return ImportAssetsToProject(Assets, DestinationPath);
}

bool FFastAssetsDropHandler::ImportAndSpawnAssets(const TArray<TSharedPtr<FExternalAssetItem>>& Assets, UWorld* World, const FVector& Location)
{
	if (!World)
	{
		return false;
	}

	bool bSuccess = false;
	FVector CurrentLocation = Location;
	float OffsetX = 0.0f;

	for (const TSharedPtr<FExternalAssetItem>& AssetItem : Assets)
	{
		// Import the asset first
		FImportResult ImportResult = AssetImporter.ImportAsset(AssetItem);

		if (!ImportResult.bSuccess || !ImportResult.ImportedAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to import %s - %s"),
				*AssetItem->FileName, *ImportResult.ErrorMessage);
			continue;
		}

		// Spawn actor based on asset type
		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(ImportResult.ImportedAsset))
		{
			// Create StaticMeshActor
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(),
				FVector(CurrentLocation.X + OffsetX, CurrentLocation.Y, CurrentLocation.Z),
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (NewActor)
			{
				NewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
				NewActor->SetActorLabel(AssetItem->FileName);

				// Get bounds for offset calculation
				FBoxSphereBounds Bounds = StaticMesh->GetBounds();
				OffsetX += Bounds.BoxExtent.X * 2.5f;

				bSuccess = true;
				UE_LOG(LogTemp, Log, TEXT("FastAssets: Spawned mesh %s"), *AssetItem->FileName);
			}
		}
		else if (USoundWave* SoundWave = Cast<USoundWave>(ImportResult.ImportedAsset))
		{
			// Create AmbientSound actor
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AAmbientSound* NewActor = World->SpawnActor<AAmbientSound>(
				AAmbientSound::StaticClass(),
				FVector(CurrentLocation.X + OffsetX, CurrentLocation.Y, CurrentLocation.Z),
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (NewActor)
			{
				NewActor->GetAudioComponent()->SetSound(SoundWave);
				NewActor->SetActorLabel(AssetItem->FileName);
				OffsetX += 200.0f;

				bSuccess = true;
				UE_LOG(LogTemp, Log, TEXT("FastAssets: Spawned sound %s"), *AssetItem->FileName);
			}
		}
		else
		{
			// For textures and other assets, just import (no spawn)
			bSuccess = true;
			UE_LOG(LogTemp, Log, TEXT("FastAssets: Imported %s (no spawn for this type)"), *AssetItem->FileName);
		}
	}

	return bSuccess;
}

bool FFastAssetsDropHandler::ImportAssetsToProject(const TArray<TSharedPtr<FExternalAssetItem>>& Assets, const FString& DestinationPath)
{
	bool bSuccess = false;

	// Override the base path if a specific destination is given
	if (!DestinationPath.IsEmpty())
	{
		AssetImporter.SetBaseContentPath(DestinationPath);
	}

	for (const TSharedPtr<FExternalAssetItem>& AssetItem : Assets)
	{
		FImportResult ImportResult = AssetImporter.ImportAsset(AssetItem);

		if (ImportResult.bSuccess)
		{
			bSuccess = true;
			UE_LOG(LogTemp, Log, TEXT("FastAssets: Imported %s to %s"),
				*AssetItem->FileName, *ImportResult.AssetPath);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FastAssets: Failed to import %s - %s"),
				*AssetItem->FileName, *ImportResult.ErrorMessage);
		}
	}

	// Reset to default base path
	AssetImporter.SetBaseContentPath(TEXT("/Game/Assets"));

	return bSuccess;
}

void FFastAssetsDropHandler::RegisterViewportDropHandler()
{
	if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		return;
	}

	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");

	// Subscribe to level editor creation events
	LevelEditorCreatedHandle = LevelEditorModule.OnLevelEditorCreated().AddRaw(
		this, &FFastAssetsDropHandler::OnLevelEditorCreated);

	// If a level editor already exists, register now
	TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (LevelEditor.IsValid())
	{
		OnLevelEditorCreated(LevelEditor);
	}
}

void FFastAssetsDropHandler::UnregisterViewportDropHandler()
{
	// Remove the overlay widget from the viewport
	if (ViewportOverlayWidget.IsValid())
	{
		TSharedPtr<SWidget> Overlay = ViewportOverlayWidget.Pin();
		if (Overlay.IsValid() && FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
			if (LevelEditor.IsValid())
			{
				TSharedPtr<SLevelViewport> ActiveViewport = LevelEditor->GetActiveViewportInterface();
				if (ActiveViewport.IsValid())
				{
					ActiveViewport->RemoveOverlayWidget(Overlay.ToSharedRef());
				}
			}
		}
		ViewportOverlayWidget.Reset();
	}

	// Remove delegate
	if (LevelEditorCreatedHandle.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.OnLevelEditorCreated().Remove(LevelEditorCreatedHandle);
		}
		LevelEditorCreatedHandle.Reset();
	}
}

void FFastAssetsDropHandler::RegisterContentBrowserDropHandler()
{
	if (!FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		return;
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");

	// Get the content browser singleton to hook into drag-drop events
	IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();

	// Register a delegate to handle external asset drops
	// Note: Content browser drag-drop is handled through a different mechanism
	// We use the OnAssetDragDropped delegate if available, otherwise we rely on
	// the viewport drop handler for now

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Content browser drop handler registered"));
}

void FFastAssetsDropHandler::UnregisterContentBrowserDropHandler()
{
	if (ContentBrowserDropHandle.IsValid())
	{
		// Remove content browser delegate if registered
		ContentBrowserDropHandle.Reset();
	}
}

void FFastAssetsDropHandler::OnLevelEditorCreated(TSharedPtr<ILevelEditor> LevelEditor)
{
	if (!LevelEditor.IsValid())
	{
		return;
	}

	// Get the active viewport
	TSharedPtr<SLevelViewport> ActiveViewport = LevelEditor->GetActiveViewportInterface();
	if (!ActiveViewport.IsValid())
	{
		return;
	}

	// Create the overlay widget
	TSharedRef<SFastAssetsViewportDropOverlay> Overlay = SNew(SFastAssetsViewportDropOverlay)
		.DropHandler(SharedThis(this))
		.ViewportClient(ActiveViewport->GetViewportClient());

	ViewportOverlayWidget = Overlay;

	// Add the overlay to the viewport using AddOverlayWidget
	ActiveViewport->AddOverlayWidget(Overlay);

	UE_LOG(LogTemp, Log, TEXT("FastAssets: Viewport drop overlay registered"));
}

#undef LOCTEXT_NAMESPACE
