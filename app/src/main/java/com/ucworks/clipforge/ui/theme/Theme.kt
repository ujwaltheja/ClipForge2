package com.ucworks.clipforge.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable

private val DarkColorScheme = darkColorScheme(
    primary = BrandPurple,
    onPrimary = TextPrimary,
    primaryContainer = BrandPurpleDark,
    onPrimaryContainer = TextPrimary,

    secondary = BrandPink,
    onSecondary = TextPrimary,
    secondaryContainer = BrandPurpleDark,
    onSecondaryContainer = TextPrimary,

    tertiary = AccentGold,
    onTertiary = TextPrimary,
    tertiaryContainer = AccentAmber,
    onTertiaryContainer = TextPrimary,

    background = BackgroundDarker,
    onBackground = TextPrimary,

    surface = SurfaceDark,
    onSurface = TextPrimary,
    surfaceVariant = SurfaceDarker,
    onSurfaceVariant = TextSecondary,

    outline = TextTertiary,
    outlineVariant = SurfaceDarker,

    error = ErrorRed,
    onError = TextPrimary,
    errorContainer = ErrorRed,
    onErrorContainer = TextPrimary,

    scrim = TransparentColor
)

@Composable
fun ClipForgeTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) {
        DarkColorScheme
    } else {
        DarkColorScheme // Force dark theme for this app
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
