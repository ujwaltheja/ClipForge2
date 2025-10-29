package com.ucworks.clipforge

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.lifecycle.lifecycleScope
import com.ucworks.clipforge.databinding.ActivityMainBinding
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Main Activity for ClipForge Video Editor
 *
 * Provides the entry point to the application and manages:
 * - Native C++ engine initialization
 * - Permission handling
 * - Navigation to other screens
 * - Project management
 */
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var enginePtr: Long = 0

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val allGranted = permissions.values.all { it }
        if (allGranted) {
            initializeEngine()
            Timber.i("All permissions granted")
        } else {
            Timber.w("Some permissions denied")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Initialize logging (using debug tree for development)
        Timber.plant(Timber.DebugTree())
        Timber.i("MainActivity created")

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setSupportActionBar(binding.toolbar)

        setupClickListeners()
        checkPermissionsAndInitialize()
    }

    private fun checkPermissionsAndInitialize() {
        val requiredPermissions = mutableListOf(
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.CAMERA,
            Manifest.permission.RECORD_AUDIO
        )

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            requiredPermissions.remove(Manifest.permission.READ_EXTERNAL_STORAGE)
            requiredPermissions.remove(Manifest.permission.WRITE_EXTERNAL_STORAGE)
            requiredPermissions.add(Manifest.permission.READ_MEDIA_VIDEO)
            requiredPermissions.add(Manifest.permission.READ_MEDIA_AUDIO)
        }

        val missingPermissions = requiredPermissions.filter {
            ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }

        if (missingPermissions.isNotEmpty()) {
            requestPermissionLauncher.launch(missingPermissions.toTypedArray())
        } else {
            initializeEngine()
        }
    }

    private fun initializeEngine() {
        lifecycleScope.launch {
            try {
                Timber.i("Creating VideoEngine instance")
                enginePtr = NativeLib.createEngine()

                if (enginePtr == 0L) {
                    Timber.e("Failed to create engine")
                    return@launch
                }

                Timber.i("Initializing VideoEngine")
                val initialized = NativeLib.initEngine(enginePtr)

                if (!initialized) {
                    Timber.e("Failed to initialize engine")
                    destroyEngine()
                    return@launch
                }

                val version = NativeLib.getEngineVersion(enginePtr)
                Timber.i("VideoEngine initialized (version: %s)", version)

            } catch (e: Exception) {
                Timber.e(e, "Error initializing engine")
            }
        }
    }

    private fun destroyEngine() {
        if (enginePtr != 0L) {
            try {
                Timber.i("Destroying VideoEngine")
                NativeLib.destroyEngine(enginePtr)
                enginePtr = 0
            } catch (e: Exception) {
                Timber.e(e, "Error destroying engine")
            }
        }
    }

    private fun setupClickListeners() {
        binding.newProjectButton.setOnClickListener {
            Timber.i("New Project clicked")
            createNewProject()
        }

        binding.importMediaButton.setOnClickListener {
            Timber.i("Import Media clicked")
            navigateToImport()
        }

        binding.recordButton.setOnClickListener {
            Timber.i("Record clicked")
            navigateToRecord()
        }

        binding.templatesButton.setOnClickListener {
            Timber.i("Templates clicked")
            navigateToTemplates()
        }
    }

    private fun createNewProject() {
        if (enginePtr == 0L) {
            Toast.makeText(this, "Engine not initialized", Toast.LENGTH_SHORT).show()
            return
        }

        try {
            val projectName = "Project ${System.currentTimeMillis()}"
            Timber.i("Creating new project: $projectName")
            Toast.makeText(this, "Project created: $projectName", Toast.LENGTH_SHORT).show()

            // TODO: Navigate to editor with project info
            // For now, just log successful engine interaction
            val clipCount = NativeLib.getClipCount(enginePtr)
            Timber.i("Engine is ready. Current clips: $clipCount")
        } catch (e: Exception) {
            Timber.e(e, "Error creating project")
            Toast.makeText(this, "Error: ${e.message}", Toast.LENGTH_SHORT).show()
        }
    }

    private fun navigateToImport() {
        val intent = android.content.Intent(this, ImportActivity::class.java)
        intent.putExtra("enginePtr", enginePtr)
        startActivity(intent)
    }

    private fun navigateToRecord() {
        Toast.makeText(this, "Recording feature coming soon", Toast.LENGTH_SHORT).show()
    }

    private fun navigateToTemplates() {
        Toast.makeText(this, "Templates feature coming soon", Toast.LENGTH_SHORT).show()
    }

    override fun onDestroy() {
        destroyEngine()
        super.onDestroy()
    }
}
