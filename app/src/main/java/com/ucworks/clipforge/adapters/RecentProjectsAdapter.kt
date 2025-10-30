package com.ucworks.clipforge.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.ucworks.clipforge.databinding.ItemProjectBinding
import com.ucworks.clipforge.models.RecentProject
import timber.log.Timber

/**
 * RecyclerView adapter for displaying recent projects.
 */
class RecentProjectsAdapter(
    private val onProjectClick: (RecentProject) -> Unit,
    private val onProjectLongClick: (RecentProject) -> Boolean
) : ListAdapter<RecentProject, RecentProjectsAdapter.ProjectViewHolder>(ProjectDiffCallback()) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ProjectViewHolder {
        val binding = ItemProjectBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return ProjectViewHolder(binding)
    }

    override fun onBindViewHolder(holder: ProjectViewHolder, position: Int) {
        val project = getItem(position)
        holder.bind(project)
    }

    inner class ProjectViewHolder(
        private val binding: ItemProjectBinding
    ) : RecyclerView.ViewHolder(binding.root) {

        fun bind(project: RecentProject) {
            with(binding) {
                tvProjectName.text = project.name
                tvProjectInfo.text = "${project.resolution} • ${project.clipCount} clips"
                tvProjectDate.text = "Modified: ${formatTime(project.lastModified)}"

                // Click listeners
                root.setOnClickListener {
                    Timber.d("Project clicked: ${project.name}")
                    onProjectClick(project)
                }

                root.setOnLongClickListener {
                    Timber.d("Project long clicked: ${project.name}")
                    onProjectLongClick(project)
                }
            }
        }

        private fun formatTime(timestamp: Long): String {
            val diff = System.currentTimeMillis() - timestamp
            return when {
                diff < 60000 -> "just now"
                diff < 3600000 -> "${diff / 60000}m ago"
                diff < 86400000 -> "${diff / 3600000}h ago"
                else -> "${diff / 86400000}d ago"
            }
        }
    }

    private class ProjectDiffCallback : DiffUtil.ItemCallback<RecentProject>() {
        override fun areItemsTheSame(oldItem: RecentProject, newItem: RecentProject): Boolean {
            return oldItem.id == newItem.id
        }

        override fun areContentsTheSame(oldItem: RecentProject, newItem: RecentProject): Boolean {
            return oldItem == newItem
        }
    }
}
