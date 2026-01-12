#pragma once
#include <soloud.h>
#include <soloud_wav.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace Unicorn::Audio {
    class SoundManager {
    public:
        SoundManager();
        ~SoundManager();

        bool Init();
        void Shutdown();

        bool LoadSound(const std::string& name, const std::string& filepath);
        void PlaySound(const std::string& name, float volume = 1.0f);

        void SetMasterVolume(float volume);
        float GetMasterVolume() const { return m_MasterVolume; }

    private:
        SoLoud::Soloud m_Engine;
        std::unordered_map<std::string, std::unique_ptr<SoLoud::Wav>> m_Sounds;
        float m_MasterVolume = 1.0f;
    };
}