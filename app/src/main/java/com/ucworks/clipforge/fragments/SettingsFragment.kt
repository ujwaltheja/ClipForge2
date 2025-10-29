package com.ucworks.clipforge.fragments

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.appcompat.app.AppCompatDelegate
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.ucworks.clipforge.databinding.FragmentSettingsBinding
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Settings Fragment - App configuration and preferences.
 * Manages:
 * - Video settings (resolution, render threads)
 * - Audio settings (sample rate)
 * - App preferences (dark mode, notifications)
 * - About and version info
 */
class SettingsFragment : Fragment() {

    private var _binding: FragmentSettingsBinding? = null
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSettingsBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        setupVideoSettings()
        setupAudioSettings()
        setupApplicationSettings()
        setupAboutSection()
        loadSettings()
    }

    private fun setupVideoSettings() {
        binding.toggleAutoResolution.setOnCheckedChangeListener { _, isChecked ->
            Timber.d("Auto resolution toggled: $isChecked")
            saveVideoSetting("auto_resolution", isChecked)
        }
    }

    private fun setupAudioSettings() {
        val sampleRates = arrayOf("44100 Hz", "48000 Hz", "96000 Hz", "192000 Hz")
        val adapter = ArrayAdapter(
            requireContext(),
            android.R.layout.simple_spinner_item,
            sampleRates
        )
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        binding.sampleRateSpinner.adapter = adapter

        binding.sampleRateSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                val selectedRate = sampleRates[position]
                Timber.d("Sample rate selected: $selectedRate")
                saveAudioSetting("sample_rate", selectedRate)
            }

            override fun onNothingSelected(parent: AdapterView<*>?) {}
        }
    }

    private fun setupApplicationSettings() {
        binding.toggleDarkMode.setOnCheckedChangeListener { _, isChecked ->
            Timber.d("Dark mode toggled: $isChecked")
            onDarkModeChanged(isChecked)
        }

        binding.toggleNotifications.setOnCheckedChangeListener { _, isChecked ->
            Timber.d("Notifications toggled: $isChecked")
            saveAppSetting("notifications", isChecked)
        }
    }

    private fun setupAboutSection() {
        try {
            val packageInfo = requireContext().packageManager.getPackageInfo(
                requireContext().packageName,
                0
            )
            binding.versionInfo.text = packageInfo.versionName
        } catch (e: Exception) {
            Timber.e(e, "Error getting version info")
            binding.versionInfo.text = "1.0.0"
        }
    }

    private fun loadSettings() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading settings")
                val prefs = requireContext().getSharedPreferences(
                    "clipforge_prefs",
                    android.content.Context.MODE_PRIVATE
                )

                // Load video settings
                val autoResolution = prefs.getBoolean("auto_resolution", true)
                binding.toggleAutoResolution.isChecked = autoResolution

                // Load audio settings
                val sampleRate = prefs.getString("sample_rate", "48000 Hz") ?: "48000 Hz"
                val adapter = binding.sampleRateSpinner.adapter as ArrayAdapter<String>
                val position = (0 until adapter.count).find { adapter.getItem(it) == sampleRate } ?: 1
                binding.sampleRateSpinner.setSelection(position)

                // Load app settings
                val darkMode = prefs.getBoolean("dark_mode", true)
                binding.toggleDarkMode.isChecked = darkMode

                val notifications = prefs.getBoolean("notifications", true)
                binding.toggleNotifications.isChecked = notifications

                Timber.d("Settings loaded successfully")
            } catch (e: Exception) {
                Timber.e(e, "Error loading settings")
            }
        }
    }

    private fun saveVideoSetting(key: String, value: Boolean) {
        lifecycleScope.launch {
            try {
                val prefs = requireContext().getSharedPreferences(
                    "clipforge_prefs",
                    android.content.Context.MODE_PRIVATE
                )
                prefs.edit().putBoolean(key, value).apply()
                Timber.d("Video setting saved: $key = $value")
            } catch (e: Exception) {
                Timber.e(e, "Error saving video setting")
            }
        }
    }

    private fun saveAudioSetting(key: String, value: String) {
        lifecycleScope.launch {
            try {
                val prefs = requireContext().getSharedPreferences(
                    "clipforge_prefs",
                    android.content.Context.MODE_PRIVATE
                )
                prefs.edit().putString(key, value).apply()
                Timber.d("Audio setting saved: $key = $value")
            } catch (e: Exception) {
                Timber.e(e, "Error saving audio setting")
            }
        }
    }

    private fun saveAppSetting(key: String, value: Boolean) {
        lifecycleScope.launch {
            try {
                val prefs = requireContext().getSharedPreferences(
                    "clipforge_prefs",
                    android.content.Context.MODE_PRIVATE
                )
                prefs.edit().putBoolean(key, value).apply()
                Timber.d("App setting saved: $key = $value")
            } catch (e: Exception) {
                Timber.e(e, "Error saving app setting")
            }
        }
    }

    private fun onDarkModeChanged(enabled: Boolean) {
        if (enabled) {
            AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES)
        } else {
            AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO)
        }
        saveAppSetting("dark_mode", enabled)
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
