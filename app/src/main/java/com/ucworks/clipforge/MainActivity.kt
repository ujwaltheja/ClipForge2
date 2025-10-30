package com.ucworks.clipforge

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.lifecycle.ViewModelProvider
import androidx.recyclerview.widget.GridLayoutManager
import com.ucworks.clipforge.databinding.ActivityMainBinding
import com.ucworks.clipforge.ui.adapters.ProjectAdapter
import com.ucworks.clipforge.ui.dialogs.NewProjectDialog
import com.ucworks.clipforge.ui.viewmodels.MainViewModel
import com.google.android.material.snackbar.Snackbar
import com.ucworks.clipforge.data.models.Project
import com.ucworks.clipforge.EditorActivity
import com.ucworks.clipforge.ui.SettingsActivity

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private lateinit var viewModel: MainViewModel
    private lateinit var projectAdapter: ProjectAdapter
    
    private val permissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val allGranted = permissions.values.all { it }
        if (allGranted) {
            loadProjects()
        } else {
            showPermissionDeniedMessage()
        }
    }
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        viewModel = ViewModelProvider(this)[MainViewModel::class.java]

        setupRecyclerView()
        setupFAB()
        checkPermissions()
        observeProjects()
    }
    
    private fun setupRecyclerView() {
        projectAdapter = ProjectAdapter(
            onProjectClick = { project ->
                openEditor(project)
            },
            onProjectLongClick = { project ->
                showProjectOptions(project)
            }
        )

        binding.recentProjectsGrid.apply {
            layoutManager = GridLayoutManager(this@MainActivity, 2)
            adapter = projectAdapter
            setHasFixedSize(true)
        }
    }
    
    private fun setupFAB() {
        binding.newProjectButton.setOnClickListener {
            NewProjectDialog { projectName, template ->
                viewModel.createProject(projectName, template)
                // Navigate to editor
            }.show(supportFragmentManager, "new_project")
        }
    }
    
    private fun checkPermissions() {
        val permissions = mutableListOf<String>()
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            permissions.add(Manifest.permission.READ_MEDIA_VIDEO)
            permissions.add(Manifest.permission.READ_MEDIA_AUDIO)
            permissions.add(Manifest.permission.READ_MEDIA_IMAGES)
        } else {
            permissions.add(Manifest.permission.READ_EXTERNAL_STORAGE)
            permissions.add(Manifest.permission.WRITE_EXTERNAL_STORAGE)
        }
        
        permissions.add(Manifest.permission.RECORD_AUDIO)
        
        val notGranted = permissions.filter {
            ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }
        
        if (notGranted.isNotEmpty()) {
            permissionLauncher.launch(notGranted.toTypedArray())
        } else {
            loadProjects()
        }
    }
    
    private fun loadProjects() {
        viewModel.loadProjects()
    }
    
    private fun observeProjects() {
        viewModel.projects.observe(this) { projects ->
            projectAdapter.submitList(projects)
        }
    }
    
    private fun openEditor(project: Project) {
        startActivity(Intent(this, EditorActivity::class.java).apply {
            putExtra("project_id", project.id)
        })
    }
    
    private fun showProjectOptions(project: Project) {
        // Show bottom sheet with options: Edit, Duplicate, Delete, Export
    }
    
    private fun showPermissionDeniedMessage() {
        Snackbar.make(
            binding.root,
            "Permissions are required to access media files",
            Snackbar.LENGTH_LONG
        ).setAction("Settings") {
            // Open app settings
        }.show()
    }
    
    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }
    
    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_settings -> {
                startActivity(Intent(this, SettingsActivity::class.java))
                true
            }
            R.id.action_help -> {
                // startActivity(Intent(this, HelpActivity::class.java))
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }
}