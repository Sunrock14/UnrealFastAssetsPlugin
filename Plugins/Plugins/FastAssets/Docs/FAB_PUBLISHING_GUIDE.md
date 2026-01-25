# FastAssets - Fab Marketplace Yayınlama Rehberi

Bu rehber, FastAssets plugin'ini Fab (eski adıyla Unreal Marketplace) üzerinde satışa sunmak için gereken adımları açıklar.

---

## 1. Ön Hazırlık

### 1.1 Fab Satıcı Hesabı Oluşturma

1. [fab.com/sellers](https://fab.com/sellers) adresine gidin
2. "Become a Seller" butonuna tıklayın
3. Epic Games hesabınızla giriş yapın
4. Satıcı profilinizi oluşturun:
   - Satıcı adı (şirket veya kişisel)
   - İletişim bilgileri
   - Ödeme bilgileri (Hyperwallet, PayPal, veya banka transferi)
   - Vergi bilgileri (W-9 veya W-8BEN formu)

### 1.2 Gerekli Belgeler

| Belge | Açıklama |
|-------|----------|
| **Kimlik Doğrulama** | Devlet tarafından verilmiş kimlik belgesi |
| **Vergi Formu** | ABD için W-9, uluslararası için W-8BEN |
| **Ödeme Bilgileri** | Banka hesabı veya PayPal |

---

## 2. Plugin Hazırlığı

### 2.1 Versiyon ve Metadata Güncelleme

`FastAssets.uplugin` dosyasını güncelleyin:

```json
{
    "FileVersion": 3,
    "Version": 1,
    "VersionName": "1.0.0",
    "FriendlyName": "Fast Assets",
    "Description": "Quick asset discovery, preview, and drag-drop import from external folders",
    "Category": "Editor",
    "CreatedBy": "Ismail Faruk Kocademir",
    "CreatedByURL": "https://farukkocademir.web.tr/",
    "DocsURL": "https://github.com/YOUR_REPO/FastAssets/wiki",
    "MarketplaceURL": "",
    "SupportURL": "https://github.com/YOUR_REPO/FastAssets/issues",
    "EngineVersion": "5.4.0",
    "EnabledByDefault": true,
    "CanContainContent": false,
    "IsBetaVersion": false,
    "IsExperimentalVersion": false,
    "Installed": true,
    "Modules": [
        {
            "Name": "FastAssets",
            "Type": "Editor",
            "LoadingPhase": "Default",
            "PlatformAllowList": ["Win64", "Mac", "Linux"]
        }
    ]
}
```

### 2.2 Gerekli Dosya Yapısı

```
FastAssets/
├── FastAssets.uplugin
├── Resources/
│   ├── Icon128.png              # 128x128 plugin ikonu (zorunlu)
│   ├── Icon256.png              # 256x256 yüksek çözünürlük
│   └── ButtonIcon_40x.png       # 40x40 toolbar ikonu
├── Source/
│   └── FastAssets/
│       ├── Public/
│       └── Private/
├── Config/
│   └── FilterPlugin.ini
└── Docs/
    ├── README.md
    └── CHANGELOG.md
```

### 2.3 İkon Gereksinimleri

| İkon | Boyut | Format | Kullanım |
|------|-------|--------|----------|
| Plugin İkonu | 128x128 px | PNG | Plugin listesinde |
| Büyük İkon | 256x256 px | PNG | Detay sayfasında |
| Toolbar İkonu | 40x40 px | PNG/SVG | Editor toolbar |

**Tasarım Önerileri:**
- Basit, tanınabilir sembol kullanın
- Koyu arka plan üzerinde görünür renkler
- Unreal Engine stil rehberine uygun

### 2.4 Kod Kalitesi Kontrolleri

Yayınlamadan önce kontrol edin:

```bash
# Derleme testi (tüm platformlar)
# Windows
"C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" FastAssets Win64 Development

# Kod standardı kontrolü
# - Epic Coding Standards uyumu
# - Tüm LOCTEXT kullanımları
# - Hardcoded string olmaması
# - Memory leak kontrolü
```

**Kontrol Listesi:**
- [ ] Tüm public API'ler dökümante edilmiş
- [ ] LOCTEXT ile lokalizasyon desteği
- [ ] Proper error handling
- [ ] No hardcoded paths
- [ ] Cross-platform uyumluluk

---

## 3. Görsel Materyaller

### 3.1 Ürün Görselleri (Zorunlu)

Fab için en az 5 görsel gerekli:

| Görsel | Boyut | İçerik |
|--------|-------|--------|
| **Ana Görsel** | 1920x1080 | Plugin ana penceresi |
| **Grid View** | 1920x1080 | Grid görünümü ekran görüntüsü |
| **List View** | 1920x1080 | Liste görünümü |
| **Drag & Drop** | 1920x1080 | Viewport'a sürükleme |
| **Settings** | 1920x1080 | Ayarlar penceresi |

**Görsel Hazırlama İpuçları:**
- Yüksek çözünürlükte (4K tercih)
- Temiz, düzenli UI
- Gerçek kullanım senaryoları
- Annotasyon/ok işaretleri eklenebilir

### 3.2 Video (Önerilen)

1-3 dakikalık tanıtım videosu:
- Plugin kurulumu
- Temel kullanım
- Drag & drop gösterimi
- Ayarlar açıklaması

**Video Özellikleri:**
- 1080p veya 4K
- YouTube veya Vimeo linki
- İngilizce sesli veya altyazılı

---

## 4. Fab'a Ürün Ekleme

### 4.1 Seller Portal'a Giriş

1. [fab.com/sellers/dashboard](https://fab.com/sellers/dashboard) adresine gidin
2. "Create New Product" butonuna tıklayın
3. "Plugin" kategorisini seçin

### 4.2 Temel Bilgiler

**Product Name:** Fast Assets

**Tagline (Kısa Açıklama - max 100 karakter):**
```
Quick asset discovery, preview, and drag-drop import from external folders
```

**Categories:**
- Primary: Editor Tools
- Secondary: Productivity

**Engine Compatibility:**
- UE 5.4, 5.5, 5.6, 5.7 (test ettiğiniz sürümler)

### 4.3 Fiyatlandırma

| Fiyat Aralığı | Önerilen |
|---------------|----------|
| Ücretsiz | Tanıtım/demo için |
| $4.99 - $9.99 | Basit araçlar |
| $14.99 - $24.99 | **Orta düzey plugin** |
| $29.99 - $49.99 | Kapsamlı araçlar |
| $50+ | Profesyonel çözümler |

**Önerilen Fiyat:** $14.99 - $19.99

**Fiyatlandırma Stratejisi:**
- Launch discount (%20-30)
- Bundle seçeneği (gelecek ürünlerle)
- Site-wide sale'lere katılım

### 4.4 Açıklama ve İçerik

`FAB_PRODUCT_DESCRIPTION.md` dosyasını referans alın.

### 4.5 Teknik Detaylar

```
Supported Platforms: Win64, Mac, Linux
Engine Versions: 5.4, 5.5, 5.6, 5.7
Module Type: Editor Only
Network Replicated: No
Documentation: Included
```

---

## 5. Paketleme ve Yükleme

### 5.1 Plugin Paketleme

```bash
# RunUAT ile paketleme
"C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="E:\_Unreal\AssetDragDropPlugin\Plugins\Plugins\FastAssets\FastAssets.uplugin" -Package="E:\FastAssets_Packaged" -Rocket

# veya manuel zip
# FastAssets klasörünü direkt zip'leyin
```

### 5.2 Paket İçeriği Kontrolü

Zip dosyası şunları içermeli:
```
FastAssets.zip
└── FastAssets/
    ├── FastAssets.uplugin
    ├── Resources/
    ├── Source/
    ├── Config/
    └── Docs/
```

### 5.3 Yükleme

1. Seller Dashboard'da "Upload Files" bölümüne gidin
2. ZIP dosyasını yükleyin
3. Her engine versiyonu için ayrı build yükleyin (gerekirse)

---

## 6. İnceleme Süreci

### 6.1 Epic Games İnceleme Kriterleri

| Kriter | Açıklama |
|--------|----------|
| **Kod Kalitesi** | Derleme hataları, uyarılar |
| **Stabilite** | Crash, memory leak |
| **Performans** | Yüksek CPU/memory kullanımı |
| **Güvenlik** | Zararlı kod, data toplama |
| **Dökümantasyon** | Kurulum, kullanım talimatları |
| **Uyumluluk** | Belirtilen engine versiyonları |

### 6.2 İnceleme Süresi

- İlk inceleme: 5-10 iş günü
- Revizyon gerekirse: +3-5 iş günü
- Toplam: 1-3 hafta

### 6.3 Sık Reddedilme Nedenleri

1. **Derleme Hataları** - Farklı platformlarda test edin
2. **Eksik Dökümantasyon** - README.md zorunlu
3. **Yetersiz Görseller** - Minimum 5 kaliteli görsel
4. **Açıklama Sorunları** - İngilizce, net, yanıltıcı olmayan
5. **Uyumluluk İddiaları** - Test etmediğiniz versiyonu belirtmeyin

---

## 7. Yayın Sonrası

### 7.1 Güncelleme Yönetimi

- Bug fix'leri hızlıca yayınlayın
- Changelog güncel tutun
- Major güncellemeleri duyurun

### 7.2 Kullanıcı Desteği

- Sorulara 24-48 saat içinde yanıt verin
- Bug raporlarını takip edin
- Feature request'leri değerlendirin

### 7.3 Pazarlama

- Unreal Engine forum'larında paylaşın
- Social media tanıtımı
- YouTube tutorial videoları
- Discord community

---

## 8. Gelir ve Vergi

### 8.1 Gelir Paylaşımı

| Fab Komisyonu | Satıcı Payı |
|---------------|-------------|
| %12 | %88 |

### 8.2 Ödeme Dönemleri

- Aylık ödeme
- Minimum eşik: $100
- Ödeme yöntemleri: PayPal, Banka transferi, Hyperwallet

### 8.3 Vergi Sorumlulukları

- Türkiye'den satış yapıyorsanız, gelir vergisi beyannamesi
- ABD'ye satışlarda W-8BEN formu ile %30 stopaj önleme
- Fatura/makbuz düzenleme gereklilikleri

---

## 9. Kontrol Listesi

### Yayın Öncesi

- [ ] Plugin derleme testi (Win64, Mac, Linux)
- [ ] Tüm engine versiyonlarında test
- [ ] Dökümantasyon tamamlandı
- [ ] Görseller hazırlandı (min. 5 adet)
- [ ] Video hazırlandı (önerilen)
- [ ] Fiyat belirlendi
- [ ] Ürün açıklaması yazıldı
- [ ] Changelog oluşturuldu
- [ ] Plugin paketlendi

### Yayın Sonrası

- [ ] Forum duyurusu yapıldı
- [ ] Social media paylaşımı
- [ ] İlk kullanıcı geri bildirimleri takip edildi
- [ ] Bug fix'ler planlandı

---

## 10. Faydalı Linkler

- [Fab Seller Portal](https://fab.com/sellers)
- [Fab Submission Guidelines](https://fab.com/help/sellers/submission-guidelines)
- [Epic Coding Standards](https://docs.unrealengine.com/epic-cplusplus-coding-standard)
- [Unreal Plugin Development](https://docs.unrealengine.com/plugins)
- [Fab Seller Community](https://forums.unrealengine.com/c/marketplace-sellers)

---

**Son Güncelleme:** 2025-01-25
**Yazar:** Ismail Faruk Kocademir
