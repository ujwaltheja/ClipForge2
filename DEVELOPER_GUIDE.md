# ClipForge - Developer Quick Reference Guide

**Version:** 1.0
**Last Updated:** October 29, 2025
**Framework:** Kotlin + C++ NDK
**Target API:** Android 8.0 - 15 (API 26-35)

---

## 🚀 Quick Start

### **1. Project Setup**
```bash
# Navigate to project root
cd ClipForge2

# Build with Gradle
./gradlew build

# Run on connected device
./gradlew installDebug

# Build and run tests
./gradlew test
```

### **2. Important Files**

| File | Purpose | Edit For |
|------|---------|----------|
| `app/build.gradle.kts` | Build config | Dependencies, NDK, CMake |
| `app/src/main/cpp/CMakeLists.txt` | C++ build | Native compilation |
| `AndroidManifest.xml` | App metadata | Permissions, activities |
| `res/navigation/nav_graph.xml` | Navigation | Screen flow |

---

## 📱 Android Architecture

### **Entry Point: MainActivity.kt**
```kotlin
// Handles:
// - Permission checking
// - Engine initialization
// - Navigation setup
```

### **Activities Overview**

| Activity | Purpose | Key Features |
|----------|---------|--------------|
| **MainActivity** | Home screen | Recent projects, new project |
| **ImportActivity** | Media import | Multi-tab (device/camera/library) |
| **EditorActivity** | Timeline editing | Preview, playback, clips |
| **EffectsActivity** | Effects/filters | Category filtering, preview |
| **AudioMixerActivity** | Audio mixing | Volume, mute, EQ controls |
| **ExportActivity** | Export config | Format, quality, progress |
| **SettingsActivity** | App settings | Preferences, dark mode |
| **AIAssistantActivity** | AI features | Smart editing, enhancement |

### **Adapter Pattern**

All adapters use `ListAdapter<T, VH>` with `DiffUtil`:

```kotlin
// Example: EffectsGridAdapter
class EffectsGridAdapter(
    private val onEffectClick: (Effect) -> Unit
) : ListAdapter<Effect, EffectViewHolder>(EffectDiffCallback())
```

**Key Methods:**
- `submitList(list)` - Update adapter data
- `getSelectedItem()` - Get user selection
- `clearSelection()` - Deselect all

---

## 💾 Data Models

### **MediaItem** - Imported Media
```kotlin
data class MediaItem(
    val id: String,
    val displayName: String,
    val uri: Uri,
    val duration: Long,
    val size: Long,
    val mimeType: String,
    val isVideo: Boolean
)

// Usage
val items = listOf(mediaItem1, mediaItem2)
mediaGridAdapter.submitList(items)
```

### **Effect** - Filters/Effects
```kotlin
data class Effect(
    val id: String,
    val name: String,
    val category: String,
    val description: String,
    val intensity: Float = 100f
)

// Library access
val allEffects = EffectLibrary.effects
val colorEffects = EffectLibrary.getByCategory("Color")
val searchResults = EffectLibrary.search("blur")
```

### **AudioTrack** - Audio Mixing
```kotlin
data class AudioTrack(
    val id: String,
    val name: String,
    val type: AudioTrackType,
    val volume: Float = 1.0f,
    val isMuted: Boolean = false,
    val bassGain: Float = 0f,
    val midGain: Float = 0f,
    val trebleGain: Float = 0f
)

// Presets
val voiceTrack = AudioPresets.voiceoverPreset
val musicTrack = AudioPresets.musicPreset
```

### **VideoClip** - Timeline Clips
```kotlin
data class VideoClip(
    val id: String,
    val sourceUri: String,
    val positionMs: Long,
    val durationMs: Long,
    val speed: Float = 1.0f,
    val volume: Float = 1.0f,
    val effects: List<String> = emptyList(),
    val colorGrade: ColorGrade? = null
)

// Usage
val clip = VideoClip(
    id = UUID.randomUUID().toString(),
    sourceUri = "file:///video.mp4",
    positionMs = 0L,
    durationMs = 30000L
)
```

### **Project** - Project Management
```kotlin
data class Project(
    val id: String,
    val name: String,
    val videoClips: List<VideoClip> = emptyList(),
    val audioTracks: List<AudioTrack> = emptyList(),
    val metadata: ProjectMetadata
)

// Templates
val template = ProjectTemplates.templates[0]  // "1080p 30fps"
```

---

## 🔌 JNI Integration

### **Calling Native Methods**

