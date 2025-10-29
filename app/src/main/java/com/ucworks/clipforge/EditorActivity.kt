package com.ucworks.clipforge

import android.os.Bundle
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.google.android.material.tabs.TabLayout
import com.ucworks.clipforge.databinding.ActivityEditorBinding
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Main editing activity for timeline-based video editing.
 * Provides preview, playback controls, timeline management, and tool panels.
 */
class EditorActivity : AppCompatActivity() {

    private lateinit var binding: ActivityEditorBinding
    private var enginePtr: Long = 0L
    private var isPlaying = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityEditorBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupToolbar()
        setupPlaybackControls()
        setupTimelineSeek()
        setupTabLayout()
        setupActionButtons()
        initializeEngine()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = getString(R.string.editing_timeline)
    }

    private fun setupPlaybackControls() {
        binding.btnPlayPause.setOnClickListener {
            Timber.d("Play/Pause button clicked")
            if (isPlaying) {
                pausePreview()
            } else {
                startPreview()
            }
        }
    }

    private fun setupTimelineSeek() {
        binding.timelineSeek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    Timber.d("Seeking to position: $progress ms")
                    seekPreview(progress.toLong())
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })
    }

    private fun setupTabLayout() {
        binding.editorTabs.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {
            override fun onTabSelected(tab: TabLayout.Tab?) {
                when (tab?.position) {
                    0 -> {
                        Timber.d("Main tab selected")
                        showMainTools()
                    }
                    1 -> {
                        Timber.d("Effects tab selected")
                        showEffectsTools()
                    }
                    2 -> {
                        Timber.d("Library tab selected")
                        showLibraryTools()
                    }
                }
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {}
            override fun onTabReselected(tab: TabLayout.Tab?) {}
        })
    }

    private fun setupActionButtons() {
        binding.btnUndo.setOnClickListener {
            Timber.d("Undo button clicked")
            // Implement undo functionality
        }

        binding.btnExport.setOnClickListener {
            Timber.d("Export button clicked")
            startExportActivity()
        }
    }

    private fun initializeEngine() {
        lifecycleScope.launch {
            try {
                Timber.d("Initializing engine for editor")
                enginePtr = NativeLib.createEngine()
                val initSuccess = NativeLib.initEngine(enginePtr)
                if (initSuccess) {
                    Timber.d("Engine initialized successfully")
                    updateTimelineInfo()
                } else {
                    Timber.e("Failed to initialize engine")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error initializing engine")
            }
        }
    }

    private fun updateTimelineInfo() {
        lifecycleScope.launch {
            try {
                val duration = NativeLib.getTimelineDuration(enginePtr)
                val clipCount = NativeLib.getClipCount(enginePtr)
                Timber.d("Timeline: duration=$duration ms, clipCount=$clipCount")

                // Update UI
                binding.totalTime.text = formatTime(duration)
                binding.timelineSeek.max = duration.toInt()
            } catch (e: Exception) {
                Timber.e(e, "Error updating timeline info")
            }
        }
    }

    private fun startPreview() {
        lifecycleScope.launch {
            try {
                Timber.d("Starting preview")
                val success = NativeLib.startPreview(enginePtr)
                if (success) {
                    isPlaying = true
                    updatePlaybackUI()
                    startPlaybackTimer()
                }
            } catch (e: Exception) {
                Timber.e(e, "Error starting preview")
            }
        }
    }

    private fun pausePreview() {
        lifecycleScope.launch {
            try {
                Timber.d("Pausing preview")
                val success = NativeLib.pausePreview(enginePtr)
                if (success) {
                    isPlaying = false
                    updatePlaybackUI()
                }
            } catch (e: Exception) {
                Timber.e(e, "Error pausing preview")
            }
        }
    }

    private fun seekPreview(position: Long) {
        lifecycleScope.launch {
            try {
                Timber.d("Seeking to $position ms")
                NativeLib.seekPreview(enginePtr, position)
                binding.currentTime.text = formatTime(position)
            } catch (e: Exception) {
                Timber.e(e, "Error seeking preview")
            }
        }
    }

    private fun startPlaybackTimer() {
        lifecycleScope.launch {
            while (isPlaying) {
                try {
                    val position = NativeLib.getPreviewPosition(enginePtr)
                    binding.currentTime.text = formatTime(position)
                    binding.timelineSeek.progress = position.toInt()
                    kotlinx.coroutines.delay(100)
                } catch (e: Exception) {
                    Timber.e(e, "Error updating playback position")
                    break
                }
            }
        }
    }

    private fun updatePlaybackUI() {
        // Update button appearance based on isPlaying state
        // Can use different icons for play vs pause
    }

    private fun showMainTools() {
        Timber.d("Showing main tools")
        // Implement dynamic tool panel switching
    }

    private fun showEffectsTools() {
        Timber.d("Showing effects tools")
        // Implement dynamic tool panel switching
    }

    private fun showLibraryTools() {
        Timber.d("Showing library tools")
        // Implement dynamic tool panel switching
    }

    private fun startExportActivity() {
        Timber.d("Starting export activity")
        startActivity(android.content.Intent(this, ExportActivity::class.java).apply {
            putExtra("enginePtr", enginePtr)
        })
    }

    private fun formatTime(milliseconds: Long): String {
        val totalSeconds = milliseconds / 1000
        val minutes = totalSeconds / 60
        val seconds = totalSeconds % 60
        return String.format("%02d:%02d", minutes, seconds)
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }

    override fun onDestroy() {
        super.onDestroy()
        lifecycleScope.launch {
            try {
                if (enginePtr != 0L) {
                    Timber.d("Destroying engine")
                    NativeLib.destroyEngine(enginePtr)
                }
            } catch (e: Exception) {
                Timber.e(e, "Error destroying engine")
            }
        }
    }
}
