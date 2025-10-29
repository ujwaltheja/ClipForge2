package com.ucworks.clipforge.fragments

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.ucworks.clipforge.NativeLib
import com.ucworks.clipforge.databinding.FragmentExportBinding
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Export Fragment - Project export and rendering configuration.
 * Provides:
 * - Format selection (MP4, WebM)
 * - Quality presets (Low, Medium, High, Ultra)
 * - Export location selection
 * - Real-time progress monitoring
 */
class ExportFragment : Fragment() {

    private var _binding: FragmentExportBinding? = null
    private val binding get() = _binding!!
    private var enginePtr: Long = 0L
    private var isExporting = false

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentExportBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        arguments?.let {
            enginePtr = it.getLong("enginePtr", 0L)
        }

        setupFormatSelection()
        setupQualitySelection()
        setupActionButtons()
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

    private fun setupActionButtons() {
        binding.btnCancel.setOnClickListener {
            Timber.d("Cancel button clicked")
            if (isExporting) {
                cancelExport()
            } else {
                requireActivity().onBackPressed()
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

        binding.btnBrowse.setOnClickListener {
            Timber.d("Browse button clicked")
            // In real app, would open file picker
            binding.exportPath.setText("/storage/emulated/0/Movies/ClipForge")
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

                    delay(500)
                } catch (e: Exception) {
                    Timber.e(e, "Error monitoring progress")
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
                    isExporting = false
                    hideExportProgress()
                    Timber.d("Export cancelled")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error cancelling export")
            }
        }
    }

    private fun onExportComplete() {
        Timber.d("Export complete")
        android.app.AlertDialog.Builder(requireContext())
            .setTitle("Export Complete")
            .setMessage("Your video has been successfully exported.")
            .setPositiveButton("OK") { _, _ ->
                hideExportProgress()
                requireActivity().onBackPressed()
            }
            .show()
    }

    private fun showExportProgress() {
        binding.root.findViewById<View>(com.ucworks.clipforge.R.id.actionBar).visibility = View.GONE
        binding.exportProgressPanel.visibility = View.VISIBLE
    }

    private fun hideExportProgress() {
        binding.exportProgressPanel.visibility = View.GONE
        binding.root.findViewById<View>(com.ucworks.clipforge.R.id.actionBar).visibility = View.VISIBLE
    }

    override fun onDestroyView() {
        super.onDestroyView()
        if (isExporting) {
            lifecycleScope.launch {
                try {
                    NativeLib.cancelExport(enginePtr)
                } catch (e: Exception) {
                    Timber.e(e, "Error cancelling export on destroy")
                }
            }
        }
        _binding = null
    }
}
