# FastAssets - Product Requirements Document (PRD)

**Versiyon**: 1.0
**Tarih**: 2025-01-14
**Yazar**: Ismail Faruk Kocademir
**Platform**: Unreal Engine 5.7+
**Hedef Pazar**: FAB Marketplace

---

## 1. Executive Summary

### 1.1 Ürün Tanımı
**FastAssets**, Unreal Engine için geliştirilmiş bir editor plugin'idir. Kullanıcıların harici klasörlerdeki asset dosyalarını taramasına, önizlemesine ve sürükle-bırak yöntemiyle projelerine import etmesine olanak tanır.

### 1.2 Problem Tanımı
Unreal Engine geliştiricileri, harici kaynaklardan (asset paketleri, indirilen modeller, ses dosyaları vb.) asset import ederken şu sorunlarla karşılaşır:
- Manuel dosya tarama ve seçme zahmetli
- Import edilen dosyaların düzensiz klasör yapısı
- Önizleme olmadan asset seçimi
- Tekrarlayan import işlemleri

### 1.3 Çözüm
FastAssets, tek bir pencereden:
- Harici klasörleri tarar ve asset'leri listeler
- Thumbnail önizleme sunar
- Sürükle-bırak ile level'e yerleştirme sağlar
- Asset tipine göre otomatik klasör organizasyonu yapar

### 1.4 Hedef Kullanıcılar
- Level Tasarımcıları
- 3D Sanatçılar
- Ses Tasarımcıları
- Indie Geliştiriciler
- AAA Stüdyo Ekipleri

---

## 2. Ürün Özellikleri

### 2.1 Harici Klasör Tarama

#### 2.1.1 Klasör Seçimi
- Native OS dosya dialog penceresi
- Son kullanılan klasörleri hatırlama
- Birden fazla klasör ekleme desteği
- Klasör yer imleri (favorites)

#### 2.1.2 Desteklenen Dosya Formatları

| Kategori | Formatlar |
|----------|-----------|
| **3D Modeller** | FBX, OBJ, GLTF, GLB, USD, USDA, USDC, ABC (Alembic) |
| **Ses Dosyaları** | WAV, MP3, OGG, FLAC, AIFF |
| **Görüntüler/Texture** | PNG, JPG, JPEG, TGA, BMP, EXR, HDR, PSD, TIFF |
| **Unreal Native** | UASSET, UMAP |
| **Data** | JSON, CSV (DataTable için) |

#### 2.1.3 Tarama Özellikleri
- Recursive alt klasör tarama (opsiyonel)
- Dosya filtresi (tip, boyut, tarih)
- Tarama derinliği limiti
- Büyük klasörlerde async tarama

---

### 2.2 Önizleme Penceresi

#### 2.2.1 Pencere Modları

**Dockable Tab**
- Content Browser benzeri entegrasyon
- Herhangi bir editor paneline sabitlenebilir
- Tab olarak gruplandırılabilir
- Layout ile birlikte kaydedilir

**Standalone Window**
- Bağımsız floating pencere
- Multi-monitor desteği
- Always-on-top seçeneği
- Boyut ve konum hatırlama

#### 2.2.2 Görünüm Modları

**Grid Görünümü (Thumbnail)**
```
┌─────────┐ ┌─────────┐ ┌─────────┐
│  [IMG]  │ │  [IMG]  │ │  [IMG]  │
│─────────│ │─────────│ │─────────│
│ Chair   │ │ Table   │ │ Lamp    │
└─────────┘ └─────────┘ └─────────┘
```
- Ayarlanabilir thumbnail boyutu (Small/Medium/Large)
- Asset tipi ikonu overlay
- Dosya adı gösterimi
- Hover'da detay tooltip

**List Görünümü (Detaylı)**
```
┌──────┬────────────┬──────────┬─────────┬──────────┐
│ Icon │ Name       │ Type     │ Size    │ Path     │
├──────┼────────────┼──────────┼─────────┼──────────┤
│ [M]  │ Chair.fbx  │ Mesh     │ 2.4 MB  │ /Models/ │
│ [S]  │ Click.wav  │ Sound    │ 124 KB  │ /Audio/  │
└──────┴────────────┴──────────┴─────────┴──────────┘
```
- Sütun sıralama
- Sütun genişlik ayarlama
- Multi-select desteği

#### 2.2.3 Arama ve Filtreleme
- Real-time arama (dosya adı)
- Tip filtresi (Mesh, Sound, Texture, vb.)
- Tarih filtresi
- Boyut filtresi
- Regex desteği (gelişmiş)

