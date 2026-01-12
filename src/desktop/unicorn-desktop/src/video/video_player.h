#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>

// Forward declare FFmpeg types to avoid including headers
#ifdef HAVE_FFMPEG
extern "C" {
    struct AVFormatContext;
    struct AVCodecContext;
    struct SwsContext;
}
#endif

namespace Unicorn::Video {

    enum class VideoState {
        Idle,
        Loading,
        Ready,
        Playing,
        Paused,
        Stopped,
        Error
    };

    struct VideoFrame {
        unsigned char* data = nullptr;
        int width = 0;
        int height = 0;
        double timestamp = 0.0;

        ~VideoFrame() {
            if (data) delete[] data;
        }
    };

    struct VideoInfo {
        int width = 0;
        int height = 0;
        double duration = 0.0;
        double fps = 0.0;
        bool hasAudio = false;
        std::string codec;
    };

    class VideoDecoder {
    public:
        VideoDecoder();
        ~VideoDecoder();

        bool LoadFromFile(const std::string& filepath);
        bool LoadFromURL(const std::string& url);

        VideoFrame* GetNextFrame();
        void Seek(double timestamp);

        VideoInfo GetInfo() const { return m_Info; }
        VideoState GetState() const { return m_State; }
        double GetCurrentTime() const { return m_CurrentTime; }

        void Start();
        void Stop();
        void Close();

    private:
        void DecodingThread();

        VideoInfo m_Info;
        VideoState m_State = VideoState::Idle;
        double m_CurrentTime = 0.0;

        std::queue<VideoFrame*> m_FrameQueue;
        std::mutex m_QueueMutex;
        std::atomic<bool> m_Running{ false };
        std::thread m_DecoderThread;

        void* m_FormatContext = nullptr;  // AVFormatContext*
        void* m_CodecContext = nullptr;   // AVCodecContext*
        void* m_SwsContext = nullptr;     // SwsContext*
        int m_VideoStreamIndex = -1;

#ifdef HAVE_FFMPEG
        AVFormatContext* GetFormatContext() { return (AVFormatContext*)m_FormatContext; }
        AVCodecContext* GetCodecContext() { return (AVCodecContext*)m_CodecContext; }
        SwsContext* GetSwsContext() { return (SwsContext*)m_SwsContext; }
#endif
    };

    class VideoPlayer {
    public:
        VideoPlayer();
        ~VideoPlayer();

        // Loading
        bool LoadVideo(const std::string& source, bool isURL = false);
        void UnloadVideo();

        // Playback control
        void Play();
        void Pause();
        void Stop();
        void Seek(double timestamp);

        // State
        VideoState GetState() const;
        double GetCurrentTime() const;
        double GetDuration() const;
        VideoInfo GetVideoInfo() const;

        // Rendering
        uint32_t GetTextureID() const { return m_TextureID; }
        void Update(float deltaTime);

        // Volume
        void SetVolume(float volume);
        float GetVolume() const { return m_Volume; }

        // Callbacks
        void SetOnStateChanged(std::function<void(VideoState)> callback);
        void SetOnError(std::function<void(const std::string&)> callback);

    private:
        void UpdateFrame();
        void UploadFrameToTexture(VideoFrame* frame);

        std::unique_ptr<VideoDecoder> m_Decoder;
        uint32_t m_TextureID = 0;

        VideoState m_State = VideoState::Idle;
        float m_Volume = 1.0f;
        double m_PlaybackTime = 0.0;

        std::function<void(VideoState)> m_OnStateChanged;
        std::function<void(const std::string&)> m_OnError;
    };

} // namespace Unicorn::Video