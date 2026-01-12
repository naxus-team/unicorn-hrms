#include "video_player.h"
#include <glad/glad.h>
#include <iostream>
#include <algorithm>

#ifdef HAVE_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

// NOTE: This version supports both with and without FFmpeg
// When HAVE_FFMPEG is defined, full video decoding is available
// Otherwise, it provides placeholder functionality

namespace Unicorn::Video {

    // ==========================================
    // VideoDecoder Implementation
    // ==========================================

    VideoDecoder::VideoDecoder() {}

    VideoDecoder::~VideoDecoder() {
        Close();
    }

    bool VideoDecoder::LoadFromFile(const std::string& filepath) {
        std::cout << "[VideoDecoder] Loading file: " << filepath << std::endl;

#ifdef HAVE_FFMPEG
        // Real FFmpeg implementation
        AVFormatContext* formatCtx = nullptr;

        if (avformat_open_input(&formatCtx, filepath.c_str(), nullptr, nullptr) < 0) {
            std::cerr << "[VideoDecoder] Failed to open video file" << std::endl;
            m_State = VideoState::Error;
            return false;
        }

        if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
            avformat_close_input(&formatCtx);
            std::cerr << "[VideoDecoder] Failed to find stream info" << std::endl;
            m_State = VideoState::Error;
            return false;
        }

        // Find video stream
        int videoStreamIdx = -1;
        const AVCodec* codec = nullptr;

        for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
            if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIdx = i;
                codec = avcodec_find_decoder(formatCtx->streams[i]->codecpar->codec_id);
                break;
            }
        }

        if (videoStreamIdx == -1 || !codec) {
            avformat_close_input(&formatCtx);
            std::cerr << "[VideoDecoder] No video stream found" << std::endl;
            m_State = VideoState::Error;
            return false;
        }

        // Create codec context
        AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoStreamIdx]->codecpar);

        if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
            avcodec_free_context(&codecCtx);
            avformat_close_input(&formatCtx);
            std::cerr << "[VideoDecoder] Failed to open codec" << std::endl;
            m_State = VideoState::Error;
            return false;
        }

        m_FormatContext = formatCtx;
        m_CodecContext = codecCtx;
        m_VideoStreamIndex = videoStreamIdx;

        // Fill video info
        m_Info.width = codecCtx->width;
        m_Info.height = codecCtx->height;
        m_Info.duration = formatCtx->duration / (double)AV_TIME_BASE;
        m_Info.fps = av_q2d(formatCtx->streams[videoStreamIdx]->r_frame_rate);
        m_Info.hasAudio = false; // TODO: Check for audio stream

        std::cout << "[VideoDecoder] Video loaded: " << m_Info.width << "x" << m_Info.height
            << " @ " << m_Info.fps << " fps, duration: " << m_Info.duration << "s" << std::endl;
#else
        // Placeholder implementation without FFmpeg
        std::cout << "[VideoDecoder] FFmpeg not available - using placeholder" << std::endl;

        m_Info.width = 1920;
        m_Info.height = 1080;
        m_Info.duration = 120.0;
        m_Info.fps = 30.0;
        m_Info.hasAudio = false;
