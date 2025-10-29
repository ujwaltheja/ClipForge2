package com.ucworks.clipforge.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.ucworks.clipforge.databinding.ItemMediaBinding
import com.ucworks.clipforge.models.MediaItem
import timber.log.Timber

/**
 * RecyclerView adapter for displaying media items in a grid.
 */
class MediaGridAdapter(
    private val onItemClick: (MediaItem) -> Unit,
    private val onItemLongClick: (MediaItem) -> Boolean
) : ListAdapter<MediaItem, MediaGridAdapter.MediaViewHolder>(MediaDiffCallback()) {

    private val selectedItems = mutableSetOf<String>()

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): MediaViewHolder {
        val binding = ItemMediaBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return MediaViewHolder(binding)
    }

    override fun onBindViewHolder(holder: MediaViewHolder, position: Int) {
        val item = getItem(position)
        holder.bind(item, selectedItems.contains(item.id))
    }

    inner class MediaViewHolder(
        private val binding: ItemMediaBinding
    ) : RecyclerView.ViewHolder(binding.root) {

        fun bind(item: MediaItem, isSelected: Boolean) {
            with(binding) {
                mediaTitle.text = item.displayName
                mediaDuration.text = item.formattedDuration
                mediaSize.text = item.formattedSize

                // Update selection state
                mediaCard.isChecked = isSelected
                mediaCard.setCardBackgroundColor(
                    if (isSelected) android.graphics.Color.parseColor("#6B4FC8")
                    else android.graphics.Color.parseColor("#2D2438")
                )

                // Set click listeners
                root.setOnClickListener {
                    Timber.d("Media item clicked: ${item.displayName}")
                    toggleSelection(item)
                    onItemClick(item)
                }

                root.setOnLongClickListener {
                    Timber.d("Media item long clicked: ${item.displayName}")
                    val handled = onItemLongClick(item)
                    if (handled) {
                        toggleSelection(item)
                    }
                    handled
                }

                // Load thumbnail (would be async in real app)
                if (item.thumbnail != null) {
                    // mediaThumb.setImageURI(item.thumbnail)
                    Timber.d("Loading thumbnail for: ${item.displayName}")
                }
            }
        }

        private fun toggleSelection(item: MediaItem) {
            if (selectedItems.contains(item.id)) {
                selectedItems.remove(item.id)
            } else {
                selectedItems.add(item.id)
            }
            notifyItemChanged(adapterPosition)
        }
    }

    fun getSelectedItems(): List<MediaItem> {
        return currentList.filter { selectedItems.contains(it.id) }
    }

    fun clearSelection() {
        selectedItems.clear()
        notifyDataSetChanged()
    }

    fun selectAll() {
        selectedItems.addAll(currentList.map { it.id })
        notifyDataSetChanged()
    }

    private class MediaDiffCallback : DiffUtil.ItemCallback<MediaItem>() {
        override fun areItemsTheSame(oldItem: MediaItem, newItem: MediaItem): Boolean {
            return oldItem.id == newItem.id
        }

        override fun areContentsTheSame(oldItem: MediaItem, newItem: MediaItem): Boolean {
            return oldItem == newItem
        }
    }
}
