# ClipForge Compose UI - Structure Diagram

## Application Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      Android Application                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │           ComposeActivity (Entry Point)                  │  │
│  │  - Manages all Compose screens                           │  │
│  │  - NavHost with 8 routes                                 │  │
│  │  - Theme provider (ClipForgeTheme)                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│                           │                                     │
│              ┌────────────┴────────────┐                        │
│              │                         │                        │
│      ┌───────▼────────┐      ┌────────▼──────────┐             │
│      │  Theme Layer   │      │  Navigation Layer │             │
│      │                │      │                   │             │
│      │ • Color.kt     │      │ • NavHost (8      │             │
│      │ • Type.kt      │      │   routes)         │             │
│      │ • Theme.kt     │      │ • State management│             │
│      └────────────────┘      └───────────────────┘             │
│                                       │                        │
│              ┌────────────────────────┼────────────────────┐   │
│              │                        │                    │   │
│      ┌───────▼──────────┐   ┌────────▼────────┐  ┌───────▼──┐ │
│      │   Screens Layer  │   │  Components     │  │  Models  │ │
│      │                  │   │                 │  │          │ │
│      │ • HomeScreen.kt  │   │ • ProjectCard   │  │ ProjectUI│ │
│      │ • EditorScreen   │   │ • QuickAction   │  │ FilterUI │ │
│      │ • FiltersScreen  │   │ • TabRow        │  │ Platform │ │
│      │ • ExportScreen   │   │ • Dialogs       │  │          │ │
│      │                  │   │ • Icons         │  │          │ │
│      └──────────────────┘   └─────────────────┘  └──────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Screen Hierarchy

```
┌─────────────────────────────────┐
│      ComposeActivity            │
│   (NavHost with 8 routes)       │
└──────────────┬──────────────────┘
               │
        ┌──────┴────────────────────────────────┐
        │                                       │
        │                    ┌──────────────────┼──────────────┐
        │                    │                  │              │
    ┌───▼──────────┐    ┌────▼─────┐    ┌─────▼─────┐    ┌──▼──────┐
    │ HomeScreen   │    │ Editor    │    │  Filters  │    │ Export   │
    │              │    │ Screen    │    │  Screen   │    │ Screen   │
    ├──────────────┤    ├───────────┤    ├───────────┤    ├──────────┤
    │ • Logo/Title │    │ • Preview │    │ • Preview │    │ • Title  │
    │ • Empty/Full │    │ • Controls│    │ • Grid    │    │ • Grids  │
    │ • Projects   │    │ • Tabs    │    │ • Selected│    │ • Settings
    │ • Actions    │    │ • Content │    │ • Labels  │    │          │
    │ • Dialogs    │    └───────────┘    └───────────┘    └──────────┘
    └──────────────┘
        │
        ├─ Empty State
        │  ├─ Logo/Icon
        │  ├─ Welcome Text
        │  ├─ New Project Button
        │  └─ Get Started Cards
        │
        └─ Projects State
           ├─ Top Bar
           ├─ Quick Actions
           ├─ Project Grid
           │  └─ Project Cards (2-col)
           │     ├─ Thumbnail
           │     ├─ Info
           │     └─ Menu
           └─ Dialogs
              ├─ Create Project
              └─ Delete Confirmation
```

---

## Data Flow

```
┌──────────────────────────────────────────┐
│   External Data Sources                  │
│  • Database (Room)                       │
│  • File System (Thumbnails)              │
│  • ViewModel (State)                     │
└────────────────┬─────────────────────────┘
                 │
        ┌────────▼──────────┐
        │  Data Models      │
        │  • ProjectUI      │
        │  • FilterUIModel  │
        │  • ExportPlatform │
        └────────┬──────────┘
                 │
        ┌────────▼──────────────────────┐
        │  Screen Composables           │
        │  (receive data as parameters) │
        └────────┬──────────────────────┘
                 │
        ┌────────▼──────────────────┐
        │  UI Components            │
        │  (render data)            │
        └────────┬──────────────────┘
                 │
        ┌────────▼──────────────────┐
        │  User Interactions        │
        │  • Clicks                 │
        │  • Selections             │
        │  • Inputs                 │
        └────────┬──────────────────┘
                 │
        ┌────────▼──────────────────┐
        │  Callbacks                │
        │  (call ViewModel methods) │
        └──────────────────────────┘
```

---

## Color System Hierarchy

