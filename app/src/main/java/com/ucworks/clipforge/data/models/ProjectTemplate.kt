package com.ucworks.clipforge.data.models

enum class ProjectTemplate(
    val displayName: String,
    val width: Int,
    val height: Int,
    val fps: Int
) {
    HD_1080P("1080p HD", 1920, 1080, 30),
    HD_720P("720p HD", 1280, 720, 30),
    UHD_4K("4K Ultra HD", 3840, 2160, 30),
    INSTAGRAM("Instagram", 1080, 1080, 30),
    INSTAGRAM_STORY("Instagram Story", 1080, 1920, 30),
    TIKTOK("TikTok", 1080, 1920, 30),
    YOUTUBE("YouTube", 1920, 1080, 60)
}