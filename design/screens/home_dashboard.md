# Home/Dashboard Screen

**Purpose:** Main entry point, project management, quick access

**Components:**
- Logo: ClipForge (purple play icon gradient)
- CTA: "+ New Project" (bold purple button)
- Quick Actions Section:
  - Import Media (with icon)
  - Record Video (with icon)
  - Browse Templates (with icon)
- Recent Projects Grid (2-column layout)
- Bottom Navigation (3 tabs: Home, Search, Settings)

**User Interactions:**
- Create new project
- Access recent projects
- Quick navigation to other screens
- Search projects
- Access settings

**Data Requirements:**
- Recent projects list
- Project metadata (name, thumbnail, duration)
- Navigation state

**Related Code Files:**
- `app/src/main/java/com/clipforge/ui/HomeActivity.kt` (Kotlin activity)
- `app/src/main/java/com/clipforge/viewmodel/HomeViewModel.kt` (ViewModel)
- `app/src/main/res/layout/activity_home.xml` (Layout)
- `ClipForge/NDK/core/timeline_manager.h` (C++ for project data)