---

### 2.3 Sürükle-Bırak (Drag & Drop)

#### 2.3.1 Sürükleme Kaynakları
- Grid'den tek asset
- List'ten tek/çoklu asset
- Klasör (tüm içerik)

#### 2.3.2 Bırakma Hedefleri

**Level Viewport**
- Mouse konumuna ray-cast ile yerleştirme
- Surface snapping
- Grid snapping (editor ayarlarına uygun)

**Content Browser**
- Seçili klasöre import
- Var olan asset üzerine bırakma (replace dialog)

**Outliner**
- Parent actor'a ekleme
- Folder'a gruplama

#### 2.3.3 Import Süreci
1. Dosya tipini tespit et
2. Hedef klasörü belirle (otomatik organizasyon)
3. Var olan asset kontrolü (duplicate check)
4. Import settings dialog (opsiyonel)
5. Asset'i import et
6. Level'e spawn et (viewport'a bırakıldıysa)

---

### 2.4 Otomatik Klasör Organizasyonu

#### 2.4.1 Varsayılan Klasör Yapısı
```
Content/
└── Assets/
    ├── Meshes/
    │   ├── Characters/
    │   ├── Props/
    │   ├── Environment/
    │   └── Vehicles/
    ├── Sounds/
    │   ├── SFX/
    │   ├── Music/
    │   └── Ambient/
    ├── Textures/
    │   ├── Characters/
    │   ├── Environment/
    │   └── UI/
    ├── Materials/
    ├── Animations/
    ├── Blueprints/
    └── Misc/
```

#### 2.4.2 Asset Tip Eşleştirmesi

| Import Edilen Tip | Hedef Klasör |
|-------------------|--------------|
| Static Mesh (FBX, OBJ, GLTF) | /Assets/Meshes/ |
| Skeletal Mesh | /Assets/Meshes/ |
| Sound Wave (WAV, MP3, OGG) | /Assets/Sounds/ |
| Texture (PNG, JPG, TGA) | /Assets/Textures/ |
| Material Instance | /Assets/Materials/ |
| Animation Sequence | /Assets/Animations/ |
| Blueprint | /Assets/Blueprints/ |
| Diğer | /Assets/Misc/ |

#### 2.4.3 Özelleştirme
- Kullanıcı tanımlı klasör kuralları
- Regex tabanlı yol belirleme
- Kaynak klasör yapısını koruma seçeneği

---

### 2.5 Ayarlar ve Tercihler

#### 2.5.1 Genel Ayarlar
- Varsayılan görünüm modu
- Thumbnail boyutu
- Son kullanılan klasörler sayısı
- Auto-refresh interval

#### 2.5.2 Import Ayarları
- Varsayılan import settings (per asset type)
- Duplicate handling (skip/replace/rename)
- Auto-import on drop
- Import settings dialog gösterme

#### 2.5.3 Organizasyon Ayarları
- Otomatik klasör oluşturma
- Özel klasör mapping'leri
- Prefix/Suffix kuralları

---

## 3. Teknik Mimari

### 3.1 Module Yapısı
```
FastAssets/
├── FastAssets.uplugin
└── Source/
    └── FastAssets/
        ├── Public/
        │   ├── FastAssets.h
        │   ├── FastAssetsCommands.h
        │   ├── FastAssetsStyle.h
        │   ├── SFastAssetsWindow.h
        │   ├── SAssetTileView.h
        │   ├── SAssetListView.h
        │   ├── FExternalAssetScanner.h
        │   ├── FAssetDragDropOp.h
        │   └── FFastAssetsSettings.h
        ├── Private/
        │   ├── FastAssets.cpp
        │   ├── FastAssetsCommands.cpp
        │   ├── FastAssetsStyle.cpp
        │   ├── SFastAssetsWindow.cpp
        │   ├── SAssetTileView.cpp
        │   ├── SAssetListView.cpp
        │   ├── FExternalAssetScanner.cpp
        │   ├── FAssetDragDropOp.cpp
        │   └── FFastAssetsSettings.cpp
        └── FastAssets.Build.cs
```

### 3.2 Temel Sınıflar

#### 3.2.1 FFastAssetsModule
- Plugin yaşam döngüsü yönetimi
- UI registration
- Menu/toolbar entegrasyonu

#### 3.2.2 SFastAssetsWindow
- Ana Slate penceresi
- Toolbar (refresh, settings, view toggle)
- Klasör seçici
- Asset view container

#### 3.2.3 SAssetTileView
- Grid görünümü widget
- STileView kullanımı
- Thumbnail rendering

