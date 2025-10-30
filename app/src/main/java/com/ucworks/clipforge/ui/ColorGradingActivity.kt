package com.ucworks.clipforge.ui

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.ViewModelProvider
import com.ucworks.clipforge.databinding.ActivityColorGradingBinding
import com.ucworks.clipforge.ui.viewmodels.EditorViewModel

class ColorGradingActivity : AppCompatActivity() {

    private lateinit var binding: ActivityColorGradingBinding
    private lateinit var viewModel: EditorViewModel

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityColorGradingBinding.inflate(layoutInflater)
        setContentView(binding.root)

        viewModel = ViewModelProvider(this).get(EditorViewModel::class.java)
        setupToolbar()
        setupControls()
    }

    private fun setupToolbar() {
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = "Color Grading"

        binding.colorGradingSaveButton.setOnClickListener {
            viewModel.saveProject()
            finish()
        }

        binding.colorGradingResetButton.setOnClickListener {
            resetAll()
        }
    }

    private fun setupControls() {
        // Exposure slider
        binding.exposureSlider.addOnChangeListener { _, value, _ ->
            binding.exposureValue.text = String.format("%.1f", value)
            updatePreview()
        }

        // Saturation slider
        binding.saturationSlider.addOnChangeListener { _, value, _ ->
            binding.saturationValue.text = String.format("%.1f", value)
            updatePreview()
        }

        // Contrast slider
        binding.contrastSlider.addOnChangeListener { _, value, _ ->
            binding.contrastValue.text = String.format("%.1f", value)
            updatePreview()
        }

        // Temperature slider
        binding.temperatureSlider.addOnChangeListener { _, value, _ ->
            binding.temperatureValue.text = String.format("%.0f", value)
            updatePreview()
        }

        // Highlights slider
        binding.highlightsSlider.addOnChangeListener { _, value, _ ->
            binding.highlightsValue.text = String.format("%.0f", value)
            updatePreview()
        }

        // Shadows slider
        binding.shadowsSlider.addOnChangeListener { _, value, _ ->
            binding.shadowsValue.text = String.format("%.0f", value)
            updatePreview()
        }

        // Midtones slider
        binding.midtonesSlider.addOnChangeListener { _, value, _ ->
            binding.midtonesValue.text = String.format("%.0f", value)
            updatePreview()
        }
    }

    private fun updatePreview() {
        val exposure = binding.exposureSlider.value
        val saturation = binding.saturationSlider.value
        val contrast = binding.contrastSlider.value

        // Apply color grading
        viewModel.updateColorGrade(
            exposure = exposure,
            saturation = saturation,
            contrast = contrast
        )
    }

    private fun resetAll() {
        binding.exposureSlider.value = 0f
        binding.saturationSlider.value = 0f
        binding.contrastSlider.value = 0f
        binding.temperatureSlider.value = 0f
        binding.highlightsSlider.value = 0f
        binding.shadowsSlider.value = 0f
        binding.midtonesSlider.value = 0f

        updatePreview()
    }
}
