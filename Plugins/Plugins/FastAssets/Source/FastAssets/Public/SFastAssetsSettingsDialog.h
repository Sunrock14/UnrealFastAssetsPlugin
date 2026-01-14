// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UFastAssetsSettings;

/**
 * Settings dialog for Fast Assets plugin
 */
class SFastAssetsSettingsDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFastAssetsSettingsDialog) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Show the settings dialog as a modal window */
	static void ShowDialog();

private:
	// UI Construction
	TSharedRef<SWidget> ConstructGeneralSection();
	TSharedRef<SWidget> ConstructImportSection();
	TSharedRef<SWidget> ConstructOrganizationSection();
	TSharedRef<SWidget> ConstructScanningSection();
	TSharedRef<SWidget> ConstructButtonsSection();

	// Button handlers
	FReply OnResetClicked();
	FReply OnApplyClicked();
	FReply OnCancelClicked();
	FReply OnOKClicked();

	// Settings helpers
	void ApplySettings();
	void LoadSettings();

	// Combo box helpers
	TSharedRef<SWidget> GenerateViewModeComboItem(TSharedPtr<FString> Item);
	void OnViewModeChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	FText GetCurrentViewModeText() const;

	TSharedRef<SWidget> GenerateThumbnailSizeComboItem(TSharedPtr<FString> Item);
	void OnThumbnailSizeChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	FText GetCurrentThumbnailSizeText() const;

	TSharedRef<SWidget> GenerateDuplicateHandlingComboItem(TSharedPtr<FString> Item);
	void OnDuplicateHandlingChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	FText GetCurrentDuplicateHandlingText() const;

private:
	// Cached settings reference
	UFastAssetsSettings* Settings;

	// Temporary values (applied on OK/Apply)
	int32 TempViewMode;
	int32 TempThumbnailSize;
	int32 TempDuplicateHandling;
	bool bTempRememberLastPath;
	bool bTempAutoImportOnDrop;
	bool bTempShowImportDialog;
	bool bTempAutoCreateFolders;
	bool bTempRecursiveScan;
	FString TempBaseImportPath;
	int32 TempMaxScanDepth;

	// Combo box options
	TArray<TSharedPtr<FString>> ViewModeOptions;
	TArray<TSharedPtr<FString>> ThumbnailSizeOptions;
	TArray<TSharedPtr<FString>> DuplicateHandlingOptions;

	// Parent window reference for closing
	TWeakPtr<SWindow> ParentWindow;
};
