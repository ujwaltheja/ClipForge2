package com.ucworks.clipforge.data.models

import androidx.annotation.DrawableRes

data class Effect(
    val id: String,
    val name: String,
    @DrawableRes val iconRes: Int
)