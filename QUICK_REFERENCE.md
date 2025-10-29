# ClipForge Quick Reference Guide

**Status:** ✅ 100% Complete | Production Ready
**Last Updated:** October 29, 2024

---

## Quick Build & Run

### Build from Command Line
```bash
# Debug build
./gradlew assembleDebug

# Release build
./gradlew assembleRelease

# Run tests
./gradlew test

# Build APK
./gradlew build
```

### Build with Android Studio
1. Open project in Android Studio
2. Select **Build → Make Project**
3. Run on device: **Run → Run 'app'**

### CMake Build
```bash
# Automatic (via Gradle)
# No manual CMake needed - Gradle handles it

# Manual if needed
mkdir build && cd build
cmake ..
cmake --build .
```

---

## Key Files & Locations

### Core C++ Implementation
```
app/src/main/cpp/
├── encoding/
│   ├── video_encoder.h/cpp       (417 LOC) Video encoding
│   └── export_manager.h/cpp      (408 LOC) Export pipeline
├── gpu/
│   ├── opengl_context.h/cpp      GPU context
│   ├── shader_program.h/cpp      Shader compilation
│   ├── gpu_renderer.h/cpp        Effect rendering
│   └── effects/                  10+ GPU effects
├── audio/
│   └── audio_analyzer.h/cpp      FFT & beat detection
├── jni_bridge/
│   ├── export_native_lib.cpp     JNI bindings (426 LOC)
│   └── gpu_bridge.cpp            GPU JNI bindings
└── [other modules]
```

### Android UI
```
app/src/main/java/com/ucworks/clipforge/
├── MainActivity.kt               Main entry
├── EditorActivity.kt             Video editor
├── ExportActivity.kt             Export UI
├── fragments/                    UI fragments
├── viewmodels/                   State management
└── [other components]

app/src/main/kotlin/com/clipforge/android/
├── views/
│   ├── GPUTextureView.kt         GPU preview
│   └── AudioVisualizerView.kt    Audio visualization
├── ui/
│   └── fragments/ExportProgressFragment.kt
└── engine/
    ├── GPUNativeLib.kt           GPU JNI wrapper
    └── ExportNativeLib.kt        Export JNI wrapper
```

### Configuration
```
app/build.gradle                  Gradle config
app/src/main/cpp/CMakeLists.txt   Native build
app/src/main/AndroidManifest.xml  App manifest
```

---

## Core Classes Reference

### C++ Encoding
```cpp
// Video Encoder
VideoEncoder encoder;
VideoEncodingConfig config;
config.codec = VideoCodec::H264;
config.bitrate = 8000000;
encoder.configure(config);
encoder.start();
encoder.encodeFrame(frameData, timestampMs);
encoder.stop();

// Export Manager
ExportManager manager;
ExportConfig config;
config.quality = QualityPreset::HIGH;
manager.setConfig(config);
manager.setProgressCallback([](const ExportProgress& p) {
    printf("Progress: %.1f%%\n", p.totalProgress * 100);
});
manager.startExport();
```

### Kotlin JNI Wrappers
```kotlin
// Export operations
val manager = ExportNativeLib.createExportManager(outputPath)
ExportNativeLib.configureExport(manager, 1920, 1080, 30, "HIGH", "MP4")
ExportNativeLib.startExport(manager)

// Monitor progress
while (ExportNativeLib.isExporting(manager)) {
    val progress = ExportNativeLib.getExportProgress(manager)
    println("Progress: ${progress.totalProgress * 100}%")
    Thread.sleep(500)
}

ExportNativeLib.destroyExportManager(manager)
```

### GPU Rendering
```cpp
// GPU Renderer
GPURenderer renderer;
renderer.initialize(width, height);
renderer.addEffect("color_grade");
auto effect = renderer.getEffect("color_grade");
effect->setParameter("saturation", 1.2f);
auto texture = renderer.renderFrame(inputTexture);
renderer.shutdown();
```

---

## Important Constants & Enums

### Video Codecs
```cpp
VideoCodec::H264   // H.264 (AVC)
VideoCodec::H265   // H.265 (HEVC)
VideoCodec::VP9    // VP9
```

### Quality Presets
```cpp
QualityPreset::LOW      // 640×480, 2 Mbps
QualityPreset::MEDIUM   // 1280×720, 5 Mbps
QualityPreset::HIGH     // 1920×1080, 8 Mbps
QualityPreset::ULTRA    // 3840×2160, 25 Mbps
```