#### 3.2.4 SAssetListView
- List görünümü widget
- SListView kullanımı
- Sortable columns

#### 3.2.5 FExternalAssetScanner
- Dosya sistemi tarama
- Async directory traversal
- File type detection
- Thumbnail generation

#### 3.2.6 FAssetDragDropOp
- Custom drag-drop operation
- Visual feedback
- Drop target validation

#### 3.2.7 FFastAssetsSettings
- UDeveloperSettings derived
- Project Settings entegrasyonu
- Serialization

### 3.3 Bağımlılıklar
```cpp
// FastAssets.Build.cs
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject",
    "Engine",
    "Slate",
    "SlateCore",
    "InputCore",
    "EditorStyle",
    "UnrealEd",
    "AssetTools",
    "ContentBrowser",
    "Projects",
    "ToolMenus",
    "EditorSubsystem",
    "LevelEditor"
});
```

### 3.4 API Kullanımları

#### IAssetTools
```cpp
IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(FilePaths, DestinationPath);
```

#### FAssetRegistryModule
```cpp
FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
AssetRegistry.Get().GetAssetsByPath(Path, OutAssets);
```

#### SDockTab
```cpp
FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName,
    FOnSpawnTab::CreateRaw(this, &FFastAssetsModule::OnSpawnTab))
    .SetDisplayName(LOCTEXT("TabTitle", "Fast Assets"))
    .SetMenuType(ETabSpawnerMenuType::Hidden);
```

---

## 4. UI/UX Tasarımı

### 4.1 Ana Pencere Layout
```
┌─────────────────────────────────────────────────────────────┐
│ [Fast Assets]                                    [_][□][X] │
├─────────────────────────────────────────────────────────────┤
│ Toolbar: [📁 Browse] [🔄 Refresh] [⚙ Settings] | [▦][≡]   │
├─────────────────────────────────────────────────────────────┤
│ Path: [C:/Assets/MyModels                           ] [📁] │
├─────────────────────────────────────────────────────────────┤
│ Search: [🔍 Type to search...        ] Filter: [All Types▼]│
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐       │
│  │  [IMG]  │  │  [IMG]  │  │  [IMG]  │  │  [IMG]  │       │
│  │─────────│  │─────────│  │─────────│  │─────────│       │
│  │ Chair   │  │ Table   │  │ Lamp    │  │ Door    │       │
│  │ .fbx    │  │ .fbx    │  │ .obj    │  │ .gltf   │       │
│  └─────────┘  └─────────┘  └─────────┘  └─────────┘       │
│                                                             │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐       │
│  │  [IMG]  │  │  [IMG]  │  │  [IMG]  │  │  [IMG]  │       │
│  │─────────│  │─────────│  │─────────│  │─────────│       │
│  │ Wood    │  │ Metal   │  │ Click   │  │ Ambient │       │
│  │ .png    │  │ .tga    │  │ .wav    │  │ .mp3    │       │
│  └─────────┘  └─────────┘  └─────────┘  └─────────┘       │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ Status: 156 assets found | 12 selected | Ready             │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Toolbar İkonları
| İkon | İşlev |
|------|-------|
| 📁 Browse | Klasör seçim dialog |
| 🔄 Refresh | Listeyi yenile |
| ⚙ Settings | Ayarlar paneli |
| ▦ Grid | Grid görünümü |
| ≡ List | Liste görünümü |

### 4.3 Context Menu (Sağ Tık)
```
┌─────────────────────┐
│ Import to Project   │
│ Import and Place    │
│ ─────────────────── │
│ Open in Explorer    │
│ Copy Path           │
│ ─────────────────── │
│ Properties...       │
└─────────────────────┘
```

### 4.4 Ayarlar Penceresi
```
┌─────────────────────────────────────────────┐
│ Fast Assets Settings                  [X]   │
├─────────────────────────────────────────────┤
│ General                                     │
│ ├─ Default View: [Grid ▼]                  │
│ ├─ Thumbnail Size: [●────] 128px           │
│ └─ Remember Last Path: [✓]                 │
│                                             │
│ Import                                      │
│ ├─ Auto-import on Drop: [✓]                │
│ ├─ Show Import Dialog: [ ]                  │
│ └─ Duplicate Handling: [Ask ▼]             │
│                                             │
│ Organization                                │
│ ├─ Auto-create Folders: [✓]                │
│ ├─ Base Path: [/Game/Assets]               │
│ └─ [Configure Folder Rules...]             │
│                                             │
│               [Reset] [Apply] [Cancel] [OK]│
└─────────────────────────────────────────────┘
```

---

## 5. FAB Marketplace Gereksinimleri

### 5.1 Plugin Metadata
```json
{
    "FileVersion": 3,
    "Version": 1,
    "VersionName": "1.0.0",
    "FriendlyName": "Fast Assets",
    "Description": "Quick asset import with drag-and-drop functionality",
    "Category": "Editor",
    "CreatedBy": "Ismail Faruk Kocademir",
    "CreatedByURL": "https://farukkocademir.web.tr/",
    "DocsURL": "https://github.com/...",
    "MarketplaceURL": "",
    "SupportURL": "https://github.com/.../issues",
    "EngineVersion": "5.7.0",
    "EnabledByDefault": true,
    "CanContainContent": true,
    "IsBetaVersion": false,
    "IsExperimentalVersion": false,
    "Installed": true
}
```

### 5.2 Gerekli Dosyalar
```
FastAssets/
├── FastAssets.uplugin
├── Resources/
│   ├── Icon128.png          (128x128 plugin icon)
│   └── ButtonIcon_40x.png   (40x40 toolbar icon)
├── Source/
│   └── ...
├── Content/
│   └── (örnek assets - opsiyonel)
├── Docs/
│   ├── README.md
│   └── CHANGELOG.md
└── Config/
    └── FilterPlugin.ini
