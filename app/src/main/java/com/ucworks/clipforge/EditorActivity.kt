package com.ucworks.clipforge

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.ViewModelProvider
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.LinearLayoutManager
import com.ucworks.clipforge.databinding.ActivityEditorBinding
import com.ucworks.clipforge.ui.adapters.EffectsAdapter
import com.ucworks.clipforge.ui.adapters.TimelineAdapter
import com.ucworks.clipforge.ui.viewmodels.EditorViewModel
import com.google.android.material.bottomsheet.BottomSheetBehavior

class EditorActivity : AppCompatActivity() {
    private lateinit var binding: ActivityEditorBinding
    private lateinit var viewModel: EditorViewModel
    private lateinit var timelineAdapter: TimelineAdapter
    private lateinit var effectsAdapter: EffectsAdapter
    private lateinit var bottomSheetBehavior: BottomSheetBehavior<View>
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityEditorBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        viewModel = ViewModelProvider(this)[EditorViewModel::class.java]
        
        setupToolbar()
        setupTimeline()
        setupPreview()
        setupControls()
        setupEffects()
        observeViewModel()
    }
    
    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = viewModel.projectName.value
        
        binding.btnUndo.setOnClickListener { viewModel.undo() }
        binding.btnRedo.setOnClickListener { viewModel.redo() }
        binding.btnExport.setOnClickListener { showExportDialog() }
    }
    
    private fun setupTimeline() {
        timelineAdapter = TimelineAdapter(
            onClipClick = { clip -> viewModel.selectClip(clip) },
            onClipMove = { clip, position -> viewModel.moveClip(clip, position) },
            onClipTrim = { clip, startTime, endTime ->
                viewModel.trimClip(clip, startTime, endTime)
            }
        )

        binding.timelineRecycler.apply {
            layoutManager = LinearLayoutManager(
                this@EditorActivity,
                LinearLayoutManager.HORIZONTAL,
                false
            )
            adapter = timelineAdapter
        }
    }
    
    private fun setupPreview() {
        binding.previewView.setOnClickListener {
            viewModel.togglePlayback()
        }
        
        binding.btnPlay.setOnClickListener {
            viewModel.togglePlayback()
        }
    }
    
    private fun setupControls() {
        binding.btnImport.setOnClickListener {
            showMediaPicker()
        }
        
        binding.btnSplit.setOnClickListener {
            viewModel.splitClipAtCurrentPosition()
        }
        
        binding.btnDelete.setOnClickListener {
            viewModel.deleteSelectedClip()
        }
        
        binding.btnSpeed.setOnClickListener {
            showSpeedDialog()
        }
        
        binding.btnVolume.setOnClickListener {
            showVolumeDialog()
        }
    }
    
    private fun setupEffects() {
        bottomSheetBehavior = BottomSheetBehavior.from(binding.effectsBottomSheet)
        bottomSheetBehavior.state = BottomSheetBehavior.STATE_HIDDEN
        
        effectsAdapter = EffectsAdapter { effect ->
            viewModel.applyEffect(effect.id)
        }
        
        binding.effectsRecycler.apply {
            layoutManager = GridLayoutManager(this@EditorActivity, 3)
            adapter = effectsAdapter
        }
        
        binding.btnEffects.setOnClickListener {
            if (bottomSheetBehavior.state == BottomSheetBehavior.STATE_HIDDEN) {
                bottomSheetBehavior.state = BottomSheetBehavior.STATE_EXPANDED
            } else {
                bottomSheetBehavior.state = BottomSheetBehavior.STATE_HIDDEN
            }
        }
    }
    
    private fun observeViewModel() {
        viewModel.clips.observe(this) { clips ->
            timelineAdapter.submitList(clips)
        }
        
        viewModel.selectedClip.observe(this) { clip ->
            // updateSelectionUI(clip)
        }
        
        viewModel.isPlaying.observe(this) { isPlaying ->
            binding.btnPlay.setImageResource(
                if (isPlaying) R.drawable.ic_pause else R.drawable.ic_play
            )
        }
        
        viewModel.currentPosition.observe(this) { position ->
            binding.tvCurrentTime.text = formatTime(position)
        }
        
        viewModel.fps.observe(this) { fps ->
            binding.tvFps.text = "FPS: $fps"
        }
        
        viewModel.error.observe(this) { error ->
            // showError(error)
        }
    }
    
    private fun formatTime(millis: Long): String {
        val seconds = (millis / 1000) % 60
        val minutes = (millis / 60000) % 60
        val hours = millis / 3600000
        return if (hours > 0) {
            String.format("%d:%02d:%02d", hours, minutes, seconds)
        } else {
            String.format("%d:%02d", minutes, seconds)
        }
    }
    
    override fun onSupportNavigateUp(): Boolean {
        onBackPressed()
        return true
    }

    private fun showExportDialog() {}
    private fun showMediaPicker() {}
    private fun showSpeedDialog() {}
    private fun showVolumeDialog() {}
}