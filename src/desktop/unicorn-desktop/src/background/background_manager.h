#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <chrono>

namespace Unicorn::Background {

    enum class RequestState {
        Idle,
        Queued,
        Loading,
        Success,
        Error,
        Cancelled,
        Timeout
    };

    enum class RequestMethod {
        GET,
        POST,
        PUT,
        HTTP_DELETE,
        PATCH,
        HEAD,
        OPTIONS
    };

    enum class CachePolicy {
        NoCache,
        CacheFirst,
        NetworkFirst,
        CacheOnly
    };

    struct RequestOptions {
        RequestMethod method = RequestMethod::GET;
        std::string url;
        std::string body;
        std::unordered_map<std::string, std::string> headers;
        int timeoutSeconds = 30;
        bool followRedirects = true;
        bool verifySSL = true;
        int maxRedirects = 5;
        CachePolicy cachePolicy = CachePolicy::NoCache;
        int retryCount = 0;
        int retryDelayMs = 1000;
        bool useCompression = true;
    };

    struct Response {
        int statusCode = 0;
        std::string body;
        std::unordered_map<std::string, std::string> headers;
        std::string error;
        double elapsedTime = 0.0;
        size_t downloadSize = 0;
        size_t uploadSize = 0;
        bool fromCache = false;
    };

    struct RequestStats {
        size_t totalRequests = 0;
        size_t successfulRequests = 0;
        size_t failedRequests = 0;
        size_t cancelledRequests = 0;
        double averageResponseTime = 0.0;
        size_t totalBytesDownloaded = 0;
        size_t totalBytesUploaded = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
    };

    using RequestCallback = std::function<void(RequestState state, const Response& response)>;
    using ProgressCallback = std::function<void(size_t current, size_t total)>;
    using UploadProgressCallback = std::function<void(size_t uploaded, size_t total)>;

    struct RequestHandle {
        size_t id;
        RequestState state;
        Response response;
        std::atomic<bool> cancelled{ false };
        std::chrono::steady_clock::time_point startTime;
        int currentRetry = 0;
    };

    class BackgroundManager {
    public:
        static BackgroundManager& Get();

        void Init();
        void Shutdown();
        void Update();

        size_t Request(const RequestOptions& options, RequestCallback callback);

        size_t RequestWithProgress(const RequestOptions& options,
            RequestCallback callback,
            ProgressCallback downloadProgress,
            UploadProgressCallback uploadProgress = nullptr);

        void Cancel(size_t requestId);
        void CancelAll();

        RequestState GetState(size_t requestId) const;
        Response GetResponse(size_t requestId) const;

        size_t GetActiveRequestCount() const;
        size_t GetPendingRequestCount() const;
        RequestStats GetStats() const;

        void SetMaxConcurrentRequests(size_t max);
        void SetGlobalTimeout(int seconds);
        void SetUserAgent(const std::string& userAgent);
        void SetDefaultHeaders(const std::unordered_map<std::string, std::string>& headers);

        void ClearCache();
        void SetCacheMaxSize(size_t maxSizeMB);
        bool IsCached(const std::string& url) const;

        std::string BuildQueryString(const std::unordered_map<std::string, std::string>& params);
        std::string UrlEncode(const std::string& value);
        std::string UrlDecode(const std::string& value);

        bool TestConnection(const std::string& url, int timeoutSeconds = 5);
        std::vector<std::string> GetRequestHistory(size_t maxCount = 50) const;

    private:
        BackgroundManager() = default;
        ~BackgroundManager();

        struct RequestData {
            size_t id;
            RequestOptions options;
            RequestCallback callback;
            ProgressCallback downloadProgressCallback;
            UploadProgressCallback uploadProgressCallback;
            std::shared_ptr<RequestHandle> handle;
            std::thread thread;
        };

        struct CachedResponse {
            Response response;
            std::chrono::steady_clock::time_point timestamp;
            size_t accessCount = 0;
        };

        void ProcessRequest(std::shared_ptr<RequestData> request);
        void ExecuteRequest(std::shared_ptr<RequestData> request);
        void CompleteRequest(size_t requestId);
        void UpdateStats(const Response& response, RequestState state);

        bool GetCachedResponse(const std::string& url, Response& response);
        void CacheResponse(const std::string& url, const Response& response);
        void CleanupCache();

        std::vector<std::shared_ptr<RequestData>> m_ActiveRequests;
        std::queue<std::shared_ptr<RequestData>> m_PendingRequests;
        std::unordered_map<size_t, std::shared_ptr<RequestHandle>> m_Handles;
        std::unordered_map<std::string, CachedResponse> m_Cache;
        std::vector<std::string> m_RequestHistory;

        mutable std::mutex m_Mutex;
        std::atomic<size_t> m_NextRequestId{ 1 };
        std::atomic<size_t> m_MaxConcurrentRequests{ 4 };
        std::atomic<int> m_GlobalTimeout{ 30 };
        std::atomic<bool> m_Running{ false };
        std::atomic<size_t> m_CacheMaxSize{ 100 };

        std::string m_UserAgent;
        std::unordered_map<std::string, std::string> m_DefaultHeaders;

        RequestStats m_Stats;
        mutable std::mutex m_StatsMutex;
    };

} // namespace Unicorn::Background