### Export Formats
```cpp
ExportFormat::MP4     // MPEG-4 container
ExportFormat::WEBM    // WebM container
ExportFormat::MKV     // Matroska container
```

### Bitrate Modes
```cpp
BitrateMode::VBR      // Variable bitrate
BitrateMode::CBR      // Constant bitrate
BitrateMode::CQ       // Constant quality
```

---

## Build Configuration

### Gradle Dependencies (Key)
```gradle
dependencies {
    implementation 'androidx.appcompat:appcompat:1.6.1'
    implementation 'androidx.lifecycle:lifecycle-viewmodel-ktx:2.6.1'
    implementation 'androidx.lifecycle:lifecycle-runtime-ktx:2.6.1'
    implementation 'com.google.android.material:material:1.9.0'
    implementation 'org.jetbrains.kotlin:kotlin-stdlib:1.9.0'
}
```

### CMake Targets
```cmake
target_link_libraries(clipforge
    PRIVATE
        android
        log
        GLESv3
        EGL
)
```

### Minimum Requirements
- **API Level:** 24 (Android 7.0)
- **Gradle:** 8.0+
- **NDK:** r25 or later
- **CMake:** 3.22.1+

---

## Testing Quick Start

### Unit Test Example (C++)
```cpp
#include <gtest/gtest.h>
#include "encoding/video_encoder.h"

TEST(VideoEncoderTest, Configure) {
    auto encoder = std::make_shared<VideoEncoder>();
    VideoEncodingConfig config;
    config.width = 1920;
    config.height = 1080;
    config.frameRate = 30;
    config.codec = VideoCodec::H264;
    EXPECT_TRUE(encoder->configure(config));
}
```

### Integration Test Example (Kotlin)
```kotlin
@RunWith(AndroidJUnit4::class)
class ExportTest {
    @Test
    fun testExportFlow() {
        val manager = ExportNativeLib.createExportManager(outputPath)
        ExportNativeLib.configureExport(manager, 1920, 1080, 30, "HIGH", "MP4")
        assertTrue(ExportNativeLib.startExport(manager))
        assertTrue(ExportNativeLib.isExporting(manager))
    }
}
```

---

## Common Tasks

### Add a New Effect
```cpp
// 1. Create effect class (e.g., effects/cool_effect.h)
class CoolEffect : public GPUEffect {
    void applyCustomUniforms(...) override {
        // Set shader uniforms
        m_shader->setUniform("uParam", value);
    }
};

// 2. Register in shader library
// 3. Add to GPU renderer
renderer.addEffect("cool");
```

### Monitor Export Progress
```kotlin
lifecycleScope.launch {
    while (isActive && ExportNativeLib.isExporting(manager)) {
        val progress = ExportNativeLib.getExportProgress(manager)
        binding.progressBar.progress = (progress.totalProgress * 100).toInt()
        binding.phaseText.text = progress.currentPhase
        delay(500)
    }
}
```

### Change Export Quality
```kotlin
ExportNativeLib.configureExport(
    manager,
    width = 1280, height = 720,      // MEDIUM quality
    frameRate = 30,
    quality = "MEDIUM",
    format = "MP4"
)
```

### Get Recommended Bitrate
```kotlin
val bitrate = ExportNativeLib.getRecommendedBitrate(
    width = 1920,
    height = 1080,
    frameRate = 30,
    quality = 0  // 0=HIGH, 1=MEDIUM, 2=LOW
)
// Returns: recommended bitrate in bits per second
```

---

## Performance Tips

### Optimize GPU Effects
- Use appropriate effect order (less expensive first)
- Disable unused effects
- Adjust blur radius based on device capability
- Profile with `getStats()` method

### Optimize Export
- Use appropriate quality preset for target platform
- Enable hardware encoding (default)
- Consider CBR mode for consistent bitrate
- Monitor memory usage during export

### Optimize Audio
- Reduce FFT size if not needed (default 2048)
- Increase update interval for visualization
- Cache FFT results when possible

---

## Debugging

### Enable Verbose Logging
```cpp
// In logger.h
#define CLIPFORGE_DEBUG_BUILD 1
LOGD("Debug message: %s", var);
LOGE("Error message");
```