```
┌─────────────────────────────────────────────────────┐
│            ClipForge Color Palette                  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Primary Colors (Brand)                       │  │
│  │ • BrandPurple (#7C5FD8) - Main button color │  │
│  │ • BrandPink (#FF1493) - Logo accent         │  │
│  │ • BrandPurpleDark (#6B4FC8) - Hover state  │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Background Colors                            │  │
│  │ • BackgroundDarker (#0F0B15) - Main bg      │  │
│  │ • BackgroundDark (#1A1625) - Top bars       │  │
│  │ • SurfaceDark (#2D2438) - Cards             │  │
│  │ • SurfaceDarker (#1F1A28) - Darker variant  │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Text Colors                                  │  │
│  │ • TextPrimary (#FFFFFF) - Main text         │  │
│  │ • TextSecondary (#999999) - Secondary text  │  │
│  │ • TextTertiary (#666666) - Subtle text      │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Accent Colors                                │  │
│  │ • AccentOrange (#FF9800) - Progress/Active  │  │
│  │ • AccentGold (#FFD700) - Highlights         │  │
│  │ • AccentAmber (#D97706) - Templates         │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Status Colors                                │  │
│  │ • SuccessGreen (#4CAF50) - Success          │  │
│  │ • ErrorRed (#F44336) - Error                │  │
│  │ • InfoBlue (#2196F3) - Information          │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

---

## Typography Hierarchy

```
┌─────────────────────────────────────────────────────┐
│          Material 3 Typography Scale                │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Display (Large Headings)                     │  │
│  │ • displayLarge: 57sp, Bold                   │  │
│  │ • displayMedium: 45sp, Bold                  │  │
│  │ • displaySmall: 36sp, Bold                   │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Headline (Section Titles)                    │  │
│  │ • headlineLarge: 32sp, Bold                  │  │
│  │ • headlineMedium: 28sp, Bold                 │  │
│  │ • headlineSmall: 24sp, Bold                  │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Title (Component Titles)                     │  │
│  │ • titleLarge: 22sp, Bold                     │  │
│  │ • titleMedium: 16sp, SemiBold                │  │
│  │ • titleSmall: 14sp, SemiBold                 │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Body (Main Content)                          │  │
│  │ • bodyLarge: 16sp, Regular                   │  │
│  │ • bodyMedium: 14sp, Regular                  │  │
│  │ • bodySmall: 12sp, Regular                   │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Label (Small Text)                           │  │
│  │ • labelLarge: 14sp, SemiBold                 │  │
│  │ • labelMedium: 12sp, SemiBold                │  │
│  │ • labelSmall: 11sp, SemiBold                 │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

---

## HomeScreen Component Tree

```
HomeScreen
├── Box (gradient background)
│   └── Column
│       ├── HomeTopBar
│       │   ├── Row
│       │   │   ├── Icon (logo)
│       │   │   └── Text ("ClipForge")
│       │   └── IconButton (settings)
│       │
│       ├── (if empty)
│       │   └── EmptyStateView
│       │       ├── Box (icon circle)
│       │       ├── Text (welcome)
│       │       ├── Button (new project)
│       │       ├── Row (import + record)
│       │       │   ├── QuickActionCard
│       │       │   └── QuickActionCard
│       │       └── QuickActionCard (templates)
│       │
│       └── (if has projects)
│           └── ProjectsContentView
│               ├── Row (quick actions)
│               │   ├── QuickActionCard (new)
│               │   ├── QuickActionCard (import)
│               │   ├── QuickActionCard (record)
│               │   └── QuickActionCard (templates)
│               ├── Row (header + search)
│               └── LazyVerticalGrid
│                   └── ProjectCard (repeating)
│                       ├── AsyncImage (thumbnail)
│                       ├── Box (gradient overlay)
│                       ├── Column (info)
│                       └── IconButton (menu)
│
├── CreateProjectDialog
│   ├── OutlinedTextField
│   ├── Button (create)
│   └── Button (cancel)
│
└── DeleteProjectDialog
    ├── Icon (delete)
    ├── Text (confirmation)
    ├── Button (delete)
    └── Button (cancel)
```

---

## EditorScreen Component Tree

```
EditorScreen
├── Box (gradient background)
│   └── Column
│       ├── EditorTopBar
│       │   ├── Text ("Editing Timeline")
│       │   ├── IconButton (back)
│       │   └── IconButton (more)
│       │
│       ├── Box (video preview)
│       │   └── Icon (placeholder)
│       │
│       ├── Row (controls)
│       │   ├── IconButton (play)
│       │   ├── Slider (progress)
│       │   └── Text (time)
│       │
│       ├── TabRow
│       │   ├── Tab (Effects)
│       │   ├── Tab (Main)
│       │   └── Tab (Library)
│       │
│       └── Column (scrollable content)
│           ├── (if Effects tab)
│           │   └── EffectsTabContent
│           │       └── Card (repeating) × 3
│           │
│           ├── (if Main tab)
│           │   └── MainTabContent
│           │       ├── Card (timeline)
│           │       ├── Card (layer) × 2
│           │
│           └── (if Library tab)
│               └── LibraryTabContent
│                   └── Card (item) × 4
```

---

## FiltersScreen Component Tree

```
FiltersScreen
├── Box (gradient background)
│   └── Column
│       ├── FiltersTopBar
│       │   ├── Text ("Filters")
│       │   └── IconButton (back)
│       │
│       ├── Box (video preview)
│       │   └── Icon (placeholder)
│       │
│       ├── Row (controls)
│       │   ├── IconButton (play)
│       │   ├── Slider (progress)
│       │   └── Text (time)
│       │
│       ├── Text (filter name)
│       │
│       └── LazyVerticalGrid (2 columns)
│           └── FilterCard (repeating) × 4
│               ├── AsyncImage (thumbnail)
│               ├── Icon (if no image)
│               └── Box (text overlay)
│                   └── Text (filter name)
```

