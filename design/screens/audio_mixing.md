# Audio Mixing Screen

**Purpose:** Multi-track audio editing and mixing

**Components:**
- Screen Title: "Audio Mixing"
- Navigation tabs: Main | Voiceover | Music
- Video Preview
- Audio Track Visualization:
  - Main track with waveform
  - Voiceover track with waveform
  - Effect Rack with controls:
    - Reverb (dial control)
    - Equalizer (dial control)
    - Pitch Shift (dial control)
    - Comp (dial control)
  - Recording button (Rec VO)

**Features:**
- Multi-track audio mixing
- Waveform visualization
- Audio effects (Reverb, EQ, Pitch, Compression)
- Voice recording
- Volume control

**Data Requirements:**
- Audio track data with waveforms
- Audio effect parameters
- Recording buffer

**Related Code Files:**
- `app/src/main/java/com/clipforge/ui/AudioActivity.kt` (Kotlin activity)
- `app/src/main/java/com/clipforge/view/AudioWaveformView.kt` (Custom view)
- `app/src/main/res/layout/activity_audio.xml` (Layout)
- `ClipForge/NDK/effects/audio_processor.h` (C++ for audio)