```

### 5.3 Kod Standartları
- Epic Games Coding Standard uyumu
- Tüm public API'ler dökümante edilmeli
- LOCTEXT kullanımı (lokalizasyon desteği)
- No hardcoded strings
- Proper error handling
- Memory leak kontrolü

### 5.4 Test Gereksinimleri
- Automation tests
- Farklı asset tipleriyle test
- Large folder test (1000+ files)
- Edge case handling
- Multi-platform test (Windows zorunlu)

### 5.5 Dökümantasyon
- README.md (kurulum, kullanım)
- API dökümantasyonu
- Video tutorial (önerilen)
- Ekran görüntüleri

---

## 6. Geliştirme Yol Haritası

### Phase 1: Temel Altyapı
- [ ] Plugin module yapısı
- [ ] Temel Slate penceresi
- [ ] Toolbar ve menü entegrasyonu
- [ ] Klasör seçim dialog

### Phase 2: Asset Tarama
- [ ] Dosya sistemi tarayıcı
- [ ] Async tarama
- [ ] Dosya tipi tespiti
- [ ] Thumbnail generation

### Phase 3: UI Geliştirme
- [ ] Grid görünümü (SAssetTileView)
- [ ] List görünümü (SAssetListView)
- [ ] Arama ve filtreleme
- [ ] View switching

### Phase 4: Drag & Drop
- [ ] Custom DragDropOperation
- [ ] Viewport drop handling
- [ ] Content Browser drop
- [ ] Visual feedback

### Phase 5: Import Sistemi
- [ ] Asset import pipeline
- [ ] Otomatik klasör organizasyonu
- [ ] Duplicate handling
- [ ] Import settings

### Phase 6: Ayarlar ve Polish
- [ ] Settings panel
- [ ] Project Settings entegrasyonu
- [ ] Error handling
- [ ] Performance optimization

### Phase 7: FAB Hazırlık
- [ ] Dökümantasyon
- [ ] İkonlar ve görseller
- [ ] Test suite
- [ ] Release packaging

---

## 7. Riskler ve Çözümler

| Risk | Olasılık | Etki | Çözüm |
|------|----------|------|-------|
| Büyük klasör performansı | Yüksek | Orta | Async tarama, pagination |
| Thumbnail memory kullanımı | Orta | Orta | Lazy loading, cache limiti |
| Import hataları | Orta | Yüksek | Detaylı error handling, rollback |
| Cross-platform uyumsuzluk | Düşük | Yüksek | Platform abstraction, test |

---

## 8. Başarı Metrikleri

- Tarama süresi: <5 saniye (1000 dosya için)
- UI responsiveness: <100ms input lag
- Memory kullanımı: <500MB (10000 asset için)
- Crash rate: %0
- FAB onay süreci: İlk denemede geçiş

---

## 9. Ekler

### 9.1 Referans Eklentiler
- Unreal Engine Content Browser
- Quixel Bridge
- Datasmith

### 9.2 Faydalı Kaynaklar
- [Slate UI Framework Documentation](https://docs.unrealengine.com/slate)
- [Asset Registry Documentation](https://docs.unrealengine.com/assetregistry)
- [FAB Submission Guidelines](https://fab.com/sellers)

---

**Doküman Sonu**
