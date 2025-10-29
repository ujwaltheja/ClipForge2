package com.ucworks.clipforge.fragments

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.SeekBar
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.GridLayoutManager
import com.google.android.material.chip.Chip
import com.ucworks.clipforge.NativeLib
import com.ucworks.clipforge.adapters.EffectsGridAdapter
import com.ucworks.clipforge.databinding.FragmentEffectsBinding
import com.ucworks.clipforge.models.EffectLibrary
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Effects Fragment - Allows selection and application of effects/filters to clips.
 * Provides:
 * - Category-based effect filtering
 * - Effect search functionality
 * - Real-time preview
 * - Intensity adjustment
 */
class EffectsFragment : Fragment() {

    private var _binding: FragmentEffectsBinding? = null
    private val binding get() = _binding!!
    private lateinit var effectsAdapter: EffectsGridAdapter
    private var enginePtr: Long = 0L
    private var selectedClipId: String? = null

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentEffectsBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        arguments?.let {
            enginePtr = it.getLong("enginePtr", 0L)
            selectedClipId = it.getString("clipId")
        }

        setupEffectsAdapter()
        setupCategoryChips()
        setupSearchBar()
        setupIntensitySlider()
        setupActionButtons()

        // Load all effects initially
        loadEffects("All")
    }

    private fun setupEffectsAdapter() {
        effectsAdapter = EffectsGridAdapter { effect ->
            Timber.d("Effect selected: ${effect.name}")
            binding.effectName.text = effect.name
            binding.effectDescription.text = effect.description
            binding.effectPropertiesPanel.visibility = View.VISIBLE
        }

        binding.effectsGrid.apply {
            layoutManager = GridLayoutManager(requireContext(), 2)
            adapter = effectsAdapter
            setHasFixedSize(false)
        }
    }

    private fun setupCategoryChips() {
        EffectLibrary.categories.forEach { category ->
            val chip = Chip(requireContext()).apply {
                text = category
                isCheckable = true
                if (category == "All") {
                    isChecked = true
                }
                setOnCheckedChangeListener { _, isChecked ->
                    if (isChecked) {
                        Timber.d("Category selected: $category")
                        loadEffects(category)
                    }
                }
            }
            binding.categoryChips.addView(chip)
        }
    }

    private fun setupSearchBar() {
        binding.btnSearch.setOnClickListener {
            val query = binding.searchEffects.text.toString()
            Timber.d("Searching effects: $query")
            searchEffects(query)
        }

        binding.searchEffects.setOnEditorActionListener { _, _, _ ->
            val query = binding.searchEffects.text.toString()
            searchEffects(query)
            false
        }
    }

    private fun setupIntensitySlider() {
        binding.effectIntensity.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    Timber.d("Effect intensity: $progress%")
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })
    }

    private fun setupActionButtons() {
        binding.btnCancelEffect.setOnClickListener {
            Timber.d("Cancel effect")
            hideEffectProperties()
        }

        binding.btnApplyEffect.setOnClickListener {
            Timber.d("Apply effect")
            applySelectedEffect()
        }
    }

    private fun loadEffects(category: String) {
        lifecycleScope.launch {
            try {
                val effects = EffectLibrary.getByCategory(category)
                Timber.d("Loaded ${effects.size} effects for category: $category")
                effectsAdapter.submitList(effects)
            } catch (e: Exception) {
                Timber.e(e, "Error loading effects")
            }
        }
    }

    private fun searchEffects(query: String) {
        lifecycleScope.launch {
            try {
                val results = EffectLibrary.search(query)
                Timber.d("Search found ${results.size} results for: $query")
                effectsAdapter.submitList(results)
            } catch (e: Exception) {
                Timber.e(e, "Error searching effects")
            }
        }
    }

    private fun applySelectedEffect() {
        lifecycleScope.launch {
            try {
                val selectedEffect = effectsAdapter.getSelectedEffect()
                if (selectedEffect == null || selectedClipId == null) {
                    Timber.w("No effect or clip selected")
                    return@launch
                }

                Timber.d("Applying effect: ${selectedEffect.name} to clip: $selectedClipId")
                val success = NativeLib.applyEffect(
                    enginePtr,
                    selectedClipId!!,
                    selectedEffect.name
                )

                if (success) {
                    Timber.d("Effect applied successfully")
                    hideEffectProperties()
                } else {
                    Timber.e("Failed to apply effect")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error applying effect")
            }
        }
    }

    private fun hideEffectProperties() {
        binding.effectPropertiesPanel.visibility = View.GONE
        effectsAdapter.clearSelection()
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
