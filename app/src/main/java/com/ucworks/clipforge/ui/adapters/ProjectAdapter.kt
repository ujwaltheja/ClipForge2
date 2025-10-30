package com.ucworks.clipforge.ui.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.ucworks.clipforge.R
import com.ucworks.clipforge.databinding.ItemProjectBinding
import com.ucworks.clipforge.data.models.Project
import java.text.SimpleDateFormat
import java.util.Locale

class ProjectAdapter(
    private val onProjectClick: (Project) -> Unit,
    private val onProjectLongClick: (Project) -> Unit
) : ListAdapter<Project, ProjectAdapter.ProjectViewHolder>(ProjectDiffCallback()) {
    
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ProjectViewHolder {
        val binding = ItemProjectBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return ProjectViewHolder(binding, onProjectClick, onProjectLongClick)
    }
    
    override fun onBindViewHolder(holder: ProjectViewHolder, position: Int) {
        holder.bind(getItem(position))
    }
    
    class ProjectViewHolder(
        private val binding: ItemProjectBinding,
        private val onProjectClick: (Project) -> Unit,
        private val onProjectLongClick: (Project) -> Unit
    ) : RecyclerView.ViewHolder(binding.root) {
        
        private val dateFormat = SimpleDateFormat("MMM dd, yyyy", Locale.getDefault())
        
        fun bind(project: Project) {
            binding.apply {
                tvProjectName.text = project.name
                tvProjectInfo.text = "${project.template.displayName} • ${project.clipCount} clips"
                tvProjectDate.text = dateFormat.format(project.updatedAt)
                
                // Load thumbnail
                if (project.thumbnailPath != null) {
                    Glide.with(binding.root.context)
                        .load(project.thumbnailPath)
                        .placeholder(R.drawable.ic_video_placeholder)
                        .into(ivThumbnail)
                } else {
                    ivThumbnail.setImageResource(R.drawable.ic_video_placeholder)
                }
                
                root.setOnClickListener { onProjectClick(project) }
                root.setOnLongClickListener { 
                    onProjectLongClick(project)
                    true
                }
            }
        }
    }
    
    class ProjectDiffCallback : DiffUtil.ItemCallback<Project>() {
        override fun areItemsTheSame(oldItem: Project, newItem: Project): Boolean {
            return oldItem.id == newItem.id
        }
        
        override fun areContentsTheSame(oldItem: Project, newItem: Project): Boolean {
            return oldItem == newItem
        }
    }
}