#include "effect.h"
#include <ctime>
#include <algorithm>

namespace clipforge {
namespace models {

// ============================================================================
// Effect Implementation
// ============================================================================

Effect::Effect(EffectType type, const std::string& name)
    : m_type(type), m_id(generateId()), m_name(name.empty() ? "Unnamed Effect" : name),
      m_createdAt(std::chrono::system_clock::now().time_since_epoch().count() / 1000000) {
    // Initialize parameters based on effect type
    if (type == EffectType::COLOR_BRIGHTNESS) {
        addParameter(EffectParameter("brightness", 0.0f, -1.0f, 1.0f, 0.0f));
    } else if (type == EffectType::COLOR_CONTRAST) {
        addParameter(EffectParameter("contrast", 1.0f, 0.5f, 2.0f, 1.0f));
    } else if (type == EffectType::COLOR_SATURATION) {
        addParameter(EffectParameter("saturation", 1.0f, 0.0f, 2.0f, 1.0f));
    }
}

void Effect::addParameter(const EffectParameter& parameter) {
    // Check if parameter already exists
    auto it = std::find_if(m_parameters.begin(), m_parameters.end(),
                          [&parameter](const EffectParameter& p) {
                              return p.name == parameter.name;
                          });

    if (it != m_parameters.end()) {
        *it = parameter;  // Update existing
    } else {
        m_parameters.push_back(parameter);  // Add new
    }
}

bool Effect::setParameterValue(const std::string& paramName, float value) {
    for (auto& param : m_parameters) {
        if (param.name == paramName) {
            // Clamp value to valid range
            param.value = std::max(param.minValue, std::min(param.maxValue, value));
            return true;
        }
    }
    return false;
}

float Effect::getParameterValue(const std::string& paramName) const {
    for (const auto& param : m_parameters) {
        if (param.name == paramName) {
            return param.value;
        }
    }
    return 0.0f;
}

void Effect::setIntensity(float intensity) {
    m_intensity = std::max(0.0f, std::min(1.0f, intensity));
}

std::string Effect::generateId() {
    // Simple UUID-like string generation
    // In production, use proper UUID library
    static unsigned int counter = 0;
    std::string id = "eff_";
    id += std::to_string(std::time(nullptr));
    id += "_";
    id += std::to_string(++counter);
    return id;
}

// ============================================================================
// EffectChain Implementation
// ============================================================================

void EffectChain::addEffect(std::shared_ptr<Effect> effect) {
    if (effect) {
        m_effects.push_back(effect);
    }
}

bool EffectChain::removeEffect(const std::string& effectId) {
    auto it = std::find_if(m_effects.begin(), m_effects.end(),
                          [&effectId](const std::shared_ptr<Effect>& e) {
                              return e && e->getId() == effectId;
                          });

    if (it != m_effects.end()) {
        m_effects.erase(it);
        return true;
    }
    return false;
}

} // namespace models
} // namespace clipforge
