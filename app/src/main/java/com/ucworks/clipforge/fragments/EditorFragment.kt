package com.ucworks.clipforge.fragments

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.SeekBar
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.google.android.material.tabs.TabLayout
import com.ucworks.clipforge.ExportActivity
import com.ucworks.clipforge.NativeLib
import com.ucworks.clipforge.databinding.FragmentEditorBinding
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Editor Fragment - Main timeline editing interface.
 * Provides:
 * - Video preview
 * - Playback controls
 * - Timeline navigation
 * - Tool panels (main, effects, library)
 */
class EditorFragment : Fragment() {

    private var _binding: FragmentEditorBinding? = null
    private val binding get() = _binding!!
    private var enginePtr: Long = 0L
    private var isPlaying = false

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentEditorBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        arguments?.let {
            enginePtr = it.getLong("enginePtr", 0L)
        }

        setupPlaybackControls()
        setupTimelineSeek()
        setupTabLayout()
        setupActionButtons()
        initializePreview()
    }

    private fun setupPlaybackControls() {
        binding.btnPlayPause.setOnClickListener {
            Timber.d("Play/Pause clicked")
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
                    Timber.d("Seeking to: $progress ms")
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
                    0 -> showMainTools()
                    1 -> showEffectsTools()
                    2 -> showLibraryTools()
                }
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {}
            override fun onTabReselected(tab: TabLayout.Tab?) {}
        })
    }

    private fun setupActionButtons() {
        binding.btnUndo.setOnClickListener {
            Timber.d("Undo clicked")
            // Implement undo in future
        }

        binding.btnExport.setOnClickListener {
            Timber.d("Export clicked")
            startActivity(Intent(requireContext(), ExportActivity::class.java).apply {
                putExtra("enginePtr", enginePtr)
            })
        }
    }

    private fun initializePreview() {
        lifecycleScope.launch {
            try {
                Timber.d("Initializing preview")
                val duration = NativeLib.getTimelineDuration(enginePtr)
                val clipCount = NativeLib.getClipCount(enginePtr)

                Timber.d("Timeline: duration=$duration ms, clipCount=$clipCount")

                binding.apply {
                    totalTime.text = formatTime(duration)
                    timelineSeek.max = duration.toInt().coerceAtLeast(1)
                }
            } catch (e: Exception) {
                Timber.e(e, "Error initializing preview")
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
                    startPlaybackTimer()
                    Timber.d("Preview started")
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
                    Timber.d("Preview paused")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error pausing preview")
            }
        }
    }

    private fun seekPreview(position: Long) {
        lifecycleScope.launch {
            try {
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
                    binding.apply {
                        currentTime.text = formatTime(position)
                        timelineSeek.progress = position.toInt()
                    }
                    delay(100)
                } catch (e: Exception) {
                    Timber.e(e, "Error updating playback")
                    break
                }
            }
        }
    }

    private fun showMainTools() {
        Timber.d("Showing main tools")
        // Show timeline/clip editing tools
    }

    private fun showEffectsTools() {
        Timber.d("Showing effects tools")
        // Show effects panel
    }

    private fun showLibraryTools() {
        Timber.d("Showing library tools")
        // Show media library
    }

    private fun formatTime(milliseconds: Long): String {
        val totalSeconds = (milliseconds / 1000).coerceAtLeast(0)
        val minutes = totalSeconds / 60
        val seconds = totalSeconds % 60
        return String.format("%02d:%02d", minutes, seconds)
    }

    override fun onDestroyView() {
        super.onDestroyView()
        isPlaying = false
        _binding = null
    }
}
