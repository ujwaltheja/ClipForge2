# Editing Timeline Screen

**Purpose:** Main video editing interface with timeline-based editing

**Components:**
- Video Preview Area (large, showing current frame)
- Playback Controls:
  - Play/Pause button
  - Progress bar (yellow/gold progress indicator)
  - Timestamp display
- Bottom Section:
  - Tabs: Effects | Main | Library
  - Timeline with clips (scrollable)
  - Track visualization
  - Thumbnails of clips in timeline

**Features:**
- Multi-track editing (Effects, Main, Music tracks visible)
- Real-time preview
- Timeline scrubbing
- Clip arrangement

**Data Requirements:**
- Video timeline data
- Clip positions and durations
- Current playback position
- Applied effects metadata

**Related Code Files:**
- `app/src/main/java/com/clipforge/ui/EditorActivity.kt` (Kotlin activity)
- `app/src/main/java/com/clipforge/view/TimelineView.kt` (Custom view)
- `app/src/main/res/layout/activity_editor.xml` (Layout)
- `ClipForge/NDK/core/timeline_manager.h` (C++ for timeline)
- `ClipForge/NDK/core/frame_processor.h` (C++ for preview)
