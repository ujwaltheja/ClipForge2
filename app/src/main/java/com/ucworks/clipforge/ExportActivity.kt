package com.ucworks.clipforge

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.ucworks.clipforge.databinding.ActivityExportBinding
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Activity for project export/rendering configuration and progress.
 * Allows selection of format, quality, and export destination.
 */
class ExportActivity : AppCompatActivity() {

    private lateinit var binding: ActivityExportBinding
    private var enginePtr: Long = 0L
    private var isExporting = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityExportBinding.inflate(layoutInflater)
        setContentView(binding.root)

        enginePtr = intent.getLongExtra("enginePtr", 0L)

        setupToolbar()
        setupFormatSelection()
        setupQualitySelection()
        setupFileLocation()
        setupActionButtons()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = getString(R.string.export_your_reel)
    }

    private fun setupFormatSelection() {
        binding.formatMp4.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                Timber.d("Format selected: MP4")
                updateEstimatedSize()
            }
        }

        binding.formatWebm.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                Timber.d("Format selected: WebM")
                updateEstimatedSize()
            }
        }
    }

    private fun setupQualitySelection() {
        binding.qualityLow.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                Timber.d("Quality selected: Low")
                updateEstimatedSize()
            }
        }

        binding.qualityMedium.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                Timber.d("Quality selected: Medium")
                updateEstimatedSize()
            }
        }

        binding.qualityHigh.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                Timber.d("Quality selected: High")
                updateEstimatedSize()
            }
        }

        binding.qualityUltra.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                Timber.d("Quality selected: Ultra")
                updateEstimatedSize()
            }
        }
    }

    private fun setupFileLocation() {
        binding.btnBrowse.setOnClickListener {
            Timber.d("Browse button clicked - selecting export location")
            selectExportLocation()
        }
    }

    private fun setupActionButtons() {
        binding.btnCancel.setOnClickListener {
            Timber.d("Cancel button clicked")
            if (isExporting) {
                cancelExport()
            } else {
                finish()
            }
        }

        binding.btnExport.setOnClickListener {
            Timber.d("Export button clicked")
            startExport()
        }

        binding.btnCancelExport.setOnClickListener {
            Timber.d("Cancel export button clicked")
            cancelExport()
        }
    }

    private fun updateEstimatedSize() {
        val quality = when {
            binding.qualityLow.isChecked -> "Low"
            binding.qualityMedium.isChecked -> "Medium"
            binding.qualityHigh.isChecked -> "High"
            binding.qualityUltra.isChecked -> "Ultra"
            else -> "Unknown"
        }

        val size = when (quality) {
            "Low" -> "~50 MB"
            "Medium" -> "~150 MB"
            "High" -> "~300 MB"
            "Ultra" -> "~1 GB"
            else -> "Unknown"
        }

        binding.estimatedSize.text = size
        Timber.d("Estimated file size: $size")
    }

    private fun selectExportLocation() {
        lifecycleScope.launch {
            try {
                Timber.d("Selecting export location")
                // In real implementation, would open file picker
                // For now, default to app cache directory
                val cacheDir = cacheDir
                binding.exportPath.setText(cacheDir.absolutePath)
            } catch (e: Exception) {
                Timber.e(e, "Error selecting export location")
            }
        }
    }

    private fun startExport() {
        lifecycleScope.launch {
            try {
                val projectName = binding.projectName.text.toString().ifEmpty { "Untitled" }
                val quality = getSelectedQuality()
                val format = getSelectedFormat()
                val outputPath = binding.exportPath.text.toString()

                if (outputPath.isEmpty()) {
                    Timber.w("Export path not selected")
                    return@launch
                }

                val fullPath = "$outputPath/$projectName.$format"
                Timber.d("Starting export: path=$fullPath, format=$format, quality=$quality")

                isExporting = true
                showExportProgress()

                val success = NativeLib.startExport(enginePtr, fullPath, format, quality)
                if (success) {
                    Timber.d("Export started successfully")
                    monitorExportProgress()
                } else {
                    Timber.e("Failed to start export")
                    hideExportProgress()
                    isExporting = false
                }
            } catch (e: Exception) {
                Timber.e(e, "Error starting export")
                hideExportProgress()
                isExporting = false
            }
        }
    }

    private fun getSelectedQuality(): String {
        return when {
            binding.qualityLow.isChecked -> "low"
            binding.qualityMedium.isChecked -> "medium"
            binding.qualityHigh.isChecked -> "high"
            binding.qualityUltra.isChecked -> "ultra"
            else -> "medium"
        }
    }

    private fun getSelectedFormat(): String {
        return when {
            binding.formatMp4.isChecked -> "mp4"
            binding.formatWebm.isChecked -> "webm"
            else -> "mp4"
        }
    }

    private fun monitorExportProgress() {
        lifecycleScope.launch {
            while (isExporting) {
                try {
                    val progress = NativeLib.getExportProgress(enginePtr)
                    val isStillExporting = NativeLib.isExporting(enginePtr)

                    binding.exportProgressBar.progress = progress.toInt()
                    binding.progressPercent.text = "${progress.toInt()}%"

                    if (!isStillExporting) {
                        Timber.d("Export completed")
                        isExporting = false
                        onExportComplete()
                    }

                    delay(500)  // Update every 500ms
                } catch (e: Exception) {
                    Timber.e(e, "Error monitoring export progress")
                    break
                }
            }
        }
    }

    private fun cancelExport() {
        lifecycleScope.launch {
            try {
                Timber.d("Cancelling export")
                val success = NativeLib.cancelExport(enginePtr)
                if (success) {
                    Timber.d("Export cancelled successfully")
                    isExporting = false
                    hideExportProgress()
                }
            } catch (e: Exception) {
                Timber.e(e, "Error cancelling export")
            }
        }
    }

    private fun onExportComplete() {
        Timber.d("Export complete - showing completion message")
        hideExportProgress()
        // Show completion dialog or auto-close
        finish()
    }

    private fun showExportProgress() {
        binding.actionBar.visibility = android.view.View.GONE
        binding.exportProgressPanel.visibility = android.view.View.VISIBLE
    }

    private fun hideExportProgress() {
        binding.exportProgressPanel.visibility = android.view.View.GONE
        binding.actionBar.visibility = android.view.View.VISIBLE
    }

    override fun onSupportNavigateUp(): Boolean {
        if (isExporting) {
            Timber.w("Cannot go back while exporting")
            return false
        }
        finish()
        return true
    }

    override fun onDestroy() {
        super.onDestroy()
        if (isExporting) {
            Timber.d("Activity destroyed while exporting - cancelling export")
            lifecycleScope.launch {
                try {
                    NativeLib.cancelExport(enginePtr)
                } catch (e: Exception) {
                    Timber.e(e, "Error cancelling export on destroy")
                }
            }
        }
    }
}
