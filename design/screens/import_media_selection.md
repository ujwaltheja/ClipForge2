# Import/Media Selection Screen

**Purpose:** Select and manage media sources

**Components:**
- Tabs: Device | Camera | Library
- Recent/Grid View (4-column layout showing thumbnails)
- Video Cards with:
  - Thumbnail image
  - Duration overlay (bottom-right)
  - Title
  - Selection checkboxes
- Add to Timeline CTA at bottom

**Features:**
- Multi-source media browsing
- Thumbnail generation
- Duration calculation
- Multi-select support

**Data Requirements:**
- MediaStore integration
- File metadata
- Thumbnail cache
- Selected items state

**Related Code Files:**
- `app/src/main/java/com/clipforge/ui/ImportActivity.kt` (Kotlin activity)
- `app/src/main/java/com/clipforge/adapter/MediaAdapter.kt` (Adapter)
- `app/src/main/res/layout/activity_import.xml` (Layout)
- `ClipForge/NDK/media/media_extractor.h` (C++ for MediaStore)
- `ClipForge/NDK/media/thumbnail_generator.h` (C++ for thumbnails)
