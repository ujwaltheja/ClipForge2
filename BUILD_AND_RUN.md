# ClipForge Jetpack Compose UI - Build & Run Guide

## Prerequisites

- Android Studio Arctic Fox (2021.1.1) or later
- Android SDK 33 (target API level)
- Android SDK 24 (minimum API level)
- Gradle 8.0 or later
- Java 11 or later

---

## Step 1: Verify Gradle Configuration

The gradle files have been updated with Compose dependencies and the required Kotlin Compose Compiler plugin.

**Gradle Updates Applied**:
- ✅ Added `org.jetbrains.kotlin.plugin.compose` to root build.gradle.kts
- ✅ Applied Compose plugin in app/build.gradle.kts
- ✅ Updated `kotlinOptions` to modern `kotlin.compilerOptions` format
- ✅ Removed deprecated `composeOptions` block (handled by plugin now)

These changes ensure compatibility with Kotlin 2.0+ and Compose.

**See**: [BUILD_FIX_GUIDE.md](BUILD_FIX_GUIDE.md) for detailed changes

---

## Step 2: Sync Project with Gradle Files

1. **In Android Studio**:
   - File → Sync Now
   - Or use: `./gradlew sync`

2. **Wait for**:
   - Gradle to download dependencies
   - Kotlin compiler plugin to install
   - IDE indexing to complete

**Expected time**: 2-5 minutes on first build

---

## Step 3: Update AndroidManifest.xml

Change the main activity from `MainActivity` to `ComposeActivity`:

**File**: `app/src/main/AndroidManifest.xml`

```xml
<activity
    android:name=".ComposeActivity"
    android:label="@string/app_name"
    android:exported="true">
    <intent-filter>
        <action android:name="android.intent.action.MAIN" />
        <category android:name="android.intent.category.LAUNCHER" />
    </intent-filter>
</activity>
```

**Optional**: Keep MainActivity but add explicit launcher:
```xml
<!-- Original MainActivity (for compatibility) -->
<activity
    android:name=".MainActivity"
    android:exported="false" />

<!-- New ComposeActivity (main launcher) -->
<activity
    android:name=".ComposeActivity"
    android:label="@string/app_name"
    android:exported="true">
    <intent-filter>
        <action android:name="android.intent.action.MAIN" />
        <category android:name="android.intent.category.LAUNCHER" />
    </intent-filter>
</activity>
```

---

## Step 4: Clean and Build

### Option A: Command Line

```bash
# Navigate to project root
cd d:\Github\Android\ClipForge2

# Clean old build artifacts
./gradlew clean

# Build debug APK
./gradlew assembleDebug

# Or build and run on connected device
./gradlew installDebug
```

### Option B: Android Studio

1. **Build** menu → Clean Project
2. **Build** menu → Rebuild Project
3. Wait for build to complete

**Expected build time**: 3-5 minutes

---

## Step 5: Run on Emulator or Device

### Option A: Android Studio

1. **Select device**:
   - Tools → Device Manager (or use device dropdown)
   - Create emulator or connect physical device

2. **Run**:
   - Run → Run 'app' (or press Shift+F10)
   - Select your device

3. **Wait for app to launch**:
   - Emulator startup: 30-60 seconds
   - Device: immediate

### Option B: Command Line

```bash
# List connected devices
./gradlew devices

# Install and run on device
./gradlew installDebug
adb shell am start -n com.ucworks.clipforge/.ComposeActivity

# Or use emulator
emulator -avd Pixel_4_API_33 &
./gradlew installDebugAndroidTest
```

---

## Step 6: Verify Installation

**Check device/emulator**:
1. App should launch with ClipForge logo
2. Home screen displays "Welcome to ClipForge"
3. "Get Started" section visible with 3 action cards
4. No crashes or errors in Logcat

**Logcat output should show**:
```
I/Choreographer: Skipped 5 frames!  The application may be doing too much work on its main thread.
I/System: bindApplication pid=12345
I/MainActivity: Starting ClipForge Compose UI
```

---

## Troubleshooting

### Build Fails: "Kotlin compiler plugin not installed"

**Solution**:
```bash
./gradlew clean
./gradlew --refresh-dependencies
./gradlew build
```

### Build Fails: "Compose annotation processor failed"

**Solution**:
1. File → Invalidate Caches
2. Restart Android Studio
3. Rebuild project

### App Crashes on Launch

**Check Logcat for errors**:
```bash
adb logcat -s AndroidRuntime
```

