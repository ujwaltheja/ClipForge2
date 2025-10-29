package com.ucworks.clipforge.fragments

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.ucworks.clipforge.AIAssistantActivity
import com.ucworks.clipforge.EditorActivity
import com.ucworks.clipforge.ImportActivity
import com.ucworks.clipforge.R
import com.ucworks.clipforge.SettingsActivity
import com.ucworks.clipforge.adapters.RecentProjectsAdapter
import com.ucworks.clipforge.databinding.FragmentMainBinding
import com.ucworks.clipforge.models.RecentProject
import kotlinx.coroutines.launch
import timber.log.Timber
import java.util.UUID

/**
 * Main/Home Fragment - Displays recent projects and entry points.
 * Provides quick access to:
 * - New project creation
 * - Recent projects list
 * - Import/record/templates shortcuts
 * - Navigation to settings and AI assistant
 */
class MainFragment : Fragment() {

    private var _binding: FragmentMainBinding? = null
    private val binding get() = _binding!!
    private lateinit var projectsAdapter: RecentProjectsAdapter
    private var enginePtr: Long = 0L

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentMainBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        setHasOptionsMenu(true)

        setupRecyclerView()
        setupActionButtons()
        loadRecentProjects()
    }

    private fun setupRecyclerView() {
        projectsAdapter = RecentProjectsAdapter(
            onProjectClick = { project ->
                Timber.d("Opening project: ${project.name}")
                openProject(project)
            },
            onProjectLongClick = { project ->
                Timber.d("Project long clicked: ${project.name}")
                showProjectContextMenu(project)
                true
            }
        )

        binding.recentProjects.apply {
            layoutManager = LinearLayoutManager(requireContext())
            adapter = projectsAdapter
            setHasFixedSize(true)
        }
    }

    private fun setupActionButtons() {
        // New Project Button
        binding.btnNewProject.setOnClickListener {
            Timber.d("New project button clicked")
            showNewProjectDialog()
        }

        // Import Button
        binding.btnImport.setOnClickListener {
            Timber.d("Import button clicked")
            startActivity(Intent(requireContext(), ImportActivity::class.java))
        }

        // AI Assistant Button
        binding.btnAI.setOnClickListener {
            Timber.d("AI Assistant button clicked")
            startActivity(Intent(requireContext(), AIAssistantActivity::class.java).apply {
                putExtra("enginePtr", enginePtr)
            })
        }
    }

    private fun loadRecentProjects() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading recent projects")
                // In real app, would load from database or SharedPreferences
                // For now, create mock recent projects
                val mockProjects = listOf(
                    RecentProject(
                        id = "proj1",
                        name = "Summer Vlog",
                        thumbnailPath = null,
                        duration = "02:45",
                        resolution = "1920x1080@30fps",
                        lastModified = System.currentTimeMillis() - 3600000,  // 1 hour ago
                        clipCount = 8
                    ),
                    RecentProject(
                        id = "proj2",
                        name = "Birthday Party",
                        thumbnailPath = null,
                        duration = "05:30",
                        resolution = "1920x1080@30fps",
                        lastModified = System.currentTimeMillis() - 86400000,  // 1 day ago
                        clipCount = 12
                    ),
                    RecentProject(
                        id = "proj3",
                        name = "Travel Montage",
                        thumbnailPath = null,
                        duration = "01:15",
                        resolution = "1920x1080@30fps",
                        lastModified = System.currentTimeMillis() - 172800000,  // 2 days ago
                        clipCount = 5
                    )
                )

                projectsAdapter.submitList(mockProjects)
                Timber.d("Loaded ${mockProjects.size} recent projects")

                // Update stats
                updateProjectStats(mockProjects)
            } catch (e: Exception) {
                Timber.e(e, "Error loading recent projects")
            }
        }
    }

    private fun updateProjectStats(projects: List<RecentProject>) {
        val totalClips = projects.sumOf { it.clipCount }
        val totalDuration = calculateTotalDuration(projects)
        binding.projectStats.text = "$totalClips total clips • $totalDuration total duration"
    }

    private fun calculateTotalDuration(projects: List<RecentProject>): String {
        // Simple calculation - in real app would parse time strings
        return "${projects.size} projects"
    }

    private fun showNewProjectDialog() {
        Timber.d("Showing new project dialog")
        // Show dialog to select project template
        val templates = listOf(
            "1080p 30fps",
            "720p 30fps",
            "1080p 60fps",
            "4K 30fps",
            "Instagram Story",
            "TikTok",
            "YouTube"
        )

        android.app.AlertDialog.Builder(requireContext())
            .setTitle("Create New Project")
            .setItems(templates.toTypedArray()) { _, which ->
                val template = templates[which]
                Timber.d("Creating project with template: $template")
                createNewProject(template)
            }
            .setNegativeButton("Cancel") { dialog, _ ->
                dialog.dismiss()
            }
            .show()
    }

    private fun createNewProject(template: String) {
        lifecycleScope.launch {
            try {
                Timber.d("Creating new project: $template")
                // In real app, would initialize project in native engine
                val projectId = UUID.randomUUID().toString()
                Timber.d("Project created with ID: $projectId")

                // Navigate to import screen
                startActivity(Intent(requireContext(), ImportActivity::class.java).apply {
                    putExtra("projectId", projectId)
                })
            } catch (e: Exception) {
                Timber.e(e, "Error creating project")
            }
        }
    }

    private fun openProject(project: RecentProject) {
        lifecycleScope.launch {
            try {
                Timber.d("Opening project: ${project.name}")
                // In real app, would load project from storage and initialize engine
                startActivity(Intent(requireContext(), EditorActivity::class.java).apply {
                    putExtra("projectId", project.id)
                    putExtra("enginePtr", enginePtr)
                })
            } catch (e: Exception) {
                Timber.e(e, "Error opening project")
            }
        }
    }

    private fun showProjectContextMenu(project: RecentProject) {
        val options = arrayOf("Edit", "Delete", "Duplicate", "Share")
        android.app.AlertDialog.Builder(requireContext())
            .setTitle(project.name)
            .setItems(options) { _, which ->
                when (which) {
                    0 -> openProject(project)  // Edit
                    1 -> deleteProject(project)  // Delete
                    2 -> duplicateProject(project)  // Duplicate
                    3 -> shareProject(project)  // Share
                }
            }
            .show()
    }

    private fun deleteProject(project: RecentProject) {
        Timber.d("Deleting project: ${project.name}")
        android.app.AlertDialog.Builder(requireContext())
            .setTitle("Delete Project?")
            .setMessage("This action cannot be undone.")
            .setPositiveButton("Delete") { _, _ ->
                Timber.d("Project deleted: ${project.name}")
                // Reload projects list
                loadRecentProjects()
            }
            .setNegativeButton("Cancel") { dialog, _ ->
                dialog.dismiss()
            }
            .show()
    }

    private fun duplicateProject(project: RecentProject) {
        Timber.d("Duplicating project: ${project.name}")
        // Create copy and reload
        loadRecentProjects()
    }

    private fun shareProject(project: RecentProject) {
        Timber.d("Sharing project: ${project.name}")
        val shareIntent = Intent().apply {
            action = Intent.ACTION_SEND
            type = "text/plain"
            putExtra(Intent.EXTRA_SUBJECT, "Check out my video: ${project.name}")
            putExtra(Intent.EXTRA_TEXT, "Created with ClipForge")
        }
        startActivity(Intent.createChooser(shareIntent, "Share project"))
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_main, menu)
        super.onCreateOptionsMenu(menu, inflater)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_settings -> {
                Timber.d("Settings menu clicked")
                startActivity(Intent(requireContext(), SettingsActivity::class.java))
                true
            }
            R.id.action_about -> {
                Timber.d("About menu clicked")
                showAboutDialog()
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    private fun showAboutDialog() {
        android.app.AlertDialog.Builder(requireContext())
            .setTitle("About ClipForge")
            .setMessage("ClipForge v1.0.0\n\nProfessional Video Editor\n\n© 2025 UCWorks\n\nBuilt with C++ NDK and Kotlin")
            .setPositiveButton("OK") { dialog, _ ->
                dialog.dismiss()
            }
            .show()
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
