package com.ucworks.clipforge.models

/**
 * Represents an available effect/filter that can be applied to clips.
 */
data class Effect(
    val id: String,
    val name: String,
    val category: String,  // "Color", "Distortion", "Blur", "Light", "Artistic", "Vintage"
    val description: String,
    val intensity: Float = 100f,  // 0-100 for adjustment
    val preview: String? = null,  // path or drawable resource
    val parameters: List<EffectParameter> = emptyList()
)

/**
 * Represents a single adjustable parameter for an effect.
 */
data class EffectParameter(
    val id: String,
    val name: String,
    val type: ParameterType,  // Slider, Toggle, Color, etc.
    val defaultValue: Float = 50f,
    val minValue: Float = 0f,
    val maxValue: Float = 100f,
    var currentValue: Float = defaultValue
)

enum class ParameterType {
    SLIDER,
    TOGGLE,
    COLOR,
    DROPDOWN
}

/**
 * Pre-defined effect presets/presets
 */
object EffectLibrary {
    val categories = listOf(
        "All", "Color", "Distortion", "Blur", "Light", "Artistic", "Vintage"
    )

    val effects = listOf(
        // Color Effects
        Effect(
            id = "vintage",
            name = "Vintage",
            category = "Color",
            description = "Classic vintage film look"
        ),
        Effect(
            id = "bw",
            name = "B&W",
            category = "Color",
            description = "Black and white conversion"
        ),
        Effect(
            id = "sepia",
            name = "Sepia",
            category = "Color",
            description = "Warm sepia tone effect"
        ),
        Effect(
            id = "saturate",
            name = "Saturate",
            category = "Color",
            description = "Increase color saturation"
        ),
        Effect(
            id = "desaturate",
            name = "Desaturate",
            category = "Color",
            description = "Reduce color saturation"
        ),

        // Blur Effects
        Effect(
            id = "blur_gaussian",
            name = "Gaussian Blur",
            category = "Blur",
            description = "Smooth blur effect"
        ),
        Effect(
            id = "blur_motion",
            name = "Motion Blur",
            category = "Blur",
            description = "Directional motion blur"
        ),
        Effect(
            id = "blur_zoom",
            name = "Zoom Blur",
            category = "Blur",
            description = "Radial zoom blur"
        ),

        // Distortion Effects
        Effect(
            id = "wave",
            name = "Wave",
            category = "Distortion",
            description = "Wave distortion effect"
        ),
        Effect(
            id = "barrel",
            name = "Barrel",
            category = "Distortion",
            description = "Barrel lens distortion"
        ),

        // Light Effects
        Effect(
            id = "bloom",
            name = "Bloom",
            category = "Light",
            description = "Glowing bloom effect"
        ),
        Effect(
            id = "glow",
            name = "Glow",
            category = "Light",
            description = "Soft glow overlay"
        ),

        // Artistic Effects
        Effect(
            id = "oil_paint",
            name = "Oil Paint",
            category = "Artistic",
            description = "Oil painting effect"
        ),
        Effect(
            id = "comic",
            name = "Comic",
            category = "Artistic",
            description = "Comic book style"
        ),

        // Vintage Effects
        Effect(
            id = "vhs",
            name = "VHS",
            category = "Vintage",
            description = "80s VHS tape look"
        ),
        Effect(
            id = "film_grain",
            name = "Film Grain",
            category = "Vintage",
            description = "Add film grain texture"
        )
    )

    fun getByCategory(category: String): List<Effect> {
        return if (category == "All") effects else effects.filter { it.category == category }
    }

    fun search(query: String): List<Effect> {
        val lowerQuery = query.lowercase()
        return effects.filter {
            it.name.lowercase().contains(lowerQuery) ||
            it.description.lowercase().contains(lowerQuery)
        }
    }
}
