# AI Assistant Panel

**Purpose:** Smart editing suggestions

**Components:**
- Title: "AI Assistant - Smart Editing Suggestions"
- Suggestion Cards:
  1. **Auto Enhance** - Color balance & sharpness
  2. **Music Selection** - AI-chosen soundtrack
  3. **Scene Highlights** - Auto 5-sec social clip
- Apply/Cancel buttons

**Features:**
- ML-based suggestions
- One-click application
- Real-time preview

**Data Requirements:**
- ML model inference results
- Suggested parameters

**Related Code Files:**
- `app/src/main/java/com/clipforge/ui/AIAssistantFragment.kt` (Kotlin fragment)
- `app/src/main/java/com/clipforge/ml/AIProcessor.kt` (ML integration)
- `app/src/main/res/layout/fragment_ai_assistant.xml` (Layout)
- `ClipForge/NDK/core/video_engine.h` (C++ for AI integration)
