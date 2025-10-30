package com.ucworks.clipforge.data.models

data class Clip(
    val id: String,
    val projectId: String,
    val filePath: String,
    val thumbnailPath: String,
    val startTime: Long,
    val endTime: Long,
    val duration: Long,
    val isSelected: Boolean = false
)