**Common issues**:
- Missing resources: Run `./gradlew clean`
- Missing dependencies: Sync Gradle
- Wrong activity: Check AndroidManifest.xml

### Preview Not Showing in Android Studio

**Solution**:
1. Open any `.kt` file with `@Composable` function
2. Click "Preview" button (top right of editor)
3. Wait for preview to render

**If still not working**:
- Build → Refresh Linked C++ Projects
- File → Invalidate Caches → Restart

---

## Gradle Commands Cheat Sheet

```bash
# Build commands
./gradlew build              # Full build
./gradlew clean              # Clean build artifacts
./gradlew assembleDebug      # Build debug APK
./gradlew assembleRelease    # Build release APK
./gradlew installDebug       # Install debug APK

# Testing commands
./gradlew test               # Run unit tests
./gradlew connectedAndroidTest  # Run instrumented tests

# Development commands
./gradlew --refresh-dependencies  # Refresh dependencies
./gradlew lint              # Run Android lint
./gradlew projectHealth     # Check project health
./gradlew dependencies      # Print dependency tree

# Information commands
./gradlew tasks             # List all tasks
./gradlew properties        # Print project properties
./gradlew -v                # Verbose output
```

---

## Running on Physical Device

### Prerequisites
1. Enable USB Debugging on device:
   - Settings → Developer Options → USB Debugging
2. Connect device via USB cable
3. Accept the "Allow USB Debugging" dialog

### Run

```bash
# List devices
adb devices

# You should see:
# List of attached devices
# emulator-5554          device
# FA76H1A00342           device

# Install and run
./gradlew installDebug
adb shell am start -n com.ucworks.clipforge/.ComposeActivity
```

---

## Running on Emulator

### Create Emulator (if needed)

1. **Tools** → **Device Manager**
2. **Create Device** button
3. Select device type: "Pixel 4" or similar
4. Select API 33 (Android 13)
5. Start the emulator

### Run

```bash
# List emulators
emulator -list-avds

# Start emulator
emulator -avd Pixel_4_API_33 &

# Run app
./gradlew installDebugAndroidTest
adb shell am start -n com.ucworks.clipforge/.ComposeActivity
```

---

## Performance Optimization

### Release Build (APK for testing)

```bash
# Generate release APK
./gradlew assembleRelease

# Output location
# app/build/outputs/apk/release/app-release.apk
```

### Size Optimization

```bash
# Check APK size
./gradlew bundleRelease
# Output: app/build/outputs/bundle/release/app-release.aab

# View bundle analysis
bundletool build-apks \
  --bundle=app/build/outputs/bundle/release/app-release.aab \
  --output=app/build/outputs/app.apks \
  --mode=universal
```

---

## Common Build Issues & Solutions

### Issue: "Program type already present"

**Cause**: Duplicate class definitions
**Solution**:
```bash
./gradlew clean --no-build-cache
./gradlew build
```

### Issue: "Execution failed for task ':app:composeMultiPlatformCodeGenDebug'"

**Cause**: Kotlin compiler version mismatch
**Solution**:
```gradle
// In app/build.gradle.kts
composeOptions {
    kotlinCompilerExtensionVersion = "1.4.3"  // Match Kotlin version
}
```

### Issue: "Gradle DSL method not found: 'compose()'"

**Cause**: Old Gradle version
**Solution**:
```bash
# Update Gradle
./gradlew wrapper --gradle-version=8.0

# Or update in gradle/wrapper/gradle-wrapper.properties
distributionUrl=https\://services.gradle.org/distributions/gradle-8.0-bin.zip
```

### Issue: "Can't find cached directory"

**Cause**: Gradle cache corrupted
**Solution**:
```bash
# Clear Gradle cache
rm -rf ~/.gradle/caches
./gradlew clean
./gradlew build
```

### Issue: App crashes with "WindowManager.BadTokenException"

**Cause**: Theme or context issue
**Solution**:
- Verify theme is applied in ComposeActivity
- Check AndroidManifest theme definition
- Ensure no duplicate theme declarations

---

## Debugging

### Enable Verbose Logging

```bash
# Build with verbose output
./gradlew build -v

# Run with ADB debugging
adb logcat -v threadtime | grep -E "(E/|I/ClipForge)"
```

### Debug APK Installation

```bash
# Uninstall previous
adb uninstall com.ucworks.clipforge

# Install with output
adb install -r -g app/build/outputs/apk/debug/app-debug.apk

# Check installation
adb shell pm list packages | grep clipforge
```

