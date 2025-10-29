package com.ucworks.clipforge

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.provider.MediaStore
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.lifecycle.lifecycleScope
import com.google.android.material.tabs.TabLayout
import com.ucworks.clipforge.databinding.ActivityImportBinding
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Activity for importing media from device, camera, or library.
 * Supports multi-tab interface for different import sources.
 */
class ImportActivity : AppCompatActivity() {

    private lateinit var binding: ActivityImportBinding

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val videoPermissionGranted = permissions[Manifest.permission.READ_MEDIA_VIDEO] ?: false
        val audioPermissionGranted = permissions[Manifest.permission.READ_MEDIA_AUDIO] ?: false
        val cameraPermissionGranted = permissions[Manifest.permission.CAMERA] ?: false

        if (videoPermissionGranted || audioPermissionGranted) {
            Timber.d("Media permissions granted")
            loadDeviceMedia()
        }
        if (cameraPermissionGranted) {
            Timber.d("Camera permission granted")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityImportBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupToolbar()
        setupTabLayout()
        setupListeners()
        checkAndRequestPermissions()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = getString(R.string.import_media)
    }

    private fun setupTabLayout() {
        binding.tabLayout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {
            override fun onTabSelected(tab: TabLayout.Tab?) {
                when (tab?.position) {
                    0 -> {
                        Timber.d("Device tab selected")
                        loadDeviceMedia()
                    }
                    1 -> {
                        Timber.d("Camera tab selected")
                        launchCamera()
                    }
                    2 -> {
                        Timber.d("Library tab selected")
                        loadLibraryMedia()
                    }
                }
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {}
            override fun onTabReselected(tab: TabLayout.Tab?) {}
        })
    }

    private fun setupListeners() {
        binding.btnCancel.setOnClickListener {
            Timber.d("Import cancelled")
            finish()
        }

        binding.btnImport.setOnClickListener {
            Timber.d("Import button clicked - importing selected items")
            importSelectedMedia()
        }
    }

    private fun checkAndRequestPermissions() {
        val requiredPermissions = mutableListOf(
            Manifest.permission.READ_MEDIA_VIDEO,
            Manifest.permission.READ_MEDIA_AUDIO
        ).apply {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                // Android 13+ permissions already added
            }
        }

        val missingPermissions = requiredPermissions.filter {
            ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }

        if (missingPermissions.isNotEmpty()) {
            Timber.d("Requesting permissions: $missingPermissions")
            requestPermissionLauncher.launch(missingPermissions.toTypedArray())
        } else {
            Timber.d("All permissions already granted")
            loadDeviceMedia()
        }
    }

    private fun loadDeviceMedia() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading device media")
                // Query MediaStore for videos and audio files
                val cursor = contentResolver.query(
                    MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                    arrayOf(
                        MediaStore.Video.Media._ID,
                        MediaStore.Video.Media.DISPLAY_NAME,
                        MediaStore.Video.Media.DURATION,
                        MediaStore.Video.Media.DATA
                    ),
                    null,
                    null,
                    null
                )

                cursor?.use { c ->
                    val items = mutableListOf<String>()
                    while (c.moveToNext()) {
                        val displayName = c.getString(1)
                        items.add(displayName)
                    }
                    Timber.d("Found ${items.size} media items")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error loading device media")
            }
        }
    }

    private fun launchCamera() {
        try {
            val cameraIntent = Intent(MediaStore.ACTION_VIDEO_CAPTURE).apply {
                putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 1)
                putExtra(MediaStore.EXTRA_DURATION_LIMIT, 120)
            }
            Timber.d("Launching camera")
            // startActivityForResult would be needed here for proper implementation
        } catch (e: Exception) {
            Timber.e(e, "Error launching camera")
        }
    }

    private fun loadLibraryMedia() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading library media")
                // Implementation would query a custom library or database
                // This is a placeholder for future integration
            } catch (e: Exception) {
                Timber.e(e, "Error loading library media")
            }
        }
    }

    private fun importSelectedMedia() {
        lifecycleScope.launch {
            try {
                Timber.d("Importing selected media")
                // Get selected items from RecyclerView adapter
                // Call NativeLib methods to add clips to engine
                // Then return to editor

                finish()
            } catch (e: Exception) {
                Timber.e(e, "Error importing media")
            }
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
