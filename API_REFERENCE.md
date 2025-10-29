# ClipForge API Reference

Complete API documentation for ClipForge Video Editor

---

## C++ API

### Core Engine

#### VideoEngine
```cpp
namespace clipforge::core {

class VideoEngine {
public:
    // Lifecycle
    bool initialize(const EngineConfig& config);
    void shutdown();
    EngineState getState() const;

    // Project Management
    bool setTimeline(std::shared_ptr<models::Timeline> timeline);
    std::shared_ptr<models::Timeline> getTimeline() const;

    // Clip Operations
    std::string addClip(const std::string& sourcePath, int64_t startPosition, int32_t trackIndex);
    bool removeClip(const std::string& clipId);
    bool moveClip(const std::string& clipId, int64_t newStartPosition, int32_t newTrackIndex);
    bool trimClip(const std::string& clipId, int64_t trimStart, int64_t trimEnd);
    bool setClipSpeed(const std::string& clipId, float speed);
    bool setClipVolume(const std::string& clipId, float volume);
    std::string splitClip(const std::string& clipId, int64_t splitTime);

    // Effects
    bool applyEffect(const std::string& clipId, std::shared_ptr<models::Effect> effect);
    bool removeEffect(const std::string& clipId, const std::string& effectId);
    std::vector<std::shared_ptr<models::Effect>> getAvailableEffects() const;

    // Audio
    std::string addAudioTrack(const std::string& name, const std::string& type);
    bool removeAudioTrack(const std::string& trackId);
    bool setAudioTrackVolume(const std::string& trackId, float volume);
    bool setAudioTrackMuted(const std::string& trackId, bool muted);

    // Preview
    bool startPreview();
    bool pausePreview();
    bool stopPreview();
    bool seekPreview(int64_t timeMs);
    bool isPreviewPlaying() const;
    int64_t getPreviewPosition() const;

    // Export
    bool startExport(const std::string& outputPath, const std::string& format, const std::string& quality);
    bool cancelExport();
    RenderProgress getExportProgress() const;
    bool isExporting() const;

    // Persistence
    bool saveProject(const std::string& projectPath);
    bool loadProject(const std::string& projectPath);
    bool hasUnsavedChanges() const;
};

}
```

### GPU System

#### GPURenderer
```cpp
namespace clipforge::gpu {

class GPURenderer {
public:
    // Initialization
    bool initialize(const RenderConfig& config);
    void shutdown();
    bool isInitialized() const;
    OpenGLContext* getContext() const;

    // Effects
    bool addEffect(std::shared_ptr<GPUEffect> effect);
    bool removeEffect(const std::string& effectName);
    std::shared_ptr<GPUEffect> getEffect(const std::string& effectName) const;
    bool hasEffect(const std::string& effectName) const;
    std::vector<std::string> getEffectNames() const;
    void clearEffects();
    bool setEffectEnabled(const std::string& effectName, bool enabled);

    // Rendering
    bool renderFrame(GLuint inputTexture, GLuint outputFramebuffer = 0);
    GLuint renderToTexture(GLuint inputTexture);
    bool applyEffect(GLuint inputTexture, const std::string& effectName, GLuint outputFramebuffer = 0);

    // Performance
    const RenderStats& getStats() const;
    void resetStats();
    void setProfilingEnabled(bool enable);

    // Framebuffers
    GLuint createIntermediateFramebuffer();
    void releaseIntermediateFramebuffer(GLuint fbo);

    // Debug
    std::string getDebugInfo() const;
    std::string getGPUInfo() const;
};

}
```

#### ShaderProgram
```cpp
namespace clipforge::gpu {

class ShaderProgram {
public:
    // Compilation
    bool compile(const ShaderSource& source);
    bool compileFromSources(const std::string& vertexSrc, const std::string& fragmentSrc,
                           const std::string& geometrySrc = "");
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath,
                      const std::string& geometryPath = "");

    // Usage
    void use() const;
    bool isValid() const;
    GLuint getId() const;

    // Uniforms (templated)
    void setUniform(const std::string& name, int value) const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, const glm::vec2& value) const;
    void setUniform(const std::string& name, const glm::vec3& value) const;
    void setUniform(const std::string& name, const glm::vec4& value) const;
    void setUniform(const std::string& name, const glm::mat3& mat) const;
    void setUniform(const std::string& name, const glm::mat4& mat) const;

    // Attributes
    void bindAttribute(GLuint index, const std::string& name) const;
    void enableAttribute(GLuint index) const;
    void disableAttribute(GLuint index) const;
    void vertexAttribPointer(GLuint index, GLint size, GLenum type,
                            GLboolean normalized, GLsizei stride, const void* offset) const;

    // Debug
    std::string getInfoLog() const;
    bool validate() const;
};

}
```

### Audio System

