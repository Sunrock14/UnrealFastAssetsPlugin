# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**FastAssets** is an Unreal Engine 5.7+ editor plugin that enables quick asset discovery, preview, and import from external directories via drag-and-drop. The plugin provides a dockable window for browsing external folders, previewing assets with thumbnails, and importing them into the project with automatic folder organization.

- **Module Type**: Editor-only
- **Status**: Beta v1.0
- **Author**: Ismail Faruk Kocademir

## Build Commands

This is an Unreal Engine plugin. Build through the engine:

```bash
# Generate project files (Windows)
# Right-click AssetDragDropPlugin.uproject → Generate Visual Studio project files

# Build from command line (Windows)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" AssetDragDropPluginEditor Win64 Development "E:\_Unreal\AssetDragDropPlugin\AssetDragDropPlugin.uproject" -waitmutex

# Build from command line (alternative using RunUAT)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="Plugins/Plugins/FastAssets/FastAssets.uplugin" -Package="PackagedPlugin" -Rocket
```

No separate test framework is configured. Testing is done by loading the plugin in the Unreal Editor.

## Architecture

### Module Structure

```
FFastAssetsModule (entry point)
├── UI Layer
│   ├── SFastAssetsWindow - Main dockable window with toolbar, path bar, search, and asset grid/list
│   ├── SFastAssetsSettingsDialog - Modal settings configuration panel
│   ├── SAssetTableRow - Table row widgets for list/grid views
│   └── SAssetTile / SAssetListRow - Individual asset display widgets
├── Business Logic
│   ├── FFastAssetImporter - Import pipeline for external assets
│   ├── FFastAssetsDropHandler - Handles drops in viewport and content browser
│   └── FFastAssetsThumbnail - Thumbnail loading and caching
└── Support
    ├── UFastAssetsSettings - UDeveloperSettings for project settings integration
    ├── FFastAssetDragDropOp - Custom drag-drop operation carrying asset data
    ├── FFastAssetsStyle - Slate style management
    └── FFastAssetsCommands - Editor command bindings
```

### Key Data Structures

**FExternalAssetItem**: Represents a scanned external file with path, filename, extension, asset type, file size, modified time, and cached thumbnail brush.

**FImportResult**: Import operation result with success flag, imported asset path/reference, and error message.

### Drag-Drop Flow

1. User drags from FastAssets window → creates `FFastAssetDragDropOp`
2. Drop targets registered by `FFastAssetsDropHandler`:
   - **Level Viewport**: Raycast placement, imports then spawns actor
   - **Content Browser**: Direct import to specified folder
3. Import uses `FFastAssetImporter` which routes by asset type to appropriate folder

### Asset Type Organization

Assets are auto-organized by type to folders under a configurable base path (default `/Game/Assets`):
- 3D Models (FBX, OBJ, GLTF, GLB) → `/Assets/Meshes/`
- Audio (WAV, MP3, OGG) → `/Assets/Sounds/`
- Textures (PNG, JPG, TGA) → `/Assets/Textures/`
- Animations → `/Assets/Animations/`
- Other → `/Assets/Misc/`

## Key Files

| File | Purpose |
|------|---------|
| `FastAssets.cpp` | Module startup/shutdown, tab registration, menu integration |
| `SFastAssetsWindow.cpp` | Main UI (~800 lines) - toolbar, directory scanning, view modes, drag detection |
| `FastAssetImporter.cpp` | Import pipeline - type detection, folder routing, UE import API calls |
| `FastAssetsDropHandler.cpp` | Drop target registration for viewport and content browser |
| `FastAssetsThumbnail.cpp` | Thumbnail loading with TMap cache, async loading support |
| `FastAssetsSettings.cpp` | UDeveloperSettings subclass for project settings |

## Module Dependencies

**Public**: Core, CoreUObject, Engine, InputCore

**Private** (notable): UnrealEd, Slate, SlateCore, AssetTools, ContentBrowser, AssetRegistry, DesktopPlatform, DeveloperSettings, ImageWrapper

## Extending the Plugin

### Adding New Asset Type Support

1. Add extension check in `FFastAssetImporter::CanImportFileType()`
2. Add folder mapping in `FFastAssetImporter` constructor: `AssetTypeToFolder.Add(TEXT("NewType"), TEXT("Folder"))`
3. Create specialized import method if needed, or use `ImportGeneric()`
4. Update type detection in `SFastAssetsWindow::DetermineAssetType()`

### Adding New Settings

1. Add `UPROPERTY` to `UFastAssetsSettings` class
2. Add UI control in `SFastAssetsSettingsDialog` section method
3. Add temporary variable and binding in settings dialog
4. Apply in `ApplySettings()` method
