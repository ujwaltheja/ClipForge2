package com.ucworks.clipforge.ui.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.ucworks.clipforge.databinding.ItemTimelineClipBinding
import com.ucworks.clipforge.data.models.Clip

class TimelineAdapter(
    private val onClipClick: (Clip) -> Unit,
    private val onClipMove: (Clip, Long) -> Unit,
    private val onClipTrim: (Clip, Long, Long) -> Unit
) : ListAdapter<Clip, TimelineAdapter.ClipViewHolder>(ClipDiffCallback()) {
    
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ClipViewHolder {
        val binding = ItemTimelineClipBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return ClipViewHolder(binding, onClipClick, onClipMove, onClipTrim)
    }
    
    override fun onBindViewHolder(holder: ClipViewHolder, position: Int) {
        holder.bind(getItem(position))
    }
    
    class ClipViewHolder(
        private val binding: ItemTimelineClipBinding,
        private val onClipClick: (Clip) -> Unit,
        private val onClipMove: (Clip, Long) -> Unit,
        private val onClipTrim: (Clip, Long, Long) -> Unit
    ) : RecyclerView.ViewHolder(binding.root) {
        
        fun bind(clip: Clip) {
            binding.apply {
                // Set clip width based on duration
                val widthPx = (clip.duration / 1000f * 10).toInt() // 10px per second
                root.layoutParams.width = widthPx.coerceAtLeast(40)
                
                // Load thumbnail
                Glide.with(binding.root.context)
                    .load(clip.thumbnailPath)
                    .into(ivClipThumbnail)
                
                // Set selection state
                root.isSelected = clip.isSelected
                
                // Click listener
                root.setOnClickListener { onClipClick(clip) }
                
                // Drag and drop (simplified)
                root.setOnLongClickListener {
                    // Implement drag functionality
                    true
                }
            }
        }
    }
    
    class ClipDiffCallback : DiffUtil.ItemCallback<Clip>() {
        override fun areItemsTheSame(oldItem: Clip, newItem: Clip): Boolean {
            return oldItem.id == newItem.id
        }
        
        override fun areContentsTheSame(oldItem: Clip, newItem: Clip): Boolean {
            return oldItem == newItem
        }
    }
}