package com.ucworks.clipforge.fragments

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.provider.MediaStore
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.GridLayoutManager
import com.google.android.material.tabs.TabLayout
import com.ucworks.clipforge.NativeLib
import com.ucworks.clipforge.adapters.MediaGridAdapter
import com.ucworks.clipforge.databinding.FragmentImportBinding
import com.ucworks.clipforge.models.MediaItem
import kotlinx.coroutines.launch
import timber.log.Timber
import java.util.UUID

/**
 * Import Fragment - Allows users to select and import media from multiple sources.
 * Supports:
 * - Device library (existing videos/audio)
 * - Camera capture
 * - Photo library
 */
class ImportFragment : Fragment() {

    private var _binding: FragmentImportBinding? = null
    private val binding get() = _binding!!
    private lateinit var mediaAdapter: MediaGridAdapter
    private var enginePtr: Long = 0L
    private var currentTab = 0

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val videoGranted = permissions[Manifest.permission.READ_MEDIA_VIDEO] ?: false
        val audioGranted = permissions[Manifest.permission.READ_MEDIA_AUDIO] ?: false

        if (videoGranted || audioGranted) {
            Timber.d("Media permissions granted")
            loadDeviceMedia()
        } else {
            Timber.w("Media permissions denied")
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentImportBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Get engine pointer from arguments
        arguments?.let {
            enginePtr = it.getLong("enginePtr", 0L)
        }

        setupMediaAdapter()
        setupTabLayout()
        setupActionButtons()
        checkPermissionsAndLoad()
    }

    private fun setupMediaAdapter() {
        mediaAdapter = MediaGridAdapter(
            onItemClick = { item ->
                Timber.d("Media item clicked: ${item.displayName}")
            },
            onItemLongClick = { item ->
                Timber.d("Media item long clicked: ${item.displayName}")
                true
            }
        )

        binding.mediaGrid.apply {
            layoutManager = GridLayoutManager(requireContext(), 2)
            adapter = mediaAdapter
            setHasFixedSize(false)
        }
    }

    private fun setupTabLayout() {
        binding.tabLayout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {
            override fun onTabSelected(tab: TabLayout.Tab?) {
                currentTab = tab?.position ?: 0
                Timber.d("Tab selected: $currentTab")
                when (currentTab) {
                    0 -> loadDeviceMedia()
                    1 -> loadCameraMedia()
                    2 -> loadLibraryMedia()
                }
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {}
            override fun onTabReselected(tab: TabLayout.Tab?) {}
        })
    }

    private fun setupActionButtons() {
        binding.btnCancel.setOnClickListener {
            Timber.d("Import cancelled")
            requireActivity().onBackPressed()
        }

        binding.btnImport.setOnClickListener {
            Timber.d("Import button clicked")
            importSelectedMedia()
        }
    }

    private fun checkPermissionsAndLoad() {
        val requiredPermissions = mutableListOf(
            Manifest.permission.READ_MEDIA_VIDEO,
            Manifest.permission.READ_MEDIA_AUDIO
        )

        val missingPermissions = requiredPermissions.filter {
            ContextCompat.checkSelfPermission(requireContext(), it) != PackageManager.PERMISSION_GRANTED
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
                val mediaItems = mutableListOf<MediaItem>()

                // Query videos
                val videoCursor = requireContext().contentResolver.query(
                    MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                    arrayOf(
                        MediaStore.Video.Media._ID,
                        MediaStore.Video.Media.DISPLAY_NAME,
                        MediaStore.Video.Media.DURATION,
                        MediaStore.Video.Media.SIZE,
                        MediaStore.Video.Media.DATE_ADDED
                    ),
                    null,
                    null,
                    "${MediaStore.Video.Media.DATE_ADDED} DESC"
                )

                videoCursor?.use { cursor ->
                    val idIndex = cursor.getColumnIndexOrThrow(MediaStore.Video.Media._ID)
                    val nameIndex = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME)
                    val durationIndex = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION)
                    val sizeIndex = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE)
                    val dateIndex = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATE_ADDED)

                    while (cursor.moveToNext()) {
                        val id = cursor.getLong(idIndex)
                        val name = cursor.getString(nameIndex)
                        val duration = cursor.getLong(durationIndex)
                        val size = cursor.getLong(sizeIndex)
                        val date = cursor.getLong(dateIndex)

                        val uri = android.net.Uri.withAppendedPath(
                            MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                            id.toString()
                        )

                        mediaItems.add(
                            MediaItem(
                                id = id.toString(),
                                displayName = name,
                                uri = uri,
                                duration = duration,
                                size = size,
                                mimeType = "video/*",
                                dateAdded = date,
                                isVideo = true
                            )
                        )
                    }
                }

                Timber.d("Found ${mediaItems.size} videos")
                mediaAdapter.submitList(mediaItems)
            } catch (e: Exception) {
                Timber.e(e, "Error loading device media")
            }
        }
    }

    private fun loadCameraMedia() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading camera media")
                // Would load from camera app or recent captures
                mediaAdapter.submitList(emptyList())
            } catch (e: Exception) {
                Timber.e(e, "Error loading camera media")
            }
        }
    }

    private fun loadLibraryMedia() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading library media")
                // Would load from custom library or cloud storage
                mediaAdapter.submitList(emptyList())
            } catch (e: Exception) {
                Timber.e(e, "Error loading library media")
            }
        }
    }

    private fun importSelectedMedia() {
        lifecycleScope.launch {
            try {
                val selectedItems = mediaAdapter.getSelectedItems()
                Timber.d("Importing ${selectedItems.size} items")

                if (selectedItems.isEmpty()) {
                    Timber.w("No items selected for import")
                    return@launch
                }

                // Add each selected item to engine
                for ((index, item) in selectedItems.withIndex()) {
                    Timber.d("Adding clip ${index + 1}/${selectedItems.size}: ${item.displayName}")
                    val clipId = NativeLib.addClip(
                        enginePtr,
                        item.uri.toString(),
                        index * 30000L,  // 30 seconds apart
                        0
                    )
                    Timber.d("Clip added: $clipId")
                }

                Timber.d("All clips imported successfully")
                requireActivity().onBackPressed()
            } catch (e: Exception) {
                Timber.e(e, "Error importing media")
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
