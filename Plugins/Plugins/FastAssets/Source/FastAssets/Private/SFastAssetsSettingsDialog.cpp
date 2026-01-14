// Copyright Ismail Faruk Kocademir. All Rights Reserved.

#include "SFastAssetsSettingsDialog.h"
#include "FastAssetsSettings.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "FastAssets"

void SFastAssetsSettingsDialog::Construct(const FArguments& InArgs)
{
	Settings = UFastAssetsSettings::Get();

	// Initialize combo box options
	ViewModeOptions.Add(MakeShared<FString>(TEXT("Grid View")));
	ViewModeOptions.Add(MakeShared<FString>(TEXT("List View")));

	ThumbnailSizeOptions.Add(MakeShared<FString>(TEXT("Small (64px)")));
	ThumbnailSizeOptions.Add(MakeShared<FString>(TEXT("Medium (128px)")));
	ThumbnailSizeOptions.Add(MakeShared<FString>(TEXT("Large (256px)")));

	DuplicateHandlingOptions.Add(MakeShared<FString>(TEXT("Ask Every Time")));
	DuplicateHandlingOptions.Add(MakeShared<FString>(TEXT("Skip Duplicates")));
	DuplicateHandlingOptions.Add(MakeShared<FString>(TEXT("Replace Existing")));
	DuplicateHandlingOptions.Add(MakeShared<FString>(TEXT("Auto Rename")));

	// Load current settings
	LoadSettings();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)

			// Scrollable content
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)

				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					SNew(SVerticalBox)

					// General Section
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						ConstructGeneralSection()
					]

					// Import Section
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						ConstructImportSection()
					]

					// Organization Section
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						ConstructOrganizationSection()
					]

					// Scanning Section
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						ConstructScanningSection()
					]
				]
			]

			// Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				ConstructButtonsSection()
			]
		]
	];
}

void SFastAssetsSettingsDialog::ShowDialog()
{
	TSharedRef<SWindow> SettingsWindow = SNew(SWindow)
		.Title(LOCTEXT("SettingsWindowTitle", "Fast Assets Settings"))
		.ClientSize(FVector2D(500, 600))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	TSharedRef<SFastAssetsSettingsDialog> SettingsDialog = SNew(SFastAssetsSettingsDialog);
	SettingsDialog->ParentWindow = SettingsWindow;

	SettingsWindow->SetContent(SettingsDialog);

	FSlateApplication::Get().AddModalWindow(SettingsWindow, FSlateApplication::Get().GetActiveTopLevelWindow());
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::ConstructGeneralSection()
{
	return SNew(SVerticalBox)

		// Section Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GeneralSection", "General"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Default View Mode
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DefaultViewMode", "Default View Mode"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ViewModeOptions)
				.OnGenerateWidget(this, &SFastAssetsSettingsDialog::GenerateViewModeComboItem)
				.OnSelectionChanged(this, &SFastAssetsSettingsDialog::OnViewModeChanged)
				[
					SNew(STextBlock)
					.Text(this, &SFastAssetsSettingsDialog::GetCurrentViewModeText)
				]
			]
		]

		// Thumbnail Size
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ThumbnailSize", "Thumbnail Size"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ThumbnailSizeOptions)
				.OnGenerateWidget(this, &SFastAssetsSettingsDialog::GenerateThumbnailSizeComboItem)
				.OnSelectionChanged(this, &SFastAssetsSettingsDialog::OnThumbnailSizeChanged)
				[
					SNew(STextBlock)
					.Text(this, &SFastAssetsSettingsDialog::GetCurrentThumbnailSizeText)
				]
			]
		]

		// Remember Last Path
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RememberLastPath", "Remember Last Path"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bTempRememberLastPath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bTempRememberLastPath = (NewState == ECheckBoxState::Checked); })
			]
		];
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::ConstructImportSection()
{
	return SNew(SVerticalBox)

		// Section Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ImportSection", "Import"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Auto Import on Drop
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AutoImportOnDrop", "Auto Import on Drop"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bTempAutoImportOnDrop ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bTempAutoImportOnDrop = (NewState == ECheckBoxState::Checked); })
			]
		]

		// Show Import Dialog
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ShowImportDialog", "Show Import Dialog"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bTempShowImportDialog ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bTempShowImportDialog = (NewState == ECheckBoxState::Checked); })
			]
		]

		// Duplicate Handling
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DuplicateHandling", "Duplicate Handling"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&DuplicateHandlingOptions)
				.OnGenerateWidget(this, &SFastAssetsSettingsDialog::GenerateDuplicateHandlingComboItem)
				.OnSelectionChanged(this, &SFastAssetsSettingsDialog::OnDuplicateHandlingChanged)
				[
					SNew(STextBlock)
					.Text(this, &SFastAssetsSettingsDialog::GetCurrentDuplicateHandlingText)
				]
			]
		];
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::ConstructOrganizationSection()
{
	return SNew(SVerticalBox)

		// Section Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OrganizationSection", "Organization"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Auto Create Folders
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AutoCreateFolders", "Auto Create Folders"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bTempAutoCreateFolders ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bTempAutoCreateFolders = (NewState == ECheckBoxState::Checked); })
			]
		]

		// Base Import Path
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BaseImportPath", "Base Import Path"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SEditableTextBox)
				.Text_Lambda([this]() { return FText::FromString(TempBaseImportPath); })
				.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type) { TempBaseImportPath = NewText.ToString(); })
			]
		];
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::ConstructScanningSection()
{
	return SNew(SVerticalBox)

		// Section Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ScanningSection", "Scanning"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Recursive Scan
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RecursiveScan", "Recursive Scan"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bTempRecursiveScan ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bTempRecursiveScan = (NewState == ECheckBoxState::Checked); })
			]
		]

		// Max Scan Depth
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MaxScanDepth", "Max Scan Depth (0 = unlimited)"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SSpinBox<int32>)
				.MinValue(0)
				.MaxValue(100)
				.Value_Lambda([this]() { return TempMaxScanDepth; })
				.OnValueChanged_Lambda([this](int32 NewValue) { TempMaxScanDepth = NewValue; })
			]
		];
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::ConstructButtonsSection()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNullWidget::NullWidget
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ResetButton", "Reset"))
			.OnClicked(this, &SFastAssetsSettingsDialog::OnResetClicked)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ApplyButton", "Apply"))
			.OnClicked(this, &SFastAssetsSettingsDialog::OnApplyClicked)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("CancelButton", "Cancel"))
			.OnClicked(this, &SFastAssetsSettingsDialog::OnCancelClicked)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("OKButton", "OK"))
			.OnClicked(this, &SFastAssetsSettingsDialog::OnOKClicked)
		];
}

