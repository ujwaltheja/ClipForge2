# ClipForge Testing Guide

Comprehensive testing strategy and procedures for ClipForge Video Editor

---

## Testing Overview

### Test Categories

1. **Unit Tests** - Individual component testing
2. **Integration Tests** - Component interaction testing
3. **Performance Tests** - Speed and resource usage
4. **Device Tests** - Real device compatibility
5. **UI Tests** - User interface and interaction
6. **Stress Tests** - Extreme condition handling

---

## Unit Tests

### C++ Component Tests

#### GPU System Tests
```cpp
class GPUTests : public ::testing::Test {
public:
    void SetUp() override {
        context_ = std::make_unique<OpenGLContext>(1920, 1080);
        ASSERT_TRUE(context_->initialize());
    }

    std::unique_ptr<OpenGLContext> context_;
};

TEST_F(GPUTests, ShaderCompilation) {
    ShaderSource source{vertexCode, fragmentCode, "", "TestShader"};
    auto shader = std::make_shared<ShaderProgram>();
    ASSERT_TRUE(shader->compile(source));
    EXPECT_TRUE(shader->isValid());
}

TEST_F(GPUTests, FramebufferCreation) {
    GLuint fbo = context_->createFramebuffer(1920, 1080, GL_RGBA);
    ASSERT_NE(fbo, 0);
    context_->deleteFramebuffer(fbo);
}

TEST_F(GPUTests, TextureManagement) {
    uint8_t data[1920 * 1080 * 4];
    GLuint texture = context_->createTexture(1920, 1080, data, GL_RGBA);
    ASSERT_NE(texture, 0);
    context_->deleteTexture(texture);
}

TEST_F(GPUTests, EffectApplication) {
    auto effect = std::make_shared<ColorGradeEffect>();
    ASSERT_TRUE(effect->isAvailable());
    effect->setParameter("intensity", 0.8f);
    EXPECT_FLOAT_EQ(effect->getParameter("intensity"), 0.8f);
}
```

#### Audio Analysis Tests
```cpp
class AudioTests : public ::testing::Test {
public:
    void SetUp() override {
        analyzer_ = std::make_unique<FFTAnalyzer>(2048, 44100);
    }

    std::unique_ptr<FFTAnalyzer> analyzer_;
};

TEST_F(AudioTests, FFTAnalysis) {
    std::vector<float> samples(2048);
    // Fill with test data (sine wave)
    for (int i = 0; i < 2048; i++) {
        samples[i] = sin(2 * M_PI * 440 * i / 44100.0f);
    }

    auto spectrum = analyzer_->analyze(samples);
    EXPECT_EQ(spectrum.magnitudes.size(), 1024);
    EXPECT_GT(spectrum.peakMagnitude, 0.0f);
}

TEST_F(AudioTests, BeatDetection) {
    BeatDetector detector;
    std::vector<float> samples(2048);
    AudioSpectrum spectrum, prevSpectrum;

    // Generate test spectrum with kick drum
    for (int i = 0; i < 100; i++) {
        spectrum.magnitudes.push_back(0.5f);
    }

    auto beats = detector.detectBeats(spectrum, prevSpectrum);
    // Verify beat detection logic
}

TEST_F(AudioTests, FrequencyConversion) {
    int bin = analyzer_->frequencyToBin(440.0f);
    float freq = analyzer_->binToFrequency(bin);
    EXPECT_NEAR(freq, 440.0f, 21.0f);  // Within one bin
}
```