#### FFTAnalyzer
```cpp
namespace clipforge::audio {

class FFTAnalyzer {
public:
    FFTAnalyzer(int fftSize = 2048, int sampleRate = 44100);

    // Analysis
    AudioSpectrum analyze(const std::vector<float>& samples, bool useWindow = true);
    AudioSpectrum analyzeStereo(const std::vector<float>& samples, bool useWindow = true);
    std::vector<float> getMagnitudes(const std::vector<float>& samples);
    std::vector<float> getBandLevels(const AudioSpectrum& spectrum) const;

    // Conversion
    float binToFrequency(int binIndex) const;
    int frequencyToBin(float frequency) const;
    std::pair<float, float> getBandFrequencyRange(FrequencyBand band) const;
    std::pair<int, int> getBandBinRange(FrequencyBand band) const;

    // Properties
    int getFFTSize() const;
    int getSampleRate() const;
    float getFrequencyResolution() const;
    float getNyquistFrequency() const;
};

class BeatDetector {
public:
    BeatDetector();

    std::vector<BeatInfo> detectBeats(const AudioSpectrum& spectrum,
                                      const AudioSpectrum& prevSpectrum);
    void setSensitivity(float sensitivity);
    void setFrequencyRange(float minHz, float maxHz);
    void reset();
    std::vector<int32_t> getRecentBeats(int timeWindowMs = 2000) const;
};

}
```

### Export System

#### VideoEncoder
```cpp
namespace clipforge::encoding {

class VideoEncoder {
public:
    // Configuration
    bool configure(const VideoEncodingConfig& config);
    const VideoEncodingConfig& getConfig() const;
    bool isConfigured() const;

    // Control
    bool start();
    bool stop();
    bool isEncoding() const;
    bool pause();
    bool resume();

    // Encoding
    bool encodeFrame(const uint8_t* frameData, int64_t timestampMs, bool isKeyFrame = false);
    bool encodeFrameWithStride(const uint8_t* frameData, int width, int height,
                              int stride, int64_t timestampMs);
    uint8_t* getInputBuffer(size_t size);
    bool submitInputBuffer(uint8_t* data, size_t size, int64_t timestampMs);

    // Quality
    bool setBitrate(int bitrate);
    bool forceKeyframe();
    bool setQualityLevel(int level);

    // Monitoring
    const EncodingStats& getStats() const;
    int64_t getFramesEncoded() const;
    float getProgress() const;
    float getEstimatedTimeRemaining() const;
    float getEncodingFPS() const;
    int64_t getBytesEncoded() const;

    // Info
    static bool isCodecSupported(VideoCodec codec);
    static std::pair<int, int> getBitrateRange(VideoCodec codec, int width, int height);
    static int getRecommendedBitrate(int width, int height, int frameRate, int qualityLevel);

    // Error
    std::string getLastError() const;
    bool hasError() const;
    void clearError();
};

class ExportManager {
public:
    using ProgressCallback = std::function<void(const ExportProgress&)>;

    // Configuration
    bool setConfig(const ExportConfig& config);
    const ExportConfig& getConfig() const;
    int64_t getEstimatedFileSize() const;

    // Control
    bool startExport();
    bool cancelExport();
    bool isExporting() const;
    bool wasCancelled() const;
    bool isComplete() const;
    bool waitForCompletion(int timeoutMs = 0);

    // Progress
    const ExportProgress& getProgress() const;
    float getProgressPercentage() const;
    void setProgressCallback(ProgressCallback callback);
    std::string getCurrentPhase() const;
    float getEstimatedTimeRemaining() const;

    // Presets
    bool applyQualityPreset(QualityPreset preset);
    static int getPresetBitrate(QualityPreset preset);
    static std::pair<int, int> getPresetResolution(QualityPreset preset);
    static ExportFormat getRecommendedFormat(VideoCodec codec);

    // Stats
    int64_t getFinalFileSize() const;
    std::string getStatistics() const;
    std::string getErrorMessage() const;
    bool hasError() const;
};

}
```

---

## Kotlin/Android API

### UI Components

#### GPUTextureView
```kotlin
class GPUTextureView : TextureView {
    // Rendering
    fun startRendering()
    fun pauseRendering()
    fun stopRendering()
    fun isRendering(): Boolean

    // Effects
    fun applyEffect(effectName: String, parameters: Map<String, Float> = emptyMap())
    fun removeEffect(effectName: String)
    fun setEffectParameter(effectName: String, paramName: String, value: Float)
    fun getAvailableEffects(): Array<String>
    fun getEffectParameters(effectName: String): Array<String>
    fun clearEffects()

    // Performance
    fun getCurrentFPS(): Int
    fun getGPUTimeMs(): Float
    fun setTargetFPS(fps: Int)
    fun setOnFPSListener(listener: ((fps: Int, gpuTime: Float) -> Unit)?)
    fun getStats(): RenderStats

    // Resource Management
    fun release()
}
```

