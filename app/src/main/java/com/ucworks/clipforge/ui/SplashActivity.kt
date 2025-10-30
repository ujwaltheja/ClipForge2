package com.ucworks.clipforge.ui

import android.animation.ObjectAnimator
import android.content.Intent
import android.os.Bundle
import android.view.View
import android.view.animation.DecelerateInterpolator
import androidx.appcompat.app.AppCompatActivity
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.lifecycle.lifecycleScope
import com.ucworks.clipforge.R
import com.ucworks.clipforge.databinding.ActivitySplashBinding
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import com.ucworks.clipforge.MainActivity

class SplashActivity : AppCompatActivity() {
    private lateinit var binding: ActivitySplashBinding
    
    override fun onCreate(savedInstanceState: Bundle?) {
        val splashScreen = installSplashScreen()
        super.onCreate(savedInstanceState)
        
        binding = ActivitySplashBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // Animate logo
        ObjectAnimator.ofFloat(binding.logoImage, View.ALPHA, 0f, 1f).apply {
            duration = 1000
            interpolator = DecelerateInterpolator()
            start()
        }
        
        lifecycleScope.launch {
            delay(2000)
            checkFirstLaunchAndNavigate()
        }
    }
    
    private fun checkFirstLaunchAndNavigate() {
        val prefs = getSharedPreferences("app_prefs", MODE_PRIVATE)
        val isFirstLaunch = prefs.getBoolean("first_launch", true)
        
        if (isFirstLaunch) {
            startActivity(Intent(this, OnboardingActivity::class.java))
            prefs.edit().putBoolean("first_launch", false).apply()
        } else {
            startActivity(Intent(this, MainActivity::class.java))
        }
        finish()
    }
}