#### Video Engine Tests
```cpp
class VideoEngineTests : public ::testing::Test {
public:
    void SetUp() override {
        engine_ = std::make_unique<VideoEngine>();
        EngineConfig config;
        ASSERT_TRUE(engine_->initialize(config));
    }

    std::unique_ptr<VideoEngine> engine_;
};

TEST_F(VideoEngineTests, TimelineSetup) {
    auto timeline = std::make_shared<Timeline>(1920, 1080, 30.0f);
    ASSERT_TRUE(engine_->setTimeline(timeline));
    EXPECT_EQ(engine_->getTimeline(), timeline);
}

TEST_F(VideoEngineTests, ClipAddition) {
    auto timeline = std::make_shared<Timeline>(1920, 1080, 30.0f);
    engine_->setTimeline(timeline);

    std::string clipId = engine_->addClip("test.mp4", 0, 0);
    EXPECT_FALSE(clipId.empty());
    EXPECT_EQ(engine_->getClipCount(), 1);
}

TEST_F(VideoEngineTests, ClipTrimming) {
    auto timeline = std::make_shared<Timeline>(1920, 1080, 30.0f);
    engine_->setTimeline(timeline);

    std::string clipId = engine_->addClip("test.mp4", 0, 0);
    ASSERT_TRUE(engine_->trimClip(clipId, 1000, 5000));
}
```

### Kotlin Component Tests
```kotlin
class ProjectViewModelTest {
    @get:Rule
    val instantExecutorRule = InstantTaskExecutorRule()

    private lateinit var viewModel: ProjectViewModel

    @Before
    fun setUp() {
        viewModel = ProjectViewModel()
    }

    @Test
    fun testProjectInitialization() {
        val project = Project(id = "1", name = "Test")
        viewModel.initializeProject(project)
        assertThat(viewModel.currentProject.value).isEqualTo(project)
    }

    @Test
    fun testAddClip() {
        val project = Project(id = "1", name = "Test")
        viewModel.initializeProject(project)

        viewModel.addClip(Uri.parse("content://test"), 0L, 0)
        assertThat(viewModel.clips.value).hasSize(1)
    }

    @Test
    fun testUndoRedo() {
        val project = Project(id = "1", name = "Test")
        viewModel.initializeProject(project)

        viewModel.addClip(Uri.parse("content://test1"), 0L, 0)
        viewModel.addClip(Uri.parse("content://test2"), 1000L, 0)

        val sizeAfterAdd = viewModel.clips.value?.size ?: 0
        viewModel.undo()
        assertThat(viewModel.clips.value?.size).isEqualTo(sizeAfterAdd - 1)

        viewModel.redo()
        assertThat(viewModel.clips.value?.size).isEqualTo(sizeAfterAdd)
    }
}
```

---

## Integration Tests

### GPU + Timeline Tests
```cpp
class GPUTimelineIntegrationTest : public ::testing::Test {
public:
    void SetUp() override {
        renderer_ = std::make_shared<GPURenderer>();
        RenderConfig config{1920, 1080};
        ASSERT_TRUE(renderer_->initialize(config));
    }

    std::shared_ptr<GPURenderer> renderer_;
};

TEST_F(GPUTimelineIntegrationTest, EffectChaining) {
    auto colorGrade = std::make_shared<ColorGradeEffect>();
    auto curves = std::make_shared<CurvesEffect>();
    auto blur = std::make_shared<GaussianBlurEffect>();

    ASSERT_TRUE(renderer_->addEffect(colorGrade));
    ASSERT_TRUE(renderer_->addEffect(curves));
    ASSERT_TRUE(renderer_->addEffect(blur));

    // Test effect chaining
    GLuint inputTexture = 1;
    GLuint outputFBO = 2;
    ASSERT_TRUE(renderer_->renderFrame(inputTexture, outputFBO));
}
```

### Export Pipeline Tests
```cpp
class ExportIntegrationTest : public ::testing::Test {
public:
    void SetUp() override {
        manager_ = std::make_shared<ExportManager>();
    }

    std::shared_ptr<ExportManager> manager_;
};

TEST_F(ExportIntegrationTest, ExportWorkflow) {
    ExportConfig config;
    config.outputPath = "/tmp/test_export.mp4";
    config.format = ExportFormat::MP4;
    config.quality = QualityPreset::MEDIUM;
    config.width = 1920;
    config.height = 1080;
    config.frameRate = 30;

    ASSERT_TRUE(manager_->setConfig(config));
    ASSERT_TRUE(manager_->startExport());

    // Simulate progress updates
    int lastProgress = 0;
    while (manager_->isExporting()) {
        const auto& progress = manager_->getProgress();
        EXPECT_GE(progress.totalProgress, lastProgress);
        lastProgress = progress.totalProgress;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(manager_->isComplete());
}
```