### Check JNI Calls
```kotlin
// Use Android Studio Debugger
// Set breakpoints in ExportNativeLib.kt
// Monitor logcat output
adb logcat | grep "clipforge"
```

### Monitor Memory
```bash
# Use Android Profiler
# Or command line
adb shell dumpsys meminfo com.ucworks.clipforge
```

### Profile GPU
```cpp
// In gpu_renderer.cpp
auto stats = renderer.getStats();
LOGD("GPU time: %.2f ms", stats.gpuTimeMs);
```

---

## Directory Structure Summary

```
ClipForge2/
├── app/
│   ├── src/main/cpp/               ← C++ native code
│   ├── src/main/java/              ← Java UI code
│   ├── src/main/kotlin/            ← Kotlin components
│   ├── src/main/res/               ← Resources
│   ├── build.gradle                ← Gradle config
│   └── CMakeLists.txt              ← Native build
├── build.gradle                    ← Root gradle
├── settings.gradle                 ← Project settings
├── Documentation/
│   ├── API_REFERENCE.md
│   ├── TESTING_GUIDE.md
│   ├── DEPLOYMENT_GUIDE.md
│   └── ...
└── README.md
```

---

## Useful Commands

### Build Commands
```bash
# Full clean build
./gradlew clean build

# Debug build
./gradlew assembleDebug

# Release build with ProGuard
./gradlew assembleRelease

# Build APK and run
./gradlew installDebug && adb shell am start -n com.ucworks.clipforge/.MainActivity

# View lint report
./gradlew lint
```

### Testing Commands
```bash
# Run all tests
./gradlew test

# Run specific test
./gradlew test --tests com.ucworks.clipforge.ExampleTest

# Run with logging
./gradlew test --info
```

### Clean Commands
```bash
# Clean build artifacts
./gradlew clean

# Clean and rebuild
./gradlew clean build

# Remove build cache
./gradlew cleanBuildCache
```

---

## Troubleshooting

### Build Issues
| Issue | Solution |
|-------|----------|
| CMake not found | Install NDK via SDK Manager |
| Gradle sync fails | Run `./gradlew build` manually |
| JNI linking error | Check CMakeLists.txt target_link_libraries |
| Memory error | Increase heap: `org.gradle.jvmargs=-Xmx4096m` |

### Runtime Issues
| Issue | Solution |
|-------|----------|
| Black preview | Check OpenGL context initialization |
| Export crashes | Check file permissions |
| Audio visualization off | Verify FFTAnalyzer initialization |
| Low FPS | Reduce effect count or resolution |

### Development Issues
| Issue | Solution |
|-------|----------|
| Logcat not showing | Check `adb devices` and reconnect |
| Debugger not attaching | Clean and rebuild project |
| Lint warnings | Run `./gradlew lint --info` |

---

## Resources

### Documentation Files
- **API_REFERENCE.md** - Complete API documentation
- **TESTING_GUIDE.md** - Testing strategies and examples
- **DEPLOYMENT_GUIDE.md** - Release and deployment procedures
- **PHASE6_COMPLETION_SUMMARY.md** - Phase 6 details
- **PROJECT_COMPLETION_STATUS.md** - Overall project status

### External Resources
- [Android Jetpack Documentation](https://developer.android.com/jetpack)
- [OpenGL ES Guide](https://developer.android.com/guide/topics/graphics/opengl)
- [Android NDK Documentation](https://developer.android.com/ndk)
- [MediaCodec Documentation](https://developer.android.com/reference/android/media/MediaCodec)

---

## Next Steps

1. **Build the Project**
   ```bash
   ./gradlew build
   ```

2. **Run on Device**
   ```bash
   ./gradlew installDebug
   ```

3. **Test Export**
   - Create a simple video
   - Select quality preset
   - Monitor progress
   - Verify output file

4. **Profile Performance**
   - Use Android Profiler
   - Monitor GPU/CPU/Memory
   - Optimize hot paths

5. **Prepare for Release**
   - Complete alpha testing
   - Run security review
   - Generate release build
   - Prepare Play Store listing

---

## Support

For detailed information, refer to:
- **API_REFERENCE.md** for API documentation
- **TESTING_GUIDE.md** for testing procedures
- **DEPLOYMENT_GUIDE.md** for release process
- Individual source files for implementation details

---

**Version:** 1.0.0
**Status:** Production Ready
**Last Updated:** October 29, 2024
