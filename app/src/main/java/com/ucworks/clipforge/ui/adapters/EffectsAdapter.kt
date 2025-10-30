package com.ucworks.clipforge.ui.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.ucworks.clipforge.R
import com.ucworks.clipforge.databinding.ItemEffectBinding
import com.ucworks.clipforge.data.models.Effect

class EffectsAdapter(
    private val onEffectClick: (Effect) -> Unit
) : RecyclerView.Adapter<EffectsAdapter.EffectViewHolder>() {
    
    private val effects = listOf(
        Effect("color_grading", "Color Grading", R.drawable.ic_effect_color),
        Effect("curves", "Curves", R.drawable.ic_effect_curves),
        Effect("hsl", "HSL", R.drawable.ic_effect_hsl),
        Effect("blur", "Blur", R.drawable.ic_effect_blur),
        Effect("vignette", "Vignette", R.drawable.ic_effect_vignette),
        Effect("glow", "Glow", R.drawable.ic_effect_glow),
        Effect("chromatic", "Chromatic", R.drawable.ic_effect_chromatic),
        Effect("glitch", "Glitch", R.drawable.ic_effect_glitch),
        Effect("posterize", "Posterize", R.drawable.ic_effect_posterize),
        Effect("invert", "Invert", R.drawable.ic_effect_invert),
        Effect("grayscale", "Grayscale", R.drawable.ic_effect_grayscale)
    )
    
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): EffectViewHolder {
        val binding = ItemEffectBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return EffectViewHolder(binding, onEffectClick)
    }
    
    override fun onBindViewHolder(holder: EffectViewHolder, position: Int) {
        holder.bind(effects[position])
    }
    
    override fun getItemCount() = effects.size
    
    class EffectViewHolder(
        private val binding: ItemEffectBinding,
        private val onEffectClick: (Effect) -> Unit
    ) : RecyclerView.ViewHolder(binding.root) {
        
        fun bind(effect: Effect) {
            binding.apply {
                effectName.text = effect.name

                root.setOnClickListener { onEffectClick(effect) }
            }
        }
    }
}
