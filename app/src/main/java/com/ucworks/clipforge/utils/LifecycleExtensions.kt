package com.ucworks.clipforge.utils

import androidx.fragment.app.Fragment
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Lifecycle Extension Functions
 * Provides common lifecycle operations as extension methods
 */

/**
 * Launch a coroutine that repeats while fragment is in STARTED state
 * Safe way to collect flows in fragments
 */
fun Fragment.launchRepeatOnLifecycle(
    state: Lifecycle.State = Lifecycle.State.STARTED,
    block: suspend () -> Unit
) {
    viewLifecycleOwner.lifecycleScope.launch {
        viewLifecycleOwner.repeatOnLifecycle(state) {
            try {
                block()
            } catch (e: Exception) {
                Timber.e(e, "Error in lifecycle coroutine")
            }
        }
    }
}

/**
 * Execute a block when lifecycle reaches CREATED state
 */
fun Fragment.onLifecycleCreated(block: () -> Unit) {
    viewLifecycleOwner.lifecycleScope.launch {
        if (viewLifecycleOwner.lifecycle.currentState.isAtLeast(Lifecycle.State.CREATED)) {
            block()
            Timber.d("Executed onLifecycleCreated block")
        }
    }
}

/**
 * Execute a block when lifecycle reaches STARTED state
 */
fun Fragment.onLifecycleStarted(block: () -> Unit) {
    viewLifecycleOwner.lifecycleScope.launch {
        if (viewLifecycleOwner.lifecycle.currentState.isAtLeast(Lifecycle.State.STARTED)) {
            block()
            Timber.d("Executed onLifecycleStarted block")
        }
    }
}

/**
 * Execute a block when lifecycle reaches RESUMED state
 */
fun Fragment.onLifecycleResumed(block: () -> Unit) {
    viewLifecycleOwner.lifecycleScope.launch {
        if (viewLifecycleOwner.lifecycle.currentState.isAtLeast(Lifecycle.State.RESUMED)) {
            block()
            Timber.d("Executed onLifecycleResumed block")
        }
    }
}

/**
 * Check if fragment is in active state
 */
fun Fragment.isLifecycleActive(): Boolean {
    return viewLifecycleOwner.lifecycle.currentState.isAtLeast(Lifecycle.State.STARTED)
}

/**
 * Check if fragment is fully resumed
 */
fun Fragment.isLifecycleResumed(): Boolean {
    return viewLifecycleOwner.lifecycle.currentState == Lifecycle.State.RESUMED
}

/**
 * Safe lifecycle scope access for fragments
 */
fun Fragment.safeLifecycleScope(action: suspend () -> Unit) {
    if (isAdded) {
        viewLifecycleOwner.lifecycleScope.launch {
            try {
                action()
            } catch (e: Exception) {
                Timber.e(e, "Error in safe lifecycle scope")
            }
        }
    }
}

/**
 * Execute block and handle lifecycle-related exceptions
 */
fun Fragment.lifecycleSafe(block: suspend () -> Unit) {
    viewLifecycleOwner.lifecycleScope.launch {
        try {
            block()
        } catch (e: Exception) {
            when (e) {
                is IllegalStateException -> {
                    Timber.w(e, "Lifecycle exception - likely fragment not attached")
                }
                else -> {
                    Timber.e(e, "Error in lifecycle-safe block")
                }
            }
        }
    }
}

/**
 * Get human-readable lifecycle state
 */
fun Lifecycle.State.getDisplayName(): String {
    return when (this) {
        Lifecycle.State.DESTROYED -> "Destroyed"
        Lifecycle.State.INITIALIZED -> "Initialized"
        Lifecycle.State.CREATED -> "Created"
        Lifecycle.State.STARTED -> "Started"
        Lifecycle.State.RESUMED -> "Resumed"
    }
}