void SFastAssetsSettingsDialog::LoadSettings()
{
	if (!Settings)
	{
		return;
	}

	TempViewMode = static_cast<int32>(Settings->DefaultViewMode);
	TempThumbnailSize = static_cast<int32>(Settings->ThumbnailSize);
	TempDuplicateHandling = static_cast<int32>(Settings->DuplicateHandling);
	bTempRememberLastPath = Settings->bRememberLastPath;
	bTempAutoImportOnDrop = Settings->bAutoImportOnDrop;
	bTempShowImportDialog = Settings->bShowImportDialog;
	bTempAutoCreateFolders = Settings->bAutoCreateFolders;
	bTempRecursiveScan = Settings->bRecursiveScan;
	TempBaseImportPath = Settings->BaseImportPath;
	TempMaxScanDepth = Settings->MaxScanDepth;
}

void SFastAssetsSettingsDialog::ApplySettings()
{
	if (!Settings)
	{
		return;
	}

	Settings->DefaultViewMode = static_cast<EFastAssetsDefaultView>(TempViewMode);
	Settings->ThumbnailSize = static_cast<EFastAssetsThumbnailSize>(TempThumbnailSize);
	Settings->DuplicateHandling = static_cast<EFastAssetsDuplicateHandling>(TempDuplicateHandling);
	Settings->bRememberLastPath = bTempRememberLastPath;
	Settings->bAutoImportOnDrop = bTempAutoImportOnDrop;
	Settings->bShowImportDialog = bTempShowImportDialog;
	Settings->bAutoCreateFolders = bTempAutoCreateFolders;
	Settings->bRecursiveScan = bTempRecursiveScan;
	Settings->BaseImportPath = TempBaseImportPath;
	Settings->MaxScanDepth = TempMaxScanDepth;

	Settings->SaveConfig();
}

FReply SFastAssetsSettingsDialog::OnResetClicked()
{
	if (Settings)
	{
		Settings->ResetToDefaults();
		LoadSettings();
	}
	return FReply::Handled();
}

FReply SFastAssetsSettingsDialog::OnApplyClicked()
{
	ApplySettings();
	return FReply::Handled();
}

FReply SFastAssetsSettingsDialog::OnCancelClicked()
{
	if (TSharedPtr<SWindow> Window = ParentWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SFastAssetsSettingsDialog::OnOKClicked()
{
	ApplySettings();

	if (TSharedPtr<SWindow> Window = ParentWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	return FReply::Handled();
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::GenerateViewModeComboItem(TSharedPtr<FString> Item)
{
	return SNew(STextBlock).Text(FText::FromString(*Item));
}

void SFastAssetsSettingsDialog::OnViewModeChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (NewValue.IsValid())
	{
		TempViewMode = ViewModeOptions.IndexOfByKey(NewValue);
	}
}

FText SFastAssetsSettingsDialog::GetCurrentViewModeText() const
{
	if (ViewModeOptions.IsValidIndex(TempViewMode))
	{
		return FText::FromString(*ViewModeOptions[TempViewMode]);
	}
	return FText::GetEmpty();
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::GenerateThumbnailSizeComboItem(TSharedPtr<FString> Item)
{
	return SNew(STextBlock).Text(FText::FromString(*Item));
}

void SFastAssetsSettingsDialog::OnThumbnailSizeChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (NewValue.IsValid())
	{
		TempThumbnailSize = ThumbnailSizeOptions.IndexOfByKey(NewValue);
	}
}

FText SFastAssetsSettingsDialog::GetCurrentThumbnailSizeText() const
{
	if (ThumbnailSizeOptions.IsValidIndex(TempThumbnailSize))
	{
		return FText::FromString(*ThumbnailSizeOptions[TempThumbnailSize]);
	}
	return FText::GetEmpty();
}

TSharedRef<SWidget> SFastAssetsSettingsDialog::GenerateDuplicateHandlingComboItem(TSharedPtr<FString> Item)
{
	return SNew(STextBlock).Text(FText::FromString(*Item));
}

void SFastAssetsSettingsDialog::OnDuplicateHandlingChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (NewValue.IsValid())
	{
		TempDuplicateHandling = DuplicateHandlingOptions.IndexOfByKey(NewValue);
	}
}

FText SFastAssetsSettingsDialog::GetCurrentDuplicateHandlingText() const
{
	if (DuplicateHandlingOptions.IsValidIndex(TempDuplicateHandling))
	{
		return FText::FromString(*DuplicateHandlingOptions[TempDuplicateHandling]);
	}
	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
