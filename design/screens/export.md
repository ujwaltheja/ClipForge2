# Export Screen

**Purpose:** Configure and export final video

**Components:**
- Screen Title: "Export Your Reel"
- Platform Selection Grid:
  - TikPai's Made (with duration: 7 seconds)
  - TikTok Gros (with duration: 9 seconds)
  - Instagram Tarn (with duration: 10 seconds)
  - TikTok Video (separate option)
  - YouTube Video
  - Instagram Reel
- Quality/Format Options
- Export CTA Button

**Features:**
- Multi-platform export templates
- Aspect ratio adjustment
- Duration optimization
- Quality presets

**Data Requirements:**
- Export presets
- Video rendering parameters
- Output format configurations

**Related Code Files:**
- `app/src/main/java/com/clipforge/ui/ExportActivity.kt` (Kotlin activity)
- `app/src/main/java/com/clipforge/viewmodel/ExportViewModel.kt` (ViewModel)
- `app/src/main/res/layout/activity_export.xml` (Layout)
- `ClipForge/NDK/rendering/export_engine.h` (C++ for export)
