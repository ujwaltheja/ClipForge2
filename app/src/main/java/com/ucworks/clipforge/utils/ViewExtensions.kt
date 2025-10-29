package com.ucworks.clipforge.utils

import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import timber.log.Timber

/**
 * View Extension Functions
 * Provides common View operations as extension methods
 */

// Visibility Extensions
fun View.visible() {
    if (visibility != View.VISIBLE) {
        visibility = View.VISIBLE
        Timber.d("View made visible: ${this.javaClass.simpleName}")
    }
}

fun View.gone() {
    if (visibility != View.GONE) {
        visibility = View.GONE
        Timber.d("View made gone: ${this.javaClass.simpleName}")
    }
}

fun View.invisible() {
    if (visibility != View.INVISIBLE) {
        visibility = View.INVISIBLE
        Timber.d("View made invisible: ${this.javaClass.simpleName}")
    }
}

fun View.setVisible(isVisible: Boolean) {
    if (isVisible) visible() else gone()
}

// Click Extensions
fun View.setOnClickListener(debounceTime: Long = 600, action: (View) -> Unit) {
    var lastClickTime = 0L
    setOnClickListener { view ->
        val currentTime = System.currentTimeMillis()
        if (currentTime - lastClickTime > debounceTime) {
            lastClickTime = currentTime
            action(view)
        } else {
            Timber.d("Click debounced: ${this.javaClass.simpleName}")
        }
    }
}

// Enable/Disable Extensions
fun View.enable() {
    if (!isEnabled) {
        isEnabled = true
        alpha = 1.0f
        Timber.d("View enabled: ${this.javaClass.simpleName}")
    }
}

fun View.disable() {
    if (isEnabled) {
        isEnabled = false
        alpha = 0.5f
        Timber.d("View disabled: ${this.javaClass.simpleName}")
    }
}

fun View.setEnabled(enabled: Boolean) {
    if (enabled) enable() else disable()
}

// Animation Extensions
fun View.fadeIn(duration: Long = 300) {
    alpha = 0f
    visible()
    animate()
        .alpha(1f)
        .setDuration(duration)
        .start()
    Timber.d("Fade in animation started")
}

fun View.fadeOut(duration: Long = 300) {
    animate()
        .alpha(0f)
        .setDuration(duration)
        .withEndAction { gone() }
        .start()
    Timber.d("Fade out animation started")
}

// Padding Extensions
fun View.setPadding(padding: Int) {
    setPadding(padding, padding, padding, padding)
}

fun View.setPaddingHorizontal(padding: Int) {
    setPadding(padding, paddingTop, padding, paddingBottom)
}

fun View.setPaddingVertical(padding: Int) {
    setPadding(paddingLeft, padding, paddingRight, padding)
}

// Size Extensions
fun View.setWidth(width: Int) {
    layoutParams = layoutParams?.apply { this.width = width } ?: ViewGroup.LayoutParams(width, ViewGroup.LayoutParams.WRAP_CONTENT)
}

fun View.setHeight(height: Int) {
    layoutParams = layoutParams?.apply { this.height = height } ?: ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, height)
}

fun View.setSize(width: Int, height: Int) {
    layoutParams = layoutParams?.apply {
        this.width = width
        this.height = height
    } ?: ViewGroup.LayoutParams(width, height)
}

// Fragment Toast Extensions
fun Fragment.showToast(message: String, duration: Int = Toast.LENGTH_SHORT) {
    Toast.makeText(requireContext(), message, duration).show()
    Timber.d("Toast shown: $message")
}

fun Fragment.showToast(messageResId: Int, duration: Int = Toast.LENGTH_SHORT) {
    Toast.makeText(requireContext(), messageResId, duration).show()
    Timber.d("Toast shown with resource ID: $messageResId")
}

// View Group Extensions
fun android.view.ViewGroup.children(): Sequence<View> {
    return (0 until childCount).asSequence().map { getChildAt(it) }
}

fun android.view.ViewGroup.forEach(action: (View) -> Unit) {
    children().forEach(action)
}

// Content Description Extension
fun View.setContentDescriptionIfEmpty(description: String) {
    if (contentDescription.isNullOrEmpty()) {
        contentDescription = description
    }
}

// Tag Extensions
fun View.getTag(key: Int, defaultValue: Any? = null): Any? {
    return getTag(key) ?: defaultValue
}

// Safe Extensions
fun View?.isVisible(): Boolean = this?.visibility == View.VISIBLE

fun View?.isGone(): Boolean = this?.visibility == View.GONE

fun View?.isInvisible(): Boolean = this?.visibility == View.INVISIBLE

/**
 * Checks if view is clickable
 */
fun View.isClickable(): Boolean = isClickable && isEnabled

/**
 * Scrolls view to specific position
 */
fun android.widget.ScrollView.scrollToView(view: View) {
    smoothScrollTo(0, view.top)
    Timber.d("Scrolled to view position")
}

/**
 * Sets view to loading state
 */
fun View.setLoading(isLoading: Boolean) {
    if (isLoading) {
        disable()
        alpha = 0.6f
    } else {
        enable()
        alpha = 1f
    }
}

/**
 * Adds bottom margin to view
 */
fun View.addBottomMargin(margin: Int) {
    val params = layoutParams as? android.view.ViewGroup.MarginLayoutParams
    params?.bottomMargin = margin
    layoutParams = params
}

/**
 * Converts pixels to dp
 */
fun View.pxToDp(px: Int): Int {
    return (px / resources.displayMetrics.density).toInt()
}

/**
 * Converts dp to pixels
 */
fun View.dpToPx(dp: Int): Int {
    return (dp * resources.displayMetrics.density).toInt()
}