---

## Performance Tests

### GPU Performance
```cpp
class GPUPerformanceTest : public ::testing::Test {
public:
    void SetUp() override {
        renderer_ = std::make_shared<GPURenderer>();
        RenderConfig config{1920, 1080};
        renderer_->initialize(config);
    }

    std::shared_ptr<GPURenderer> renderer_;
};

TEST_F(GPUPerformanceTest, EffectRenderingSpeed) {
    auto effect = std::make_shared<ColorGradeEffect>();
    renderer_->addEffect(effect);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 60; i++) {  // 1 second at 60fps
        GLuint inputTexture = 1;
        GLuint outputFBO = 2;
        renderer_->renderFrame(inputTexture, outputFBO);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LE(duration.count(), 1100);  // Should be ~1000ms, allow 100ms overhead
}

TEST_F(GPUPerformanceTest, MultipleEffectsChain) {
    renderer_->addEffect(std::make_shared<ColorGradeEffect>());
    renderer_->addEffect(std::make_shared<GaussianBlurEffect>());
    renderer_->addEffect(std::make_shared<VignetteEffect>());

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 60; i++) {
        renderer_->renderFrame(1, 2);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LE(duration.count(), 1200);  // Allow slightly more for multiple effects
}
```

### Audio Performance
```cpp
TEST(AudioPerformanceTest, FFTSpeed) {
    FFTAnalyzer analyzer(2048, 44100);
    std::vector<float> samples(2048);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        analyzer.analyze(samples);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto avgTime = duration.count() / 100.0;
    EXPECT_LE(avgTime, 2.0);  // Should be <2ms per FFT
}
```

---

## Device Tests

### Android Compatibility
```kotlin
@RunWith(AndroidJUnit4::class)
class DeviceCompatibilityTest {

    @Test
    fun testOpenGLES3Support() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val configurationInfo = activityManager.deviceConfigurationInfo

        assertThat(ConfigurationInfo.GL_THREAD_RELEASED).isTrue()
        assertTrue(configurationInfo.glEsVersion >= 0x00030000)
    }

    @Test
    fun testHardwareEncodingSupport() {
        val mimeType = "video/avc"  // H.264
        val codecs = MediaCodecList(MediaCodecList.REGULAR_CODECS)
        var found = false

        for (info in codecs.codecInfos) {
            if (info.isEncoder && mimeType in info.supportedTypes) {
                found = true
                break
            }
        }

        assertTrue(found)
    }

    @Test
    fun testPermissions() {
        // Test Android 13+ media permissions
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            assertThat(
                ContextCompat.checkSelfPermission(
                    InstrumentationRegistry.getInstrumentation().targetContext,
                    Manifest.permission.READ_MEDIA_VIDEO
                )
            ).isEqualTo(PackageManager.PERMISSION_GRANTED)
        }
    }
}
```

---

## UI Tests

### Fragment Navigation
```kotlin
@RunWith(AndroidJUnit4::class)
class EditorFragmentTest {
    @get:Rule
    val activityScenarioRule: ActivityScenarioRule<EditorActivity> =
        ActivityScenarioRule(EditorActivity::class.java)

    @Test
    fun testFragmentDisplay() {
        onView(withId(R.id.timeline_view)).check(matches(isDisplayed()))
        onView(withId(R.id.video_preview)).check(matches(isDisplayed()))
    }

    @Test
    fun testPlaybackControls() {
        onView(withId(R.id.play_button)).perform(click())
        onView(withId(R.id.pause_button)).check(matches(isDisplayed()))
    }

    @Test
    fun testEffectsPanel() {
        onView(withId(R.id.effects_tab)).perform(click())
        onView(withId(R.id.effects_grid)).check(matches(isDisplayed()))
    }
}
```

