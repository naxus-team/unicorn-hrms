#include "sound_manager.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <unordered_set>

namespace Unicorn::Audio {

    SoundManager::SoundManager() {}

    SoundManager::~SoundManager() {
        Shutdown();
    }

    bool SoundManager::Init() {
        auto result = m_Engine.init();
        if (result != SoLoud::SO_NO_ERROR) {
            std::cerr << "[Audio] Failed to initialize SoLoud: " << result << std::endl;
            return false;
        }
        std::cout << "[Audio] SoLoud initialized successfully" << std::endl;
        return true;
    }

    void SoundManager::Shutdown() {
        m_Sounds.clear();
        m_Engine.deinit();
        std::cout << "[Audio] Shutdown complete" << std::endl;
    }

    bool SoundManager::LoadSound(const std::string& name, const std::string& filepath) {
        // Check if file exists first
        std::ifstream file(filepath);
        if (!file.good()) {
            std::cerr << "[Audio] File not found: " << filepath << std::endl;
            std::cerr << "[Audio] Please add WAV files to assets/sounds/ directory" << std::endl;
            return false;
        }
        file.close();

        auto sound = std::make_unique<SoLoud::Wav>();
        auto result = sound->load(filepath.c_str());

        if (result != SoLoud::SO_NO_ERROR) {
            std::cerr << "[Audio] Failed to load: " << filepath << " (Error: " << result << ")" << std::endl;
            return false;
        }

        m_Sounds[name] = std::move(sound);
        std::cout << "[Audio] Loaded: " << name << " from " << filepath << std::endl;
        return true;
    }

    void SoundManager::PlaySound(const std::string& name, float volume) {
        auto it = m_Sounds.find(name);
        if (it == m_Sounds.end()) {
            static std::unordered_set<std::string> missingWarned;
            if (missingWarned.find(name) == missingWarned.end()) {
                std::cerr << "[Audio] Sound not found: " << name << std::endl;
                missingWarned.insert(name);
            }
            return;
        }

        m_Engine.play(*it->second, volume * m_MasterVolume);
    }

    void SoundManager::SetMasterVolume(float volume) {
        m_MasterVolume = std::clamp(volume, 0.0f, 1.0f);
        m_Engine.setGlobalVolume(m_MasterVolume);
    }
}