### Monitor App Performance

```bash
# CPU profiling
adb shell am profile start --sampling 1000 com.ucworks.clipforge

# Memory profiling
adb shell dumpsys meminfo com.ucworks.clipforge

# Frame profiling
adb shell dumpsys gfxinfo com.ucworks.clipforge
```

---

## IDE Setup Tips

### Android Studio Preferences

**Settings** → **Languages & Frameworks** → **Kotlin**:
```
Kotlin compiler version: 1.8.0+
IDE plugin version: Latest
```

**Settings** → **Languages & Frameworks** → **Android**:
```
Android SDK: API 33
Target SDK: 33
Min SDK: 24
```

### Enable Compose Preview

**Settings** → **Experimental** → **Enable Live Edit for Compose Preview**

### Memory Settings

If build is slow, increase memory:

In `gradle.properties`:
```properties
org.gradle.jvmargs=-Xmx2048m -XX:+UseG1GC
```

---

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Set up Java
        uses: actions/setup-java@v3
        with:
          java-version: '11'

      - name: Build
        run: ./gradlew build

      - name: Upload APK
        uses: actions/upload-artifact@v3
        with:
          name: app-debug.apk
          path: app/build/outputs/apk/debug/app-debug.apk
```

---

## Verification Checklist

After successful build and run:

- [ ] App launches without crashes
- [ ] Home screen shows "Welcome to ClipForge"
- [ ] All 4 quick action cards visible
- [ ] Navigation between screens works
- [ ] Dialogs appear and disappear
- [ ] Colors match design (purple buttons, orange progress)
- [ ] Typography displays correctly
- [ ] Images load properly (if any)
- [ ] No ANRs (Application Not Responding)
- [ ] No memory leaks (check Logcat)

---

## First Run Checklist

```bash
# 1. Clean project
./gradlew clean

# 2. Sync dependencies
./gradlew sync

# 3. Build debug APK
./gradlew assembleDebug

# 4. Install on device
adb install -r app/build/outputs/apk/debug/app-debug.apk

# 5. Launch app
adb shell am start -n com.ucworks.clipforge/.ComposeActivity

# 6. Monitor logs
adb logcat -s ClipForge
```

---

## Performance Baseline

Expected performance on modern device:

| Metric | Expected |
|--------|----------|
| Build time | 3-5 minutes (first), <1 min (incremental) |
| App startup | <2 seconds |
| Home screen render | <500ms |
| Grid scroll | 60 FPS |
| Dialog open | <200ms |
| Navigation | <300ms |
| APK size | 20-30 MB (debug), 10-15 MB (release) |

---

## Support Resources

### Android Documentation
- [Jetpack Compose](https://developer.android.com/jetpack/compose)
- [Material 3](https://m3.material.io/)
- [Navigation Compose](https://developer.android.com/jetpack/compose/navigation)

### ClipForge Documentation
- `COMPOSE_QUICK_START.md` - Getting started
- `UI_COMPOSE_IMPLEMENTATION.md` - Detailed specs
- `UI_REFERENCE_MAPPING.md` - Design verification

### Troubleshooting
- Check Logcat for errors
- Review build logs: `./gradlew build --stacktrace`
- Search Android docs for specific error codes

---

## Next Steps After Build

1. ✅ Build and run successfully
2. ✅ Verify all screens render
3. ⏭️ Connect to ViewModel and database
4. ⏭️ Implement remaining placeholder screens
5. ⏭️ Add animations and polish
6. ⏭️ Run tests
7. ⏭️ Build release APK

---

## Quick Reference

```bash
# One-liner: Clean, build, and install
./gradlew clean assembleDebug && adb install -r app/build/outputs/apk/debug/app-debug.apk && adb shell am start -n com.ucworks.clipforge/.ComposeActivity

# Watch build output
./gradlew build --console=rich

# Parallel build (faster)
./gradlew build -x test --parallel

# Skip tests
./gradlew assembleDebug -x test

# Full verbose output
./gradlew build --stacktrace --debug
```

---

**Last Updated**: October 2025
**Status**: ✅ Complete
**Build System**: Gradle 8.0+
**Target SDK**: 33
**Min SDK**: 24

---

## Support

If you encounter issues:

1. Check this guide for your specific error
2. Review the troubleshooting section
3. Check Android/Gradle documentation
4. Review app/build.gradle.kts for dependency versions
5. Rebuild from clean: `./gradlew clean build`

**Happy building! 🚀**