```kotlin
// In Activity/ViewModel
val enginePtr = NativeLib.createEngine()
val initialized = NativeLib.initEngine(enginePtr)

// Add clip
NativeLib.addClip(
    enginePtr = enginePtr,
    sourcePath = "/path/to/video.mp4",
    startPosition = 0L,
    trackIndex = 0
)

// Apply effect
NativeLib.applyEffect(
    enginePtr = enginePtr,
    clipId = clipId,
    effectName = "vintage"
)

// Start export
NativeLib.startExport(
    enginePtr = enginePtr,
    outputPath = "/output/video.mp4",
    format = "mp4",
    quality = "high"
)

// Cleanup
NativeLib.destroyEngine(enginePtr)
```

### **NativeLib Methods Reference**

**Engine Lifecycle:**
- `createEngine()` → Long (engine pointer)
- `initEngine(enginePtr: Long)` → Boolean
- `destroyEngine(enginePtr: Long)` → Unit
- `getEngineVersion(enginePtr: Long)` → String

**Clip Operations:**
- `addClip(enginePtr, sourcePath, startPos, trackIndex)` → String (clipId)
- `removeClip(enginePtr, clipId)` → Boolean
- `moveClip(enginePtr, clipId, newPos, newTrack)` → Boolean
- `trimClip(enginePtr, clipId, trimStart, trimEnd)` → Boolean
- `setClipSpeed(enginePtr, clipId, speed)` → Boolean
- `setClipVolume(enginePtr, clipId, volume)` → Boolean
- `splitClip(enginePtr, clipId, splitTime)` → String (new clipId)

**Effects:**
- `applyEffect(enginePtr, clipId, effectName)` → Boolean
- `removeEffect(enginePtr, clipId, effectId)` → Boolean

**Audio:**
- `addAudioTrack(enginePtr, name, type)` → String (trackId)
- `removeAudioTrack(enginePtr, trackId)` → Boolean
- `setAudioTrackVolume(enginePtr, trackId, volume)` → Boolean
- `setAudioTrackMuted(enginePtr, trackId, muted)` → Boolean

**Preview:**
- `startPreview(enginePtr)` → Boolean
- `pausePreview(enginePtr)` → Boolean
- `stopPreview(enginePtr)` → Boolean
- `seekPreview(enginePtr, timeMs)` → Unit
- `getPreviewPosition(enginePtr)` → Long (milliseconds)
- `isPreviewPlaying(enginePtr)` → Boolean

**Export:**
- `startExport(enginePtr, path, format, quality)` → Boolean
- `cancelExport(enginePtr)` → Boolean
- `getExportProgress(enginePtr)` → Float (0-100)
- `isExporting(enginePtr)` → Boolean

---

## 📐 UI Resources

### **Colors**
Location: `res/values/colors.xml`

```xml
<!-- Brand Colors -->
<color name="brand_purple">#7C5FD8</color>
<color name="brand_purple_dark">#6B4FC8</color>
<color name="brand_purple_light">#8B7DE8</color>

<!-- Backgrounds -->
<color name="background_dark">#1A1625</color>
<color name="background_darker">#0F0B15</color>

<!-- Text -->
<color name="text_primary">#FFFFFF</color>
<color name="text_secondary">#999999</color>
<color name="text_tertiary">#666666</color>

<!-- Status -->
<color name="success">#4CAF50</color>
<color name="error">#F44336</color>
```

### **Dimensions**
Location: `res/values/dimens.xml`

```xml
<!-- Padding -->
<dimen name="padding_small">8dp</dimen>
<dimen name="padding_medium">16dp</dimen>
<dimen name="padding_large">24dp</dimen>

<!-- Buttons -->
<dimen name="button_height">48dp</dimen>

<!-- Icons -->
<dimen name="icon_medium">24dp</dimen>
<dimen name="icon_large">32dp</dimen>

<!-- Text -->
<dimen name="text_body_large">16sp</dimen>
<dimen name="text_label_small">11sp</dimen>
```

### **Strings**
Location: `res/values/strings.xml`

```xml
<!-- Main -->
<string name="app_name">ClipForge</string>
<string name="import_media">Import Media</string>
<string name="editing_timeline">Editing Timeline</string>
<string name="export_your_reel">Export Your Reel</string>
```

---

## 🔄 Common Workflows

### **Import Media Workflow**
```kotlin
// 1. User selects media from ImportActivity
val selectedMedia = mediaGridAdapter.getSelectedItems()

// 2. For each item, add to timeline
for (item in selectedMedia) {
    val clipId = NativeLib.addClip(
        enginePtr,
        item.uri.toString(),
        positionMs = 0L,
        trackIndex = 0
    )
}

// 3. Refresh timeline display
updateTimelineUI()
```

