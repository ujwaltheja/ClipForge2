package com.ucworks.clipforge

import android.os.Bundle
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.lifecycle.lifecycleScope
import com.ucworks.clipforge.databinding.ActivitySettingsBinding
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Settings screen for app configuration and preferences.
 * Manages video, audio, and application-level settings.
 */
class SettingsActivity : AppCompatActivity() {

    private lateinit var binding: ActivitySettingsBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySettingsBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupToolbar()
        setupVideoSettings()
        setupAudioSettings()
        setupApplicationSettings()
        setupAboutSection()
        loadSettings()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = getString(R.string.settings)
    }

    private fun setupVideoSettings() {
        binding.toggleAutoResolution.setOnCheckedChangeListener { _, isChecked ->
            Timber.d("Auto resolution toggled: $isChecked")
            onAutoResolutionChanged(isChecked)
        }
    }

    private fun setupAudioSettings() {
        val sampleRates = arrayOf("44100 Hz", "48000 Hz", "96000 Hz", "192000 Hz")
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, sampleRates)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        binding.sampleRateSpinner.adapter = adapter

        binding.sampleRateSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(parent: AdapterView<*>?, view: android.view.View?, position: Int, id: Long) {
                val selectedRate = sampleRates[position]
                Timber.d("Sample rate selected: $selectedRate")
                onSampleRateChanged(selectedRate)
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
            onNotificationsChanged(isChecked)
        }
    }

    private fun setupAboutSection() {
        binding.versionInfo.apply {
            try {
                val packageInfo = packageManager.getPackageInfo(packageName, 0)
                text = packageInfo.versionName
            } catch (e: Exception) {
                Timber.e(e, "Error getting version info")
                text = "1.0.0"
            }
        }

        binding.root.findViewById<android.view.View>(R.id.help_support).setOnClickListener {
            Timber.d("Help & Support clicked")
            showHelpAndSupport()
        }
    }

    private fun loadSettings() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading settings from SharedPreferences")
                val sharedPref = getSharedPreferences("clipforge_prefs", MODE_PRIVATE)

                // Load video settings
                val autoResolution = sharedPref.getBoolean("auto_resolution", true)
                binding.toggleAutoResolution.isChecked = autoResolution

                // Load audio settings
                val sampleRate = sharedPref.getString("sample_rate", "48000 Hz")
                val adapter = binding.sampleRateSpinner.adapter as ArrayAdapter<String>
                val position = (0 until adapter.count).find { adapter.getItem(it) == sampleRate } ?: 0
                binding.sampleRateSpinner.setSelection(position)

                // Load application settings
                val darkMode = sharedPref.getBoolean("dark_mode", true)
                binding.toggleDarkMode.isChecked = darkMode

                val notifications = sharedPref.getBoolean("notifications", true)
                binding.toggleNotifications.isChecked = notifications

                Timber.d("Settings loaded successfully")
            } catch (e: Exception) {
                Timber.e(e, "Error loading settings")
            }
        }
    }

    private fun onAutoResolutionChanged(enabled: Boolean) {
        lifecycleScope.launch {
            try {
                val sharedPref = getSharedPreferences("clipforge_prefs", MODE_PRIVATE)
                sharedPref.edit().putBoolean("auto_resolution", enabled).apply()
                Timber.d("Auto resolution setting saved: $enabled")
            } catch (e: Exception) {
                Timber.e(e, "Error saving auto resolution setting")
            }
        }
    }

    private fun onSampleRateChanged(rate: String) {
        lifecycleScope.launch {
            try {
                val sharedPref = getSharedPreferences("clipforge_prefs", MODE_PRIVATE)
                sharedPref.edit().putString("sample_rate", rate).apply()
                Timber.d("Sample rate setting saved: $rate")
            } catch (e: Exception) {
                Timber.e(e, "Error saving sample rate setting")
            }
        }
    }

    private fun onDarkModeChanged(enabled: Boolean) {
        lifecycleScope.launch {
            try {
                val sharedPref = getSharedPreferences("clipforge_prefs", MODE_PRIVATE)
                sharedPref.edit().putBoolean("dark_mode", enabled).apply()

                if (enabled) {
                    AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES)
                } else {
                    AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO)
                }

                Timber.d("Dark mode setting saved: $enabled")
                recreate()
            } catch (e: Exception) {
                Timber.e(e, "Error saving dark mode setting")
            }
        }
    }

    private fun onNotificationsChanged(enabled: Boolean) {
        lifecycleScope.launch {
            try {
                val sharedPref = getSharedPreferences("clipforge_prefs", MODE_PRIVATE)
                sharedPref.edit().putBoolean("notifications", enabled).apply()
                Timber.d("Notifications setting saved: $enabled")
            } catch (e: Exception) {
                Timber.e(e, "Error saving notifications setting")
            }
        }
    }

    private fun showHelpAndSupport() {
        Timber.d("Showing help and support information")
        // In a real app, would show a dialog or open a help screen
        android.app.AlertDialog.Builder(this)
            .setTitle(R.string.help_support)
            .setMessage("ClipForge Help & Support\n\nFor help, visit our website or contact support@clipforge.com")
            .setPositiveButton("OK") { dialog, _ ->
                dialog.dismiss()
            }
            .show()
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
