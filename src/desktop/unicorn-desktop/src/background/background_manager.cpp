#ifdef _WIN32
#undef max
#undef min
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "background_manager.h"
#include <curl/curl.h>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <sstream>

namespace Unicorn::Background {

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        std::string* str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
        size_t totalSize = size * nitems;
        auto* headers = static_cast<std::unordered_map<std::string, std::string>*>(userdata);

        std::string header(buffer, totalSize);
        size_t colonPos = header.find(':');

        if (colonPos != std::string::npos) {
            std::string key = header.substr(0, colonPos);
            std::string value = header.substr(colonPos + 1);

            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            (*headers)[key] = value;
        }

        return totalSize;
    }

    static int CurlProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
        curl_off_t /*ultotal*/, curl_off_t /*ulnow*/) {
        auto* progress = static_cast<ProgressCallback*>(clientp);

        if (progress && dltotal > 0) {
            (*progress)(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
        }

        return 0;
    }

    BackgroundManager& BackgroundManager::Get() {
        static BackgroundManager instance;
        return instance;
    }

    void BackgroundManager::Init() {
        curl_global_init(CURL_GLOBAL_ALL);
        m_Running = true;
        std::cout << "[BackgroundManager] Initialized" << std::endl;
    }

    void BackgroundManager::Shutdown() {
        m_Running = false;
        CancelAll();

        for (auto& request : m_ActiveRequests) {
            if (request->thread.joinable()) {
                request->thread.join();
            }
        }

        m_ActiveRequests.clear();

        while (!m_PendingRequests.empty()) {
            m_PendingRequests.pop();
        }

        curl_global_cleanup();
        std::cout << "[BackgroundManager] Shutdown" << std::endl;
    }

    BackgroundManager::~BackgroundManager() {
        if (m_Running) {
            Shutdown();
        }
    }

    void BackgroundManager::Update() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        m_ActiveRequests.erase(
            std::remove_if(m_ActiveRequests.begin(), m_ActiveRequests.end(),
                [](const std::shared_ptr<RequestData>& req) {
                    return req->handle->state != RequestState::Loading;
                }),
            m_ActiveRequests.end()
        );

        while (m_ActiveRequests.size() < m_MaxConcurrentRequests && !m_PendingRequests.empty()) {
            auto request = m_PendingRequests.front();
            m_PendingRequests.pop();

            ProcessRequest(request);
            m_ActiveRequests.push_back(request);
        }
    }

    size_t BackgroundManager::Request(const RequestOptions& options, RequestCallback callback) {
        return RequestWithProgress(options, callback, nullptr, nullptr);
    }

    size_t BackgroundManager::RequestWithProgress(const RequestOptions& options,
        RequestCallback callback,
        ProgressCallback downloadProgress,
        UploadProgressCallback uploadProgress) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        size_t requestId = m_NextRequestId++;

        auto handle = std::make_shared<RequestHandle>();
        handle->id = requestId;
        handle->state = RequestState::Idle;
        handle->cancelled = false;

        auto request = std::make_shared<RequestData>();
        request->id = requestId;
        request->options = options;
        request->callback = callback;
        request->downloadProgressCallback = downloadProgress;
        request->uploadProgressCallback = uploadProgress;
        request->handle = handle;

        m_Handles[requestId] = handle;
        m_PendingRequests.push(request);

        if (callback) {
            callback(RequestState::Idle, handle->response);
        }

        return requestId;
    }

    void BackgroundManager::ProcessRequest(std::shared_ptr<RequestData> request) {
        request->handle->state = RequestState::Loading;

        if (request->callback) {
            request->callback(RequestState::Loading, request->handle->response);
        }

        request->thread = std::thread([this, request]() {
            ExecuteRequest(request);
            });

        request->thread.detach();
    }

    void BackgroundManager::ExecuteRequest(std::shared_ptr<RequestData> request) {
        auto startTime = std::chrono::high_resolution_clock::now();

        CURL* curl = curl_easy_init();
        if (!curl) {
            request->handle->state = RequestState::Error;
            request->handle->response.error = "Failed to initialize CURL";

            if (request->callback) {
                request->callback(RequestState::Error, request->handle->response);
            }
            return;
        }

        std::string responseBody;
        std::unordered_map<std::string, std::string> responseHeaders;

        curl_easy_setopt(curl, CURLOPT_URL, request->options.url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, request->options.timeoutSeconds);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, request->options.followRedirects ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        if (request->downloadProgressCallback) {
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CurlProgressCallback);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &request->downloadProgressCallback);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        }

        switch (request->options.method) {
        case RequestMethod::POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->options.body.c_str());
            break;
        case RequestMethod::PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->options.body.c_str());
            break;
        case RequestMethod::HTTP_DELETE:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case RequestMethod::PATCH:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->options.body.c_str());
            break;
        default:
            break;
        }

        struct curl_slist* headers = nullptr;
        for (const auto& [key, value] : request->options.headers) {
            std::string header = key + ": " + value;
            headers = curl_slist_append(headers, header.c_str());
        }
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        CURLcode res = CURLE_OK;

        if (!request->handle->cancelled) {
            res = curl_easy_perform(curl);
        }
        else {
            res = CURLE_ABORTED_BY_CALLBACK;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        request->handle->response.elapsedTime = duration.count() / 1000.0;
        request->handle->response.body = responseBody;
        request->handle->response.headers = responseHeaders;

        double downloadSize = 0;
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &downloadSize);
        request->handle->response.downloadSize = static_cast<size_t>(downloadSize);

        double uploadSize = 0;
        curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &uploadSize);
        request->handle->response.uploadSize = static_cast<size_t>(uploadSize);

        if (request->handle->cancelled) {
            request->handle->state = RequestState::Cancelled;
            request->handle->response.error = "Request cancelled";
        }
        else if (res != CURLE_OK) {
            request->handle->state = RequestState::Error;
            request->handle->response.error = curl_easy_strerror(res);
        }
        else {
            long statusCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
            request->handle->response.statusCode = static_cast<int>(statusCode);

            if (statusCode >= 200 && statusCode < 300) {
                request->handle->state = RequestState::Success;
            }
            else {
                request->handle->state = RequestState::Error;
                request->handle->response.error = "HTTP " + std::to_string(statusCode);
            }
        }

        if (headers) {
            curl_slist_free_all(headers);
        }

        curl_easy_cleanup(curl);

        if (request->callback) {
            request->callback(request->handle->state, request->handle->response);
        }

        UpdateStats(request->handle->response, request->handle->state);

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::string historyEntry = request->options.url + " - " +
                std::to_string(request->handle->response.statusCode) + " - " +
                std::to_string(static_cast<int>(request->handle->response.elapsedTime * 1000)) + "ms";
            m_RequestHistory.push_back(historyEntry);

            if (m_RequestHistory.size() > 100) {
                m_RequestHistory.erase(m_RequestHistory.begin());
            }
        }
    }

    void BackgroundManager::Cancel(size_t requestId) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto it = m_Handles.find(requestId);
        if (it != m_Handles.end()) {
            it->second->cancelled = true;
        }
    }

    void BackgroundManager::CancelAll() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        for (auto& [id, handle] : m_Handles) {
            handle->cancelled = true;
        }
    }

    RequestState BackgroundManager::GetState(size_t requestId) const {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto it = m_Handles.find(requestId);
        if (it != m_Handles.end()) {
            return it->second->state;
        }

        return RequestState::Idle;
    }

    Response BackgroundManager::GetResponse(size_t requestId) const {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto it = m_Handles.find(requestId);
        if (it != m_Handles.end()) {
            return it->second->response;
        }

        return Response{};
    }

    size_t BackgroundManager::GetActiveRequestCount() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_ActiveRequests.size();
    }

    size_t BackgroundManager::GetPendingRequestCount() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_PendingRequests.size();
    }

    void BackgroundManager::SetMaxConcurrentRequests(size_t max) {
        m_MaxConcurrentRequests = max;
    }

    void BackgroundManager::SetGlobalTimeout(int seconds) {
        m_GlobalTimeout = seconds;
    }

    void BackgroundManager::SetUserAgent(const std::string& userAgent) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_UserAgent = userAgent;
    }

    void BackgroundManager::SetDefaultHeaders(const std::unordered_map<std::string, std::string>& headers) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_DefaultHeaders = headers;
    }

    void BackgroundManager::ClearCache() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Cache.clear();
    }

    void BackgroundManager::SetCacheMaxSize(size_t maxSizeMB) {
        m_CacheMaxSize = maxSizeMB;
    }

    bool BackgroundManager::IsCached(const std::string& url) const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Cache.find(url) != m_Cache.end();
    }

    std::string BackgroundManager::BuildQueryString(const std::unordered_map<std::string, std::string>& params) {
        std::string result;
        bool first = true;

        for (const auto& [key, value] : params) {
            if (!first) result += "&";
            result += UrlEncode(key) + "=" + UrlEncode(value);
            first = false;
        }

        return result;
    }

    std::string BackgroundManager::UrlEncode(const std::string& value) {
        static const char hexChars[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(value.length() * 3); // Worst case: every char encoded

        for (char c : value) {
            if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
                result += c;
            }
            else {
                unsigned char uc = static_cast<unsigned char>(c);
                result += '%';
                result += hexChars[uc >> 4];
                result += hexChars[uc & 0x0F];
            }
        }

        return result;
    }

    std::string BackgroundManager::UrlDecode(const std::string& value) {
        std::string result;
        for (size_t i = 0; i < value.length(); i++) {
            if (value[i] == '%' && i + 2 < value.length()) {
                int hex = std::stoi(value.substr(i + 1, 2), nullptr, 16);
                result += static_cast<char>(hex);
                i += 2;
            }
            else if (value[i] == '+') {
                result += ' ';
            }
            else {
                result += value[i];
            }
        }
        return result;
    }

    bool BackgroundManager::TestConnection(const std::string& url, int timeoutSeconds) {
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);

        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        curl_easy_cleanup(curl);

        return (res == CURLE_OK && response_code >= 200 && response_code < 400);
    }

    std::vector<std::string> BackgroundManager::GetRequestHistory(size_t maxCount) const {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (maxCount == 0) {
            return {};
        }

        size_t startIdx = m_RequestHistory.size() > maxCount ?
            m_RequestHistory.size() - maxCount : 0;

        return std::vector<std::string>(
            m_RequestHistory.begin() + static_cast<std::ptrdiff_t>(startIdx),
            m_RequestHistory.end()
        );
    }

    RequestStats BackgroundManager::GetStats() const {
        std::lock_guard<std::mutex> lock(m_StatsMutex);
        return m_Stats;
    }

    void BackgroundManager::UpdateStats(const Response& response, RequestState state) {
        std::lock_guard<std::mutex> lock(m_StatsMutex);

        m_Stats.totalRequests++;

        switch (state) {
        case RequestState::Success:
            m_Stats.successfulRequests++;
            break;
        case RequestState::Error:
        case RequestState::Timeout:
            m_Stats.failedRequests++;
            break;
        case RequestState::Cancelled:
            m_Stats.cancelledRequests++;
            break;
        default:
            break;
        }

        if (state == RequestState::Success) {
            double totalTime = m_Stats.averageResponseTime * (m_Stats.successfulRequests - 1);
            m_Stats.averageResponseTime = (totalTime + response.elapsedTime) / m_Stats.successfulRequests;

            m_Stats.totalBytesDownloaded += response.downloadSize;
            m_Stats.totalBytesUploaded += response.uploadSize;

            if (response.fromCache) {
                m_Stats.cacheHits++;
            }
            else {
                m_Stats.cacheMisses++;
            }
        }
    }

    bool BackgroundManager::GetCachedResponse(const std::string& url, Response& response) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto it = m_Cache.find(url);
        if (it != m_Cache.end()) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
                now - it->second.timestamp
            ).count();

            if (elapsed < 60) {
                response = it->second.response;
                response.fromCache = true;
                it->second.accessCount++;
                return true;
            }
            else {
                m_Cache.erase(it);
            }
        }

        return false;
    }

    void BackgroundManager::CacheResponse(const std::string& url, const Response& response) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (m_Cache.size() >= m_CacheMaxSize) {
            CleanupCache();
        }

        CachedResponse cached;
        cached.response = response;
        cached.timestamp = std::chrono::steady_clock::now();
        cached.accessCount = 0;

        m_Cache[url] = cached;
    }

    void BackgroundManager::CleanupCache() {
        if (m_Cache.empty()) return;

        auto oldest = m_Cache.begin();
        for (auto it = m_Cache.begin(); it != m_Cache.end(); ++it) {
            if (it->second.timestamp < oldest->second.timestamp) {
                oldest = it;
            }
        }

        m_Cache.erase(oldest);
    }

    void BackgroundManager::CompleteRequest(size_t requestId) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Handles.erase(requestId);
    }

} // namespace Unicorn::Background