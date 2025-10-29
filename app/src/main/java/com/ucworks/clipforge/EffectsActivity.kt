package com.ucworks.clipforge

import android.os.Bundle
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.google.android.material.chip.Chip
import com.google.android.material.chip.ChipGroup
import com.ucworks.clipforge.databinding.ActivityEffectsBinding
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Activity for managing and applying effects/filters to clips.
 * Provides effect selection, preview, and parameter adjustment.
 */
class EffectsActivity : AppCompatActivity() {

    private lateinit var binding: ActivityEffectsBinding
    private var enginePtr: Long = 0L
    private var selectedClipId: String? = null
    private var selectedEffectId: String? = null

    private val effectCategories = listOf(
        "All", "Color", "Distortion", "Blur", "Light", "Artistic", "Vintage"
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityEffectsBinding.inflate(layoutInflater)
        setContentView(binding.root)

        enginePtr = intent.getLongExtra("enginePtr", 0L)
        selectedClipId = intent.getStringExtra("clipId")

        setupToolbar()
        setupCategoryChips()
        setupSearchBar()
        setupEffectProperties()
        setupActionButtons()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = getString(R.string.effects)
    }

    private fun setupCategoryChips() {
        effectCategories.forEach { category ->
            val chip = Chip(this).apply {
                text = category
                isCheckable = true
                if (category == "All") {
                    isChecked = true
                }
                setOnCheckedChangeListener { _, isChecked ->
                    if (isChecked) {
                        Timber.d("Filter by category: $category")
                        filterEffectsByCategory(category)
                    }
                }
            }
            binding.categoryChips.addView(chip)
        }
    }

    private fun setupSearchBar() {
        binding.searchEffects.setOnEditorActionListener { v, actionId, event ->
            val query = binding.searchEffects.text.toString()
            Timber.d("Searching effects: $query")
            searchEffects(query)
            false
        }

        binding.btnSearch.setOnClickListener {
            val query = binding.searchEffects.text.toString()
            Timber.d("Search button clicked: $query")
            searchEffects(query)
        }
    }

    private fun setupEffectProperties() {
        binding.effectIntensity.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser && selectedEffectId != null) {
                    Timber.d("Effect intensity changed to: $progress%")
                    // Update effect parameter in native engine
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })
    }

    private fun setupActionButtons() {
        binding.btnCancelEffect.setOnClickListener {
            Timber.d("Cancel effect button clicked")
            hideEffectProperties()
        }

        binding.btnApplyEffect.setOnClickListener {
            Timber.d("Apply effect button clicked")
            if (selectedClipId != null && selectedEffectId != null) {
                applyEffectToClip()
            }
        }
    }

    private fun filterEffectsByCategory(category: String) {
        lifecycleScope.launch {
            try {
                Timber.d("Loading effects for category: $category")
                // Load effects from effects library based on category
                // Display in RecyclerView grid
                val effects = when (category) {
                    "All" -> listOf("Vintage", "B&W", "Sepia", "Blur", "Sharpen")
                    "Color" -> listOf("Saturate", "Desaturate", "Hue Shift", "Color Adjust")
                    "Distortion" -> listOf("Wave", "Barrel", "Lens Distortion")
                    "Blur" -> listOf("Gaussian Blur", "Motion Blur", "Zoom Blur")
                    "Light" -> listOf("Bloom", "Glow", "Light Leak")
                    "Artistic" -> listOf("Oil Paint", "Comic", "Watercolor")
                    "Vintage" -> listOf("8-bit", "VHS", "Film Grain")
                    else -> emptyList()
                }
                Timber.d("Found ${effects.size} effects in category: $category")
            } catch (e: Exception) {
                Timber.e(e, "Error loading effects")
            }
        }
    }

    private fun searchEffects(query: String) {
        lifecycleScope.launch {
            try {
                Timber.d("Searching for effects matching: $query")
                // Search effects by name or tags
                val results = listOf("Vintage", "Blur", "Sepia").filter {
                    it.contains(query, ignoreCase = true)
                }
                Timber.d("Found ${results.size} matching effects")
            } catch (e: Exception) {
                Timber.e(e, "Error searching effects")
            }
        }
    }

    fun selectEffect(effectName: String) {
        lifecycleScope.launch {
            try {
                Timber.d("Effect selected: $effectName")
                binding.effectName.text = effectName
                binding.effectIntensity.progress = 100
                selectedEffectId = effectName // In real app, would be a unique ID
                showEffectProperties()
            } catch (e: Exception) {
                Timber.e(e, "Error selecting effect")
            }
        }
    }

    private fun showEffectProperties() {
        binding.effectPropertiesPanel.visibility = android.view.View.VISIBLE
    }

    private fun hideEffectProperties() {
        binding.effectPropertiesPanel.visibility = android.view.View.GONE
        selectedEffectId = null
    }

    private fun applyEffectToClip() {
        lifecycleScope.launch {
            try {
                if (selectedClipId != null && selectedEffectId != null) {
                    Timber.d("Applying effect to clip: clipId=$selectedClipId, effect=$selectedEffectId")
                    val success = NativeLib.applyEffect(enginePtr, selectedClipId!!, selectedEffectId!!)
                    if (success) {
                        Timber.d("Effect applied successfully")
                        hideEffectProperties()
                        finish()
                    } else {
                        Timber.e("Failed to apply effect")
                    }
                }
            } catch (e: Exception) {
                Timber.e(e, "Error applying effect")
            }
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
