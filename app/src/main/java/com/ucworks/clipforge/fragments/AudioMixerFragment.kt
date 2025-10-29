package com.ucworks.clipforge.fragments

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.SeekBar
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.ucworks.clipforge.NativeLib
import com.ucworks.clipforge.adapters.AudioTrackAdapter
import com.ucworks.clipforge.databinding.FragmentAudioMixerBinding
import com.ucworks.clipforge.models.AudioPresets
import com.ucworks.clipforge.models.AudioTrack
import com.ucworks.clipforge.models.AudioTrackType
import kotlinx.coroutines.launch
import timber.log.Timber
import java.util.UUID

/**
 * Audio Mixer Fragment - Audio track management and mixing.
 * Provides:
 * - Master volume control
 * - Per-track volume, mute, and pan
 * - EQ controls
 * - Add/remove tracks
 */
class AudioMixerFragment : Fragment() {

    private var _binding: FragmentAudioMixerBinding? = null
    private val binding get() = _binding!!
    private lateinit var audioAdapter: AudioTrackAdapter
    private var enginePtr: Long = 0L
    private val audioTracks = mutableListOf<AudioTrack>()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentAudioMixerBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        arguments?.let {
            enginePtr = it.getLong("enginePtr", 0L)
        }

        setupMasterVolumeControl()
        setupAudioTracksList()
        setupActionButtons()
        loadAudioTracks()
    }

    private fun setupMasterVolumeControl() {
        binding.masterVolume.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    val volume = progress / 100f
                    Timber.d("Master volume changed to: $volume")
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })

        binding.masterVolume.progress = 100  // Default to full volume
    }

    private fun setupAudioTracksList() {
        audioAdapter = AudioTrackAdapter(
            onVolumeChanged = { track, volume ->
                Timber.d("Track ${track.name} volume changed to: $volume")
                updateTrackVolume(track.id, volume)
            },
            onMuteToggled = { track, muted ->
                Timber.d("Track ${track.name} mute toggled: $muted")
                toggleTrackMute(track.id, muted)
            },
            onTrackRemoved = { track ->
                Timber.d("Track ${track.name} removed")
                removeAudioTrack(track)
            }
        )

        binding.audioTracksList.apply {
            layoutManager = LinearLayoutManager(requireContext())
            adapter = audioAdapter
            setHasFixedSize(false)
        }
    }

    private fun setupActionButtons() {
        binding.btnAddTrack.setOnClickListener {
            Timber.d("Add track button clicked")
            showAddTrackDialog()
        }

        binding.btnDone.setOnClickListener {
            Timber.d("Done button clicked")
            requireActivity().onBackPressed()
        }
    }

    private fun loadAudioTracks() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading audio tracks")
                // Create default tracks
                audioTracks.clear()

                // Main audio track (from video)
                audioTracks.add(
                    AudioTrack(
                        id = UUID.randomUUID().toString(),
                        name = "Main Audio",
                        type = AudioTrackType.MUSIC,
                        volume = 1.0f
                    )
                )

                audioAdapter.submitList(audioTracks.toList())
                Timber.d("Loaded ${audioTracks.size} audio tracks")
            } catch (e: Exception) {
                Timber.e(e, "Error loading audio tracks")
            }
        }
    }

    private fun updateTrackVolume(trackId: String, volume: Float) {
        lifecycleScope.launch {
            try {
                val success = NativeLib.setAudioTrackVolume(enginePtr, trackId, volume)
                if (success) {
                    Timber.d("Track volume updated: $trackId")
                } else {
                    Timber.e("Failed to update track volume")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error updating track volume")
            }
        }
    }

    private fun toggleTrackMute(trackId: String, muted: Boolean) {
        lifecycleScope.launch {
            try {
                val success = NativeLib.setAudioTrackMuted(enginePtr, trackId, muted)
                if (success) {
                    Timber.d("Track mute toggled: $trackId")
                } else {
                    Timber.e("Failed to toggle track mute")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error toggling track mute")
            }
        }
    }

    private fun removeAudioTrack(track: AudioTrack) {
        lifecycleScope.launch {
            try {
                Timber.d("Removing audio track: ${track.name}")
                val success = NativeLib.removeAudioTrack(enginePtr, track.id)
                if (success) {
                    audioTracks.remove(track)
                    audioAdapter.submitList(audioTracks.toList())
                    Timber.d("Track removed successfully")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error removing track")
            }
        }
    }

    private fun showAddTrackDialog() {
        Timber.d("Showing add track dialog")
        val types = arrayOf("Music", "Voiceover", "Sound Effects", "Ambient")

        android.app.AlertDialog.Builder(requireContext())
            .setTitle("Add Audio Track")
            .setItems(types) { _, which ->
                val type = when (which) {
                    0 -> AudioTrackType.MUSIC
                    1 -> AudioTrackType.VOICEOVER
                    2 -> AudioTrackType.SFX
                    3 -> AudioTrackType.AMBIENT
                    else -> AudioTrackType.MUSIC
                }
                addNewTrack(type)
            }
            .show()
    }

    private fun addNewTrack(type: AudioTrackType) {
        lifecycleScope.launch {
            try {
                Timber.d("Adding new track of type: $type")
                val trackId = NativeLib.addAudioTrack(
                    enginePtr,
                    "${type.name} Track",
                    type.name.lowercase()
                )

                if (trackId.isNotEmpty()) {
                    val newTrack = when (type) {
                        AudioTrackType.VOICEOVER -> AudioPresets.voiceoverPreset
                        AudioTrackType.MUSIC -> AudioPresets.musicPreset
                        AudioTrackType.SFX -> AudioPresets.sfxPreset
                        AudioTrackType.AMBIENT -> AudioPresets.ambientPreset
                    }

                    audioTracks.add(newTrack.copy(id = trackId))
                    audioAdapter.submitList(audioTracks.toList())
                    Timber.d("Track added successfully: $trackId")
                } else {
                    Timber.e("Failed to add track")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error adding track")
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