#### AudioVisualizerView
```kotlin
class AudioVisualizerView : View {
    // Data Update
    fun setSpectrumData(bands: FloatArray)
    fun setWaveformData(samples: FloatArray)
    fun setBeatStrength(strength: Float)
    fun setPeakFrequency(frequency: Float)

    // Control
    fun clear()
}
```

### ViewModels

#### ProjectViewModel
```kotlin
class ProjectViewModel : ViewModel {
    // State
    val currentProject: LiveData<Project>
    val clips: LiveData<List<VideoClip>>
    val audioTracks: LiveData<List<AudioTrack>>
    val isLoading: LiveData<Boolean>
    val hasUnsavedChanges: LiveData<Boolean>

    // Operations
    fun initializeProject(project: Project)
    fun addClip(sourceUri: Uri, position: Long, track: Int)
    fun removeClip(clipId: String)
    fun updateClip(clip: VideoClip)
    fun addAudioTrack(name: String, type: String)
    fun removeAudioTrack(trackId: String)
    fun updateAudioTrack(track: AudioTrack)
    fun saveProject()
    fun discardChanges()
}
```

#### EditorViewModel
```kotlin
class EditorViewModel : ViewModel {
    // State
    val isPlaying: LiveData<Boolean>
    val currentPosition: LiveData<Long>
    val timelineDuration: LiveData<Long>
    val selectedClipId: LiveData<String?>
    val activeToolTab: LiveData<Int>

    // Playback
    fun startPlayback()
    fun pausePlayback()
    fun stopPlayback()
    fun seek(position: Long)

    // Selection
    fun selectClip(clipId: String)
    fun setToolTab(tab: Int)

    // Undo/Redo
    fun undo()
    fun redo()
}
```

#### ExportViewModel
```kotlin
class ExportViewModel : ViewModel {
    // State
    val currentProject: LiveData<Project>
    val selectedQuality: LiveData<String>
    val selectedFormat: LiveData<String>
    val exportProgress: LiveData<Float>
    val isExporting: LiveData<Boolean>
    val exportError: LiveData<String>

    // Operations
    fun setProject(project: Project)
    fun setQualityPreset(quality: String)
    fun setExportFormat(format: String)
    fun getSelectedQuality(): String
    fun getSelectedFormat(): String
    fun getOutputPath(): String
    fun updateProgress(progress: Float)
    fun setExporting(isExporting: Boolean)
    fun setError(error: String)
    fun clearError()

    // Calculations
    fun getRecommendedBitrate(quality: String): Int
    fun getRecommendedResolution(quality: String): Pair<Int, Int>
    fun estimateFileSize(): Long
    fun getCodecForFormat(format: String): String
    fun isCompatible(format: String, quality: String): Boolean
    fun getMimeType(format: String): String
}
```

### Native Library Wrappers

#### NativeLib
```kotlin
object NativeLib {
    // Engine
    external fun createEngine(): Long
    external fun destroyEngine(enginePtr: Long)
    external fun initializeEngine(enginePtr: Long): Boolean

    // Preview
    external fun startPreview(enginePtr: Long): Boolean
    external fun pausePreview(enginePtr: Long): Boolean
    external fun stopPreview(enginePtr: Long): Boolean
    external fun seekPreview(enginePtr: Long, timeMs: Long): Boolean
    external fun getPreviewPosition(enginePtr: Long): Long

    // Export
    external fun startExport(enginePtr: Long, outputPath: String, format: String, quality: String): Boolean
    external fun cancelExport(enginePtr: Long): Boolean
    external fun getExportProgress(enginePtr: Long): Float
    external fun isExporting(enginePtr: Long): Boolean
}
```

#### GPUNativeLib
```kotlin
object GPUNativeLib {
    // Renderer Management
    external fun createGPURenderer(enginePtr: Long, width: Int, height: Int): Long
    external fun destroyGPURenderer(enginePtr: Long, rendererPtr: Long)
    external fun resizeGPURenderer(enginePtr: Long, rendererPtr: Long, width: Int, height: Int)

    // Effects
    external fun applyGPUEffect(enginePtr: Long, rendererPtr: Long, effectName: String): Boolean
    external fun removeGPUEffect(enginePtr: Long, rendererPtr: Long, effectName: String): Boolean
    external fun setEffectParameter(enginePtr: Long, rendererPtr: Long, effectName: String,
                                   paramName: String, value: Float): Boolean
    external fun clearGPUEffects(enginePtr: Long, rendererPtr: Long)

    // Rendering
    external fun renderGPUFrame(enginePtr: Long, rendererPtr: Long, timeMs: Long): Int
    external fun renderGPUToTexture(enginePtr: Long, rendererPtr: Long, inputTexture: Int): Int

    // Info
    external fun getAvailableEffects(enginePtr: Long, rendererPtr: Long): Array<String>
    external fun getEffectParameters(enginePtr: Long, rendererPtr: Long, effectName: String): Array<String>
    external fun getGPUStats(enginePtr: Long, rendererPtr: Long): GPUStats
    external fun getGPUInfo(enginePtr: Long, rendererPtr: Long): String
}
```