### **Apply Effect Workflow**
```kotlin
// 1. User selects effect
val selectedEffect = effectsAdapter.getSelectedEffect()

// 2. Apply to clip
NativeLib.applyEffect(
    enginePtr,
    selectedClipId,
    selectedEffect.id
)

// 3. Show preview (real-time)
startPreviewUpdate()
```

### **Mix Audio Workflow**
```kotlin
// 1. User adjusts volume slider
binding.trackVolume.setOnSeekBarChangeListener { progress ->
    val volume = progress / 100f

    // 2. Update native engine
    NativeLib.setAudioTrackVolume(
        enginePtr,
        trackId,
        volume
    )
}
```

### **Export Workflow**
```kotlin
// 1. User configures export settings
val format = "mp4"  // or "webm"
val quality = "high"  // low, medium, high, ultra
val outputPath = "/path/to/output.mp4"

// 2. Start export
NativeLib.startExport(enginePtr, outputPath, format, quality)

// 3. Monitor progress
lifecycleScope.launch {
    while (NativeLib.isExporting(enginePtr)) {
        val progress = NativeLib.getExportProgress(enginePtr)
        updateProgressBar(progress)
        delay(500)
    }
}
```

---

## 🛠️ Development Tips

### **Logging**
```kotlin
import timber.log.Timber

// Setup (in Application or MainActivity)
if (BuildConfig.DEBUG) {
    Timber.plant(Timber.DebugTree())
}

// Usage
Timber.d("Debug message")
Timber.e(exception, "Error message")
Timber.i("Info message")
```

### **Coroutines**
```kotlin
// In Activity/Fragment
lifecycleScope.launch {
    try {
        // Your async code
        val result = NativeLib.startExport(...)
    } catch (e: Exception) {
        Timber.e(e, "Error")
    }
}
```

### **View Binding**
```kotlin
// Setup
private lateinit var binding: ActivityMainBinding

override fun onCreate(savedInstanceState: Bundle?) {
    binding = ActivityMainBinding.inflate(layoutInflater)
    setContentView(binding.root)
}

// Usage
binding.btnNewProject.setOnClickListener {
    startActivity(Intent(this, ImportActivity::class.java))
}
```

### **SharedPreferences**
```kotlin
// Save
val prefs = getSharedPreferences("clipforge_prefs", MODE_PRIVATE)
prefs.edit().putString("key", "value").apply()

// Load
val value = prefs.getString("key", "default")
```

---

## 📋 Testing

### **Unit Tests**
```kotlin
// Test model
@Test
fun testVideoClipDuration() {
    val clip = VideoClip(
        id = "test",
        sourceUri = "file.mp4",
        positionMs = 0,
        durationMs = 30000,
        speed = 2.0f
    )
    assertEquals(15000, clip.getPlaybackDuration())
}
```

### **UI Tests**
```kotlin
// Test activity
@get:Rule
val activityRule = ActivityScenarioRule(MainActivity::class.java)

@Test
fun testMainActivityLaunches() {
    activityRule.scenario.onActivity { activity ->
        assertNotNull(activity)
    }
}
```

---

## 🐛 Debugging

### **Debug Breakpoints**
1. Click line number in IDE to add breakpoint
2. Run in debug mode: `./gradlew installDebugApk`
3. Android Studio debugger panel shows variables

### **Logcat Filtering**
```bash
# Filter by tag
adb logcat | grep "ClipForge"

# Filter by level
adb logcat | grep "E:"  # Errors only

# Clear logs
adb logcat -c
```

### **Common Issues**

| Issue | Solution |
|-------|----------|
| Compilation error | Clean: `./gradlew clean` |
| APK won't install | Uninstall: `adb uninstall com.ucworks.clipforge` |
| Native crash | Check logcat for stack trace |
| Permission denied | Check AndroidManifest.xml permissions |
| Memory leak | Use Android Profiler in Android Studio |

---

## 📚 Additional Resources

- **Android Documentation:** https://developer.android.com/
- **Kotlin Docs:** https://kotlinlang.org/docs/
- **NDK Guide:** https://developer.android.com/ndk
- **Material Design 3:** https://m3.material.io/
- **Jetpack Libraries:** https://developer.android.com/jetpack

---

## 📞 Support

For issues or questions:
1. Check the logcat output
2. Review relevant documentation files
3. Check code comments (KDoc)
4. Search Android Stack Overflow

---

**Happy Coding! 🚀**

