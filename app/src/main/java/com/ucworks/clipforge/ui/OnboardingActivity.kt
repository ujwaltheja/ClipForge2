package com.ucworks.clipforge.ui

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.viewpager2.widget.ViewPager2
import com.ucworks.clipforge.databinding.ActivityOnboardingBinding
import com.ucworks.clipforge.ui.adapters.OnboardingAdapter
import com.ucworks.clipforge.data.models.OnboardingPage
import com.google.android.material.tabs.TabLayoutMediator
import com.ucworks.clipforge.R
import com.ucworks.clipforge.MainActivity

class OnboardingActivity : AppCompatActivity() {
    private lateinit var binding: ActivityOnboardingBinding
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityOnboardingBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setupOnboarding()
    }
    
    private fun setupOnboarding() {
        val pages = listOf(
            OnboardingPage(
                "Professional Editing",
                "Create stunning videos with 10+ GPU-accelerated effects",
                R.drawable.onboarding_1
            ),
            OnboardingPage(
                "Real-time Preview",
                "See your edits instantly at 60fps with hardware acceleration",
                R.drawable.onboarding_2
            ),
            OnboardingPage(
                "Audio Magic",
                "Beat detection, spectrum analysis, and professional mixing",
                R.drawable.onboarding_3
            ),
            OnboardingPage(
                "Export Quality",
                "4K support with H.264, H.265, and VP9 codecs",
                R.drawable.onboarding_4
            )
        )
        
        binding.viewPager.adapter = OnboardingAdapter(pages)
        
        TabLayoutMediator(binding.tabLayout, binding.viewPager) { _, _ -> }.attach()
        
        binding.viewPager.registerOnPageChangeCallback(object : ViewPager2.OnPageChangeCallback() {
            override fun onPageSelected(position: Int) {
                binding.btnNext.text = if (position == pages.size - 1) "Get Started" else "Next"
            }
        })
        
        binding.btnNext.setOnClickListener {
            if (binding.viewPager.currentItem < pages.size - 1) {
                binding.viewPager.currentItem += 1
            } else {
                startActivity(Intent(this, MainActivity::class.java))
                finish()
            }
        }
        
        binding.btnSkip.setOnClickListener {
            startActivity(Intent(this, MainActivity::class.java))
            finish()
        }
    }
}