#### ExportNativeLib
```kotlin
object ExportNativeLib {
    // Manager
    external fun createExportManager(outputPath: String): Long
    external fun destroyExportManager(managerPtr: Long)
    external fun configureExport(managerPtr: Long, width: Int, height: Int, frameRate: Int,
                                quality: String, format: String): Boolean

    // Control
    external fun startExport(managerPtr: Long): Boolean
    external fun cancelExport(managerPtr: Long): Boolean
    external fun pauseExport(managerPtr: Long): Boolean
    external fun resumeExport(managerPtr: Long): Boolean
    external fun isExporting(managerPtr: Long): Boolean
    external fun isExportComplete(managerPtr: Long): Boolean

    // Progress
    external fun getExportProgress(managerPtr: Long): ExportProgress
    external fun getExportPhase(managerPtr: Long): String
    external fun getExportOutputPath(managerPtr: Long): String
    external fun getExportFileSize(managerPtr: Long): Long
    external fun getExportEstimatedTimeRemaining(managerPtr: Long): Float
}
```

---

## Data Models

### C++ Models
```cpp
// Project timeline
class Timeline {
    std::vector<std::shared_ptr<VideoClip>> getClips();
    std::shared_ptr<AudioTrack> getAudioTrack(const std::string& id);
    int64_t getDuration() const;
};

// Video clip
class VideoClip {
    std::string id;
    std::string sourceUri;
    int64_t positionMs, durationMs;
    float speed, volume, opacity;
    std::vector<std::shared_ptr<Effect>> effects;
};

// Audio track
class AudioTrack {
    std::string id, name;
    float volume, pan;
    bool isMuted, isSolo;
};

// Effect
class Effect {
    std::string id, name, category;
    float intensity;
    std::vector<EffectParameter> parameters;
};
```

### Kotlin Data Models
```kotlin
// Project
data class Project(
    val id: String,
    val name: String,
    val videoDurationMs: Long,
    val videoWidth: Int,
    val videoHeight: Int,
    val videoFrameRate: Int,
    val videoClips: List<VideoClip>,
    val audioTracks: List<AudioTrack>
)

// Media item
data class MediaItem(
    val id: String,
    val displayName: String,
    val uri: Uri,
    val duration: Long,
    val mimeType: String
)

// Audio track
data class AudioTrack(
    val id: String,
    val name: String,
    val volume: Float,
    val isMuted: Boolean,
    val bassGain: Float,
    val midGain: Float,
    val trebleGain: Float
)
```

---

## Configuration Classes

```cpp
// Engine
struct EngineConfig {
    int32_t previewQuality = 720;
    bool useHardwareAccel = true;
    int32_t maxRenderThreads = 4;
    bool enablePreviewCache = true;
    size_t maxCacheSize = 500 * 1024 * 1024;
};

// GPU Rendering
struct RenderConfig {
    int renderWidth = 1920;
    int renderHeight = 1080;
    int outputWidth = 1920;
    int outputHeight = 1080;
    bool useMultisampling = true;
    int sampleCount = 4;
};

// Video Encoding
struct VideoEncodingConfig {
    std::string outputPath;
    VideoCodec codec = VideoCodec::H264;
    int width = 1920, height = 1080, frameRate = 30;
    int bitrate = 8000000;
    bool useHardwareEncoding = true;
};

// Export
struct ExportConfig {
    std::string outputPath;
    ExportFormat format = ExportFormat::MP4;
    QualityPreset quality = QualityPreset::HIGH;
    int width = 1920, height = 1080, frameRate = 30;
    std::shared_ptr<models::Timeline> timeline;
};
```

---

## Error Handling

```cpp
// Exception types
enum class ErrorType {
    NetworkError,
    DatabaseError,
    FileSystemError,
    PermissionError,
    InvalidDataError,
    NativeError,
    TimeoutError,
    UnknownError
};

// Error handling
try {
    if (!result) {
        throw std::runtime_error("Operation failed");
    }
} catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what();
}
```

---

## Thread Safety

- All GPU operations use dedicated render thread
- Audio processing uses background threads
- Export uses separate thread pool
- Thread-safe queues for communication
- Atomic operations for state flags

---

## Performance Considerations

- Use hardware acceleration when available
- Leverage GPU for effects
- Cache frequently used shaders
- Stream large media files
- Profile with provided tools
- Monitor memory usage

---

End of API Reference
