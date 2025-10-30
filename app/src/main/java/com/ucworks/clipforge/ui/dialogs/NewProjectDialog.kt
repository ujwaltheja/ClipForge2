package com.ucworks.clipforge.ui.dialogs

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import androidx.fragment.app.DialogFragment
import com.ucworks.clipforge.data.models.ProjectTemplate
import com.ucworks.clipforge.databinding.DialogNewProjectBinding

class NewProjectDialog(private val onProjectCreated: (String, ProjectTemplate) -> Unit) : DialogFragment() {

    private lateinit var binding: DialogNewProjectBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = DialogNewProjectBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        val templateNames = ProjectTemplate.values().map { it.displayName }
        val adapter = ArrayAdapter(requireContext(), android.R.layout.simple_spinner_item, templateNames)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        binding.templateSpinner.adapter = adapter

        binding.createButton.setOnClickListener {
            val projectName = binding.projectNameEditText.text.toString()
            val selectedTemplate = ProjectTemplate.values()[binding.templateSpinner.selectedItemPosition]
            onProjectCreated(projectName, selectedTemplate)
            dismiss()
        }

        binding.cancelButton.setOnClickListener {
            dismiss()
        }
    }
}