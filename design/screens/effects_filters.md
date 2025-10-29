# Effects/Filters Screen

**Purpose:** Apply visual effects to video clips

**Components:**
- Back navigation
- Screen Title: "Filters"
- Video Preview (large with filter applied)
- Playback Controls
- Filter Grid (4 preview tiles):
  - Original
  - Nostalgia
  - Midnight
  - Festive
  - (and more variations)

**Features:**
- Filter library with previews
- Real-time preview
- One-click application
- Filter categories

**Data Requirements:**
- Filter definitions
- Filter preview rendering
- Applied filters state

**Related Code Files:**
- `app/src/main/java/com/clipforge/ui/EffectsActivity.kt` (Kotlin activity)
- `app/src/main/java/com/clipforge/adapter/FilterAdapter.kt` (Adapter)
- `app/src/main/res/layout/activity_effects.xml` (Layout)
- `ClipForge/NDK/effects/effects_processor.h` (C++ for effects)
- `ClipForge/NDK/effects/filter_library.h` (C++ for filters)
