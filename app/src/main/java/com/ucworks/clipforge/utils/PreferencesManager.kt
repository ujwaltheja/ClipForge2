package com.ucworks.clipforge.utils

import android.content.Context
import android.content.SharedPreferences
import androidx.core.content.edit
import timber.log.Timber

/**
 * PreferencesManager - Wrapper around SharedPreferences
 * Provides type-safe access to app preferences
 */
class PreferencesManager(context: Context) {

    private val prefs: SharedPreferences = context.getSharedPreferences(
        "clipforge_prefs",
        Context.MODE_PRIVATE
    )

    companion object {
        // Keys
        private const val KEY_APP_VERSION = "app_version"
        private const val KEY_FIRST_LAUNCH = "first_launch"
        private const val KEY_DARK_MODE = "dark_mode"
        private const val KEY_NOTIFICATIONS = "notifications"
        private const val KEY_AUTO_RESOLUTION = "auto_resolution"
        private const val KEY_SAMPLE_RATE = "sample_rate"
        private const val KEY_RENDER_THREADS = "render_threads"
        private const val KEY_LAST_PROJECT_ID = "last_project_id"
        private const val KEY_RECENT_PROJECTS = "recent_projects"
    }

    // App preferences

    fun getAppVersion(): String {
        return prefs.getString(KEY_APP_VERSION, "1.0.0") ?: "1.0.0"
    }

    fun setAppVersion(version: String) {
        prefs.edit { putString(KEY_APP_VERSION, version) }
        Timber.d("App version set: $version")
    }

    fun isFirstLaunch(): Boolean {
        return prefs.getBoolean(KEY_FIRST_LAUNCH, true)
    }

    fun setFirstLaunch(isFirst: Boolean) {
        prefs.edit { putBoolean(KEY_FIRST_LAUNCH, isFirst) }
        Timber.d("First launch flag set: $isFirst")
    }

    // UI preferences

    fun isDarkModeEnabled(): Boolean {
        return prefs.getBoolean(KEY_DARK_MODE, true)
    }

    fun setDarkModeEnabled(enabled: Boolean) {
        prefs.edit { putBoolean(KEY_DARK_MODE, enabled) }
        Timber.d("Dark mode: $enabled")
    }

    fun areNotificationsEnabled(): Boolean {
        return prefs.getBoolean(KEY_NOTIFICATIONS, true)
    }

    fun setNotificationsEnabled(enabled: Boolean) {
        prefs.edit { putBoolean(KEY_NOTIFICATIONS, enabled) }
        Timber.d("Notifications: $enabled")
    }

    // Video preferences

    fun isAutoResolutionEnabled(): Boolean {
        return prefs.getBoolean(KEY_AUTO_RESOLUTION, true)
    }

    fun setAutoResolutionEnabled(enabled: Boolean) {
        prefs.edit { putBoolean(KEY_AUTO_RESOLUTION, enabled) }
        Timber.d("Auto resolution: $enabled")
    }

    fun getRenderThreads(): Int {
        return prefs.getInt(KEY_RENDER_THREADS, 0)  // 0 = auto
    }

    fun setRenderThreads(threads: Int) {
        prefs.edit { putInt(KEY_RENDER_THREADS, threads) }
        Timber.d("Render threads: $threads")
    }

    // Audio preferences

    fun getSampleRate(): String {
        return prefs.getString(KEY_SAMPLE_RATE, "48000") ?: "48000"
    }

    fun setSampleRate(rate: String) {
        prefs.edit { putString(KEY_SAMPLE_RATE, rate) }
        Timber.d("Sample rate: $rate")
    }

    // Project preferences

    fun getLastProjectId(): String? {
        return prefs.getString(KEY_LAST_PROJECT_ID, null)
    }

    fun setLastProjectId(projectId: String) {
        prefs.edit { putString(KEY_LAST_PROJECT_ID, projectId) }
        Timber.d("Last project ID: $projectId")
    }

    fun getRecentProjects(): Set<String> {
        return prefs.getStringSet(KEY_RECENT_PROJECTS, emptySet()) ?: emptySet()
    }

    fun addRecentProject(projectId: String) {
        val recent = getRecentProjects().toMutableSet()
        recent.add(projectId)
        if (recent.size > 10) {  // Keep only 10 most recent
            recent.remove(recent.first())
        }
        prefs.edit { putStringSet(KEY_RECENT_PROJECTS, recent) }
        Timber.d("Recent projects updated: ${recent.size} items")
    }

    fun removeRecentProject(projectId: String) {
        val recent = getRecentProjects().toMutableSet()
        recent.remove(projectId)
        prefs.edit { putStringSet(KEY_RECENT_PROJECTS, recent) }
        Timber.d("Project removed from recent")
    }

    // Generic preferences

    fun putString(key: String, value: String) {
        prefs.edit { putString(key, value) }
        Timber.d("Preference saved: $key")
    }

    fun getString(key: String, default: String = ""): String {
        return prefs.getString(key, default) ?: default
    }

    fun putInt(key: String, value: Int) {
        prefs.edit { putInt(key, value) }
        Timber.d("Preference saved: $key")
    }

    fun getInt(key: String, default: Int = 0): Int {
        return prefs.getInt(key, default)
    }

    fun putLong(key: String, value: Long) {
        prefs.edit { putLong(key, value) }
        Timber.d("Preference saved: $key")
    }

    fun getLong(key: String, default: Long = 0L): Long {
        return prefs.getLong(key, default)
    }

    fun putBoolean(key: String, value: Boolean) {
        prefs.edit { putBoolean(key, value) }
        Timber.d("Preference saved: $key")
    }

    fun getBoolean(key: String, default: Boolean = false): Boolean {
        return prefs.getBoolean(key, default)
    }

    fun putFloat(key: String, value: Float) {
        prefs.edit { putFloat(key, value) }
        Timber.d("Preference saved: $key")
    }

    fun getFloat(key: String, default: Float = 0f): Float {
        return prefs.getFloat(key, default)
    }

    // Utility methods

    fun remove(key: String) {
        prefs.edit { remove(key) }
        Timber.d("Preference removed: $key")
    }

    fun clear() {
        prefs.edit { clear() }
        Timber.d("All preferences cleared")
    }

    fun contains(key: String): Boolean {
        return prefs.contains(key)
    }

    fun getAll(): Map<String, *> {
        return prefs.all
    }

    fun exportPreferences(): Map<String, *> {
        return getAll()
    }

    fun printAllPreferences() {
        Timber.d("=== All Preferences ===")
        getAll().forEach { (key, value) ->
            Timber.d("$key: $value")
        }
        Timber.d("=== End Preferences ===")
    }
}