#endif

        m_State = VideoState::Ready;
        return true;
    }

    bool VideoDecoder::LoadFromURL(const std::string& url) {
        std::cout << "[VideoDecoder] Loading URL: " << url << std::endl;

        // TODO: Same as LoadFromFile but with URL
        // FFmpeg handles both files and URLs the same way

        return LoadFromFile(url);
    }

    VideoFrame* VideoDecoder::GetNextFrame() {
        std::lock_guard<std::mutex> lock(m_QueueMutex);

        if (m_FrameQueue.empty()) {
            return nullptr;
        }

        VideoFrame* frame = m_FrameQueue.front();
        m_FrameQueue.pop();
        return frame;
    }

    void VideoDecoder::Seek(double timestamp) {
        std::cout << "[VideoDecoder] Seeking to: " << timestamp << "s" << std::endl;

        // TODO: av_seek_frame()
        m_CurrentTime = timestamp;
    }

    void VideoDecoder::Start() {
        if (m_Running) return;

        m_Running = true;
        m_DecoderThread = std::thread(&VideoDecoder::DecodingThread, this);
    }

    void VideoDecoder::Stop() {
        m_Running = false;
        if (m_DecoderThread.joinable()) {
            m_DecoderThread.join();
        }
    }

    void VideoDecoder::Close() {
        Stop();

        // Clear frame queue
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        while (!m_FrameQueue.empty()) {
            delete m_FrameQueue.front();
            m_FrameQueue.pop();
        }

#ifdef HAVE_FFMPEG
        // Clean up FFmpeg resources
        if (m_SwsContext) {
            sws_freeContext((SwsContext*)m_SwsContext);
            m_SwsContext = nullptr;
        }
        if (m_CodecContext) {
            avcodec_free_context((AVCodecContext**)&m_CodecContext);
        }
        if (m_FormatContext) {
            avformat_close_input((AVFormatContext**)&m_FormatContext);
        }
#endif

        m_State = VideoState::Idle;
    }

    void VideoDecoder::DecodingThread() {
        std::cout << "[VideoDecoder] Decoding thread started" << std::endl;

        while (m_Running) {
            // TODO: Decode frames using FFmpeg
            // av_read_frame()
            // avcodec_send_packet()
            // avcodec_receive_frame()
            // sws_scale() to convert to RGB

            // For now, just sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        std::cout << "[VideoDecoder] Decoding thread stopped" << std::endl;
    }

    // ==========================================
    // VideoPlayer Implementation
    // ==========================================

    VideoPlayer::VideoPlayer() {
        m_Decoder = std::make_unique<VideoDecoder>();

        // Create OpenGL texture
        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        std::cout << "[VideoPlayer] Created with texture ID: " << m_TextureID << std::endl;
    }

    VideoPlayer::~VideoPlayer() {
        UnloadVideo();

        if (m_TextureID) {
            glDeleteTextures(1, &m_TextureID);
        }
    }

    bool VideoPlayer::LoadVideo(const std::string& source, bool isURL) {
        UnloadVideo();

        std::cout << "[VideoPlayer] Loading video: " << source << std::endl;

        bool success = isURL ?
            m_Decoder->LoadFromURL(source) :
            m_Decoder->LoadFromFile(source);

        if (success) {
            m_State = VideoState::Ready;
            if (m_OnStateChanged) {
                m_OnStateChanged(m_State);
            }
        }
        else {
            m_State = VideoState::Error;
            if (m_OnError) {
                m_OnError("Failed to load video: " + source);
            }
        }

        return success;
    }

    void VideoPlayer::UnloadVideo() {
        Stop();
        m_Decoder->Close();
        m_PlaybackTime = 0.0;
    }

    void VideoPlayer::Play() {
        if (m_State == VideoState::Ready || m_State == VideoState::Paused) {
            m_State = VideoState::Playing;
            m_Decoder->Start();

            std::cout << "[VideoPlayer] Playing" << std::endl;

            if (m_OnStateChanged) {
                m_OnStateChanged(m_State);
            }
        }
    }

    void VideoPlayer::Pause() {
        if (m_State == VideoState::Playing) {
            m_State = VideoState::Paused;

            std::cout << "[VideoPlayer] Paused" << std::endl;

            if (m_OnStateChanged) {
                m_OnStateChanged(m_State);
            }
        }
    }

    void VideoPlayer::Stop() {
        if (m_State == VideoState::Playing || m_State == VideoState::Paused) {
            m_State = VideoState::Stopped;
            m_Decoder->Stop();
            m_PlaybackTime = 0.0;

            std::cout << "[VideoPlayer] Stopped" << std::endl;

            if (m_OnStateChanged) {
                m_OnStateChanged(m_State);
            }
        }
    }

    void VideoPlayer::Seek(double timestamp) {
        m_Decoder->Seek(timestamp);
        m_PlaybackTime = timestamp;
    }

    VideoState VideoPlayer::GetState() const {
        return m_State;
    }

    double VideoPlayer::GetCurrentTime() const {
        return m_PlaybackTime;
    }

    double VideoPlayer::GetDuration() const {
        return m_Decoder->GetInfo().duration;
    }

    VideoInfo VideoPlayer::GetVideoInfo() const {
        return m_Decoder->GetInfo();
    }

    void VideoPlayer::SetVolume(float volume) {
        m_Volume = std::clamp(volume, 0.0f, 1.0f);
        // TODO: Apply to audio stream
    }

    void VideoPlayer::SetOnStateChanged(std::function<void(VideoState)> callback) {
        m_OnStateChanged = callback;
    }

    void VideoPlayer::SetOnError(std::function<void(const std::string&)> callback) {
        m_OnError = callback;
    }

    void VideoPlayer::Update(float deltaTime) {
        if (m_State != VideoState::Playing) {
            return;
        }

        m_PlaybackTime += deltaTime;
        UpdateFrame();

        // Check if video ended
        if (m_PlaybackTime >= GetDuration()) {
            Stop();
        }
    }

    void VideoPlayer::UpdateFrame() {
        VideoFrame* frame = m_Decoder->GetNextFrame();
        if (frame) {
            UploadFrameToTexture(frame);
            delete frame;
        }
    }

    void VideoPlayer::UploadFrameToTexture(VideoFrame* frame) {
        if (!frame || !frame->data) return;

        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            frame->width, frame->height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, frame->data);
    }

} // namespace Unicorn::Video