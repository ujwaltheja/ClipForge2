package com.ucworks.clipforge.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.SeekBar
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.ucworks.clipforge.databinding.ItemAudioTrackBinding
import com.ucworks.clipforge.models.AudioTrack
import timber.log.Timber

/**
 * RecyclerView adapter for displaying audio tracks with mixing controls.
 */
class AudioTrackAdapter(
    private val onVolumeChanged: (AudioTrack, Float) -> Unit,
    private val onMuteToggled: (AudioTrack, Boolean) -> Unit,
    private val onTrackRemoved: (AudioTrack) -> Unit
) : ListAdapter<AudioTrack, AudioTrackAdapter.TrackViewHolder>(AudioTrackDiffCallback()) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): TrackViewHolder {
        val binding = ItemAudioTrackBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return TrackViewHolder(binding)
    }

    override fun onBindViewHolder(holder: TrackViewHolder, position: Int) {
        val track = getItem(position)
        holder.bind(track)
    }

    inner class TrackViewHolder(
        private val binding: ItemAudioTrackBinding
    ) : RecyclerView.ViewHolder(binding.root) {

        fun bind(track: AudioTrack) {
            with(binding) {
                // Track Info
                trackName.text = track.name
                trackType.text = track.type.name
                volumeLabel.text = track.volumeDbString

                // Volume Slider (0-200 for 0x-2x range)
                trackVolume.progress = (track.volume * 100).toInt()
                trackVolume.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                        if (fromUser) {
                            val volume = progress / 100f
                            volumeLabel.text = String.format("%+.1f dB", 20 * kotlin.math.log10(volume.coerceIn(0.01f, 2.0f)))
                            onVolumeChanged(track, volume)
                            Timber.d("Track ${track.name} volume changed to: $volume")
                        }
                    }

                    override fun onStartTrackingTouch(seekBar: SeekBar?) {}
                    override fun onStopTrackingTouch(seekBar: SeekBar?) {}
                })

                // Mute Button
                btnMute.isChecked = track.isMuted
                btnMute.setOnCheckedChangeListener { _, isChecked ->
                    Timber.d("Track ${track.name} mute toggled: $isChecked")
                    onMuteToggled(track, isChecked)
                }

                // Remove Button
                btnRemove.setOnClickListener {
                    Timber.d("Remove track: ${track.name}")
                    onTrackRemoved(track)
                }

                // Pan Control (future enhancement)
                panLabel.text = track.panString

                // EQ Display (future enhancement for more controls)
                if (track.bassGain != 0f || track.midGain != 0f || track.trebleGain != 0f) {
                    eqStatus.text = "EQ Active"
                } else {
                    eqStatus.text = "No EQ"
                }
            }
        }
    }

    private class AudioTrackDiffCallback : DiffUtil.ItemCallback<AudioTrack>() {
        override fun areItemsTheSame(oldItem: AudioTrack, newItem: AudioTrack): Boolean {
            return oldItem.id == newItem.id
        }

        override fun areContentsTheSame(oldItem: AudioTrack, newItem: AudioTrack): Boolean {
            return oldItem == newItem
        }
    }
}
