// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "SFastAssetsViewportDropOverlay.h"
#include "FastAssetsDropHandler.h"
#include "FAssetDragDropOp.h"
#include "EditorViewportClient.h"
#include "Engine/World.h"
#include "Editor.h"
#include "UnrealClient.h"
#include "SceneView.h"

#define LOCTEXT_NAMESPACE "FastAssets"

void SFastAssetsViewportDropOverlay::Construct(const FArguments& InArgs)
{
	DropHandler = InArgs._DropHandler;
	ViewportClient = InArgs._ViewportClient;
	bIsDragOver = false;

	// Create a fully transparent widget that fills the entire viewport
	ChildSlot
	[
		SNullWidget::NullWidget
	];
}

void SFastAssetsViewportDropOverlay::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (FFastAssetsDropHandler::IsFastAssetDrag(DragDropEvent))
	{
		bIsDragOver = true;
	}
}

void SFastAssetsViewportDropOverlay::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	bIsDragOver = false;
}

FReply SFastAssetsViewportDropOverlay::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (FFastAssetsDropHandler::IsFastAssetDrag(DragDropEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFastAssetsViewportDropOverlay::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	bIsDragOver = false;

	if (!FFastAssetsDropHandler::IsFastAssetDrag(DragDropEvent))
	{
		return FReply::Unhandled();
	}

	if (!DropHandler.IsValid())
	{
		return FReply::Unhandled();
	}

	// Get screen position from the drag event
	FVector2D ScreenPosition = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());

	// Convert to world location via ray cast
	FVector WorldLocation;
	if (!GetWorldLocationFromScreenPosition(ScreenPosition, WorldLocation))
	{
		// Use a default location if ray cast fails
		WorldLocation = FVector::ZeroVector;
		UE_LOG(LogTemp, Warning, TEXT("FastAssets: Could not determine drop location, using origin"));
	}

	// Delegate to the drop handler
	if (DropHandler->HandleViewportDrop(MyGeometry, DragDropEvent, WorldLocation))
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

bool SFastAssetsViewportDropOverlay::GetWorldLocationFromScreenPosition(const FVector2D& ScreenPosition, FVector& OutWorldLocation) const
{
	TSharedPtr<FEditorViewportClient> ViewportClientPtr = ViewportClient.Pin();
	if (!ViewportClientPtr.IsValid())
	{
		return false;
	}

	UWorld* World = ViewportClientPtr->GetWorld();
	if (!World)
	{
		return false;
	}

	FViewport* Viewport = ViewportClientPtr->Viewport;
	if (!Viewport)
	{
		return false;
	}

	// Get viewport dimensions
	FIntPoint ViewportSize = Viewport->GetSizeXY();
	if (ViewportSize.X == 0 || ViewportSize.Y == 0)
	{
		return false;
	}

	// Use the viewport client's built-in screen to world conversion
	FVector RayOrigin, RayDirection;

	// Get view location and rotation from the viewport client
	FVector ViewLocation = ViewportClientPtr->GetViewLocation();
	FRotator ViewRotation = ViewportClientPtr->GetViewRotation();

	// Calculate aspect ratio and FOV
	float AspectRatio = (float)ViewportSize.X / (float)ViewportSize.Y;
	float FOV = ViewportClientPtr->ViewFOV;

	// Convert screen position to normalized device coordinates (-1 to 1)
	float NormalizedX = (ScreenPosition.X / ViewportSize.X) * 2.0f - 1.0f;
	float NormalizedY = 1.0f - (ScreenPosition.Y / ViewportSize.Y) * 2.0f;

	// Calculate the ray direction in view space
	float TanHalfFOV = FMath::Tan(FMath::DegreesToRadians(FOV * 0.5f));
	FVector RayDirViewSpace(
		NormalizedX * TanHalfFOV * AspectRatio,
		NormalizedY * TanHalfFOV,
		1.0f
	);
	RayDirViewSpace.Normalize();

	// Transform ray direction from view space to world space
	FMatrix ViewRotationMatrix = FRotationMatrix(ViewRotation);
	RayDirection = ViewRotationMatrix.TransformVector(RayDirViewSpace);
	RayOrigin = ViewLocation;

	// Perform line trace against world geometry
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	const float TraceDistance = 1000000.0f; // 10km max trace distance
	FVector TraceEnd = RayOrigin + RayDirection * TraceDistance;

	if (World->LineTraceSingleByChannel(HitResult, RayOrigin, TraceEnd, ECC_Visibility, QueryParams))
	{
		OutWorldLocation = HitResult.ImpactPoint;
		return true;
	}

	// If no hit, place at a default distance along the ray
	const float DefaultDistance = 1000.0f;
	OutWorldLocation = RayOrigin + RayDirection * DefaultDistance;
	return true;
}

#undef LOCTEXT_NAMESPACE
