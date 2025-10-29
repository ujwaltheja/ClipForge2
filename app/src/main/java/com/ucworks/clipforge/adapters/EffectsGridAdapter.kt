package com.ucworks.clipforge.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.ucworks.clipforge.databinding.ItemEffectBinding
import com.ucworks.clipforge.models.Effect
import timber.log.Timber

/**
 * RecyclerView adapter for displaying effects/filters in a grid.
 */
class EffectsGridAdapter(
    private val onEffectClick: (Effect) -> Unit
) : ListAdapter<Effect, EffectsGridAdapter.EffectViewHolder>(EffectDiffCallback()) {

    private var selectedEffect: String? = null

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): EffectViewHolder {
        val binding = ItemEffectBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return EffectViewHolder(binding)
    }

    override fun onBindViewHolder(holder: EffectViewHolder, position: Int) {
        val item = getItem(position)
        val isSelected = item.id == selectedEffect
        holder.bind(item, isSelected)
    }

    inner class EffectViewHolder(
        private val binding: ItemEffectBinding
    ) : RecyclerView.ViewHolder(binding.root) {

        fun bind(effect: Effect, isSelected: Boolean) {
            with(binding) {
                effectName.text = effect.name
                effectCategory.text = effect.category
                effectDescription.text = effect.description

                // Update selection state
                effectCard.isChecked = isSelected
                effectCard.setCardBackgroundColor(
                    if (isSelected) android.graphics.Color.parseColor("#6B4FC8")
                    else android.graphics.Color.parseColor("#2D2438")
                )

                if (isSelected) {
                    effectCard.setStrokeColor(android.graphics.Color.parseColor("#7C5FD8"))
                    effectCard.strokeWidth = 3
                } else {
                    effectCard.strokeWidth = 0
                }

                root.setOnClickListener {
                    Timber.d("Effect selected: ${effect.name}")
                    val previousSelected = selectedEffect
                    selectedEffect = effect.id

                    // Update UI for previous and new selection
                    val previousPosition = currentList.indexOfFirst { it.id == previousSelected }
                    if (previousPosition >= 0) {
                        notifyItemChanged(previousPosition)
                    }
                    notifyItemChanged(adapterPosition)

                    onEffectClick(effect)
                }
            }
        }
    }

    fun getSelectedEffect(): Effect? {
        return currentList.find { it.id == selectedEffect }
    }

    fun clearSelection() {
        val previousSelected = selectedEffect
        selectedEffect = null
        val previousPosition = currentList.indexOfFirst { it.id == previousSelected }
        if (previousPosition >= 0) {
            notifyItemChanged(previousPosition)
        }
    }

    private class EffectDiffCallback : DiffUtil.ItemCallback<Effect>() {
        override fun areItemsTheSame(oldItem: Effect, newItem: Effect): Boolean {
            return oldItem.id == newItem.id
        }

        override fun areContentsTheSame(oldItem: Effect, newItem: Effect): Boolean {
            return oldItem == newItem
        }
    }
}
