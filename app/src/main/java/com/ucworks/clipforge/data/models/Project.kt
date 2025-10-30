package com.ucworks.clipforge.data.models

import android.os.Parcelable
import kotlinx.parcelize.Parcelize
import java.util.Date

@Parcelize
data class Project(
    val id: String,
    val name: String,
    val template: ProjectTemplate,
    val createdAt: Date,
    val updatedAt: Date,
    val thumbnailPath: String?,
    val duration: Long,
    val clipCount: Int
) : Parcelable