### Effect Application
```kotlin
@RunWith(AndroidJUnit4::class)
class GPUTextureViewTest {
    private lateinit var textureView: GPUTextureView

    @Before
    fun setUp() {
        textureView = GPUTextureView(InstrumentationRegistry.getInstrumentation().targetContext)
    }

    @Test
    fun testEffectApplication() {
        textureView.applyEffect("ColorGrade")
        val effects = textureView.getAvailableEffects()
        assertThat(effects).contains("ColorGrade")
    }

    @Test
    fun testParameterControl() {
        textureView.applyEffect("HSL")
        textureView.setEffectParameter("HSL", "hue", 0.5f)
        // Verify parameter was set
    }
}
```

---

## Stress Tests

### Heavy Load Tests
```cpp
class StressTest : public ::testing::Test {
public:
    std::shared_ptr<VideoEngine> engine_;

    void SetUp() override {
        engine_ = std::make_shared<VideoEngine>();
        engine_->initialize(EngineConfig());
    }
};

TEST_F(StressTest, ManyClips) {
    auto timeline = std::make_shared<Timeline>(1920, 1080, 30.0f);
    engine_->setTimeline(timeline);

    // Add 100 clips
    for (int i = 0; i < 100; i++) {
        std::string clipId = engine_->addClip("test.mp4", i * 1000, 0);
        EXPECT_FALSE(clipId.empty());
    }

    EXPECT_EQ(engine_->getClipCount(), 100);
}

TEST_F(StressTest, ManyEffects) {
    auto renderer = std::make_shared<GPURenderer>();
    RenderConfig config;
    renderer->initialize(config);

    // Add multiple effects
    std::vector<std::string> effectNames = {
        "ColorGrade", "Curves", "HSL", "Blur", "Vignette",
        "Glow", "ChromaticAberration", "Glitch", "Posterize"
    };

    for (const auto& name : effectNames) {
        if (name == "ColorGrade") {
            renderer->addEffect(std::make_shared<ColorGradeEffect>());
        }
        // ... add other effects
    }

    EXPECT_EQ(renderer->getEffectCount(), effectNames.size());
}
```

---

## Memory Tests

### Memory Leak Detection
```cpp
TEST(MemoryTest, NoLeaksInGPUInit) {
    {
        auto context = std::make_unique<OpenGLContext>(1920, 1080);
        ASSERT_TRUE(context->initialize());
    }
    // Context should be properly destroyed
}

TEST(MemoryTest, TextureCleanup) {
    auto context = std::make_unique<OpenGLContext>(1920, 1080);
    context->initialize();

    GLuint texture = context->createTexture(1920, 1080, nullptr, GL_RGBA);
    context->deleteTexture(texture);

    // Verify no dangling pointers
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
```

---

## Continuous Integration

### Test Execution
```bash
# Run all tests
gradle test

# Run specific test class
gradle test --tests "GPUTests"

# Run with coverage
gradle test jacocoTestReport

# Performance testing
gradle performanceTest

# Device testing
./gradlew connectedAndroidTest
```

---

## Test Coverage Goals

- **C++ Core**: 80%+ coverage
- **GPU System**: 85%+ coverage
- **Audio System**: 80%+ coverage
- **Android UI**: 70%+ coverage
- **Overall**: 75%+ coverage

---

## Debugging Tips

### GPU Debugging
```cpp
// Enable GPU validation
glEnable(GL_DEBUG_OUTPUT);
glDebugMessageCallback(debugCallback, nullptr);

// Check framebuffer status
GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
if (status != GL_FRAMEBUFFER_COMPLETE) {
    LOG(ERROR) << "Framebuffer error: " << status;
}
```

### Performance Profiling
```cpp
auto start = std::chrono::high_resolution_clock::now();
// Code to profile
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
LOG(INFO) << "Duration: " << duration.count() << " microseconds";
```

### Memory Profiling
```bash
# Memory usage on Android
adb shell dumpsys meminfo com.clipforge

# GPU memory
adb shell dumpsys SurfaceFlinger
```

---

End of Testing Guide