---

## ExportScreen Component Tree

```
ExportScreen
├── Box (gradient background)
│   └── Column
│       ├── ExportTopBar
│       │   ├── Text ("Export")
│       │   └── IconButton (back)
│       │
│       ├── Column (scrollable)
│       │   ├── Text ("Export Your Reel")
│       │   │
│       │   ├── LazyVerticalGrid (3 columns)
│       │   │   └── ExportPlatformCard × 6
│       │   │       ├── Box (icon)
│       │   │       │   └── Icon (platform icon)
│       │   │       └── Text (platform name)
│       │   │
│       │   ├── Text ("Export Settings")
│       │   │
│       │   ├── Card (Quality)
│       │   │   └── Row (quality buttons)
│       │   │       ├── Button (HD)
│       │   │       ├── Button (Full HD)
│       │   │       └── Button (4K)
│       │   │
│       │   └── Card (Format)
│       │       └── Row (format buttons)
│       │           ├── Button (MP4)
│       │           ├── Button (MOV)
│       │           └── Button (WebM)
```

---

## Navigation Routes Map

```
┌─────────────────────────────────────┐
│        Navigation Routes            │
├─────────────────────────────────────┤
│                                     │
│  home ──────────────────────────┐   │
│  │                              │   │
│  ├─→ editor/{projectId} ──→ ┐   │   │
│  │                           │   │   │
│  ├─→ filters               │   │   │
│  │                           │   │   │
│  ├─→ export ← ← ← ← ← ← ← ← ┘   │   │
│  │                              │   │
│  ├─→ import                     │   │
│  │                              │   │
│  ├─→ record                     │   │
│  │                              │   │
│  ├─→ templates                 │   │
│  │                              │   │
│  ├─→ settings                  │   │
│  │                              │   │
│  └─ (back) ← ← ← ← ← ← ← ← ← ← ┘   │
│                                     │
└─────────────────────────────────────┘
```

---

## File Dependencies

```
ComposeActivity.kt
├─ HomeScreen.kt
│  ├─ Color.kt
│  ├─ Type.kt (via Theme.kt)
│  └─ Icons
│
├─ EditorScreen.kt
│  ├─ Color.kt
│  ├─ Type.kt (via Theme.kt)
│  └─ Icons
│
├─ FiltersScreen.kt
│  ├─ Color.kt
│  ├─ Type.kt (via Theme.kt)
│  ├─ Icons
│  └─ Coil (AsyncImage)
│
└─ ExportScreen.kt
   ├─ Color.kt
   ├─ Type.kt (via Theme.kt)
   └─ Icons

Theme.kt
├─ Color.kt
├─ Type.kt
└─ Material3
```

---

## Performance Optimization Structure

```
┌────────────────────────────────────┐
│   Performance Optimizations        │
├────────────────────────────────────┤
│                                    │
│ ✓ LazyVerticalGrid                │
│   └─ Only renders visible items    │
│                                    │
│ ✓ remember() State Management      │
│   └─ Prevents unnecessary recomp   │
│                                    │
│ ✓ AsyncImage with Coil             │
│   └─ Efficient image loading       │
│                                    │
│ ✓ rememberScrollState()            │
│   └─ Smooth scrolling              │
│                                    │
│ ✓ Modifier chaining                │
│   └─ Efficient composition         │
│                                    │
│ ✓ Material 3 optimized             │
│   └─ Latest compose libs           │
│                                    │
└────────────────────────────────────┘
```

---

## Testing Architecture

```
┌─────────────────────────────────────┐
│      Testing Structure              │
├─────────────────────────────────────┤
│                                     │
│ Unit Tests                          │
│ └─ ViewModel logic                  │
│    └─ Data transformations          │
│       └─ Navigation logic           │
│                                     │
│ Compose UI Tests                    │
│ └─ HomeScreen rendering             │
│    └─ Dialog interactions           │
│       └─ Navigation between screens │
│                                     │
│ Integration Tests                   │
│ └─ Activity lifecycle               │
│    └─ Database + UI                 │
│       └─ Navigation + State         │
│                                     │
└─────────────────────────────────────┘
```

---

## Module Dependencies

```
app/build.gradle.kts
│
├─ Compose Core
│  ├─ compose.ui:1.4.3
│  ├─ material3:1.0.1
│  ├─ foundation:1.4.3
│  └─ runtime:1.4.3
│
├─ Integration
│  ├─ activity-compose:1.7.2
│  ├─ lifecycle-viewmodel-compose:2.6.1
│  ├─ lifecycle-runtime-compose:2.6.1
│  └─ navigation-compose:2.5.3
│
└─ Extensions
   ├─ material-icons-extended:1.4.3
   └─ coil-compose:2.2.2
```

---

**Architecture Version**: 1.0
**Last Updated**: October 2025
**Status**: ✅ Complete
