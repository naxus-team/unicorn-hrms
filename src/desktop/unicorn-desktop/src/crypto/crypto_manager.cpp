#ifdef _WIN32
#undef max
#undef min
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "crypto_manager.h"

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace Unicorn::Crypto {

    CryptoManager& CryptoManager::Get() {
        static CryptoManager instance;
        return instance;
    }

    void CryptoManager::Init() {
        if (m_Initialized) return;

#ifdef HAVE_ZLIB
        std::cout << "[CryptoManager] Initialized with ZLIB " << ZLIB_VERSION << std::endl;
#else
        std::cout << "[CryptoManager] Initialized (ZLIB not available)" << std::endl;
#endif

        m_Initialized = true;
    }

    void CryptoManager::Shutdown() {
        m_Initialized = false;
        std::cout << "[CryptoManager] Shutdown" << std::endl;
    }

    CryptoManager::~CryptoManager() {
        if (m_Initialized) {
            Shutdown();
        }
    }

    // ========================================
    // Compression Functions
    // ========================================

    CompressionResult CryptoManager::Compress(
        const std::vector<uint8_t>& data,
        CompressionLevel level
    ) {
        CompressionResult result;
        result.originalSize = data.size();

#ifdef HAVE_ZLIB
        if (data.empty()) {
            result.error = "Input data is empty";
            return result;
        }

        // Calculate maximum compressed size
        uLongf compressedSize = compressBound(data.size());
        result.data.resize(compressedSize);

        // Compress
        int zlibResult = compress2(
            result.data.data(),
            &compressedSize,
            data.data(),
            data.size(),
            static_cast<int>(level)
        );

        if (zlibResult == Z_OK) {
            result.data.resize(compressedSize);
            result.compressedSize = compressedSize;
            result.compressionRatio = (float)result.originalSize / (float)result.compressedSize;
            result.success = true;
        }
        else {
            result.error = "Compression failed with code " + std::to_string(zlibResult);
        }
#else
        result.error = "ZLIB not available";
#endif

        return result;
    }

    CompressionResult CryptoManager::CompressString(
        const std::string& text,
        CompressionLevel level
    ) {
        std::vector<uint8_t> data(text.begin(), text.end());
        return Compress(data, level);
    }

    CompressionResult CryptoManager::Decompress(const std::vector<uint8_t>& compressedData) {
        CompressionResult result;

#ifdef HAVE_ZLIB
        if (compressedData.empty()) {
            result.error = "Compressed data is empty";
            return result;
        }

        // Start with reasonable buffer size
        uLongf decompressedSize = compressedData.size() * 4;
        result.data.resize(decompressedSize);

        int zlibResult = Z_BUF_ERROR;

        // Try decompression with increasing buffer sizes
        for (int attempt = 0; attempt < 5 && zlibResult == Z_BUF_ERROR; ++attempt) {
            decompressedSize = compressedData.size() * (4 << attempt);
            result.data.resize(decompressedSize);

            zlibResult = uncompress(
                result.data.data(),
                &decompressedSize,
                compressedData.data(),
                compressedData.size()
            );
        }

        if (zlibResult == Z_OK) {
            result.data.resize(decompressedSize);
            result.originalSize = decompressedSize;
            result.compressedSize = compressedData.size();
            result.compressionRatio = (float)result.originalSize / (float)result.compressedSize;
            result.success = true;
        }
        else {
            result.error = "Decompression failed with code " + std::to_string(zlibResult);
        }
#else
        result.error = "ZLIB not available";
#endif

        return result;
    }

    std::string CryptoManager::DecompressToString(const std::vector<uint8_t>& compressedData) {
        auto result = Decompress(compressedData);
        if (result.success) {
            return std::string(result.data.begin(), result.data.end());
        }
        return "";
    }

    // ========================================
    // Encryption Functions (Basic)
    // ========================================

    EncryptionResult CryptoManager::EncryptXOR(
        const std::vector<uint8_t>& data,
        const std::string& key
    ) {
        EncryptionResult result;

        if (data.empty()) {
            result.error = "Input data is empty";
            return result;
        }

        if (key.empty()) {
            result.error = "Encryption key is empty";
            return result;
        }

        result.data = data;

        // XOR each byte with key
        for (size_t i = 0; i < result.data.size(); ++i) {
            result.data[i] ^= key[i % key.length()];
        }

        result.success = true;
        return result;
    }

    EncryptionResult CryptoManager::DecryptXOR(
        const std::vector<uint8_t>& encryptedData,
        const std::string& key
    ) {
        // XOR is symmetric - encryption and decryption are the same
        return EncryptXOR(encryptedData, key);
    }

    // ========================================
    // Hashing Functions
    // ========================================

    uint32_t CryptoManager::HashCRC32(const std::vector<uint8_t>& data) {
#ifdef HAVE_ZLIB
        return crc32(0L, data.data(), data.size());
#else
        // Simple fallback hash if zlib not available
        uint32_t hash = 0;
        for (auto byte : data) {
            hash = (hash << 5) + hash + byte;
        }
        return hash;
#endif
    }

    uint32_t CryptoManager::HashCRC32(const std::string& text) {
        std::vector<uint8_t> data(text.begin(), text.end());
        return HashCRC32(data);
    }

    // ========================================
    // Utility Functions
    // ========================================

    std::string CryptoManager::ToHex(const std::vector<uint8_t>& data) {
        static const char hexChars[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(data.size() * 2);

        for (uint8_t byte : data) {
            result += hexChars[byte >> 4];
            result += hexChars[byte & 0x0F];
        }

        return result;
    }

    std::vector<uint8_t> CryptoManager::FromHex(const std::string& hex) {
        std::vector<uint8_t> result;

        if (hex.length() % 2 != 0) {
            return result; // Invalid hex string
        }

        result.reserve(hex.length() / 2);

        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
            result.push_back(byte);
        }

        return result;
    }

    std::string CryptoManager::Base64Encode(const std::vector<uint8_t>& data) {
        static const char base64Chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        int val = 0;
        int valBits = -6;

        for (uint8_t c : data) {
            val = (val << 8) + c;
            valBits += 8;

            while (valBits >= 0) {
                result.push_back(base64Chars[(val >> valBits) & 0x3F]);
                valBits -= 6;
            }
        }

        if (valBits > -6) {
            result.push_back(base64Chars[((val << 8) >> (valBits + 8)) & 0x3F]);
        }

        while (result.size() % 4) {
            result.push_back('=');
        }

        return result;
    }

    std::vector<uint8_t> CryptoManager::Base64Decode(const std::string& encoded) {
        static const int base64Decode[] = {
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
            52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
            -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
            15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
            -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
        };

        std::vector<uint8_t> result;
        int val = 0;
        int valBits = -8;

        for (unsigned char c : encoded) {
            if (c == '=') break;
            if (c > 127 || base64Decode[c] == -1) continue;

            val = (val << 6) + base64Decode[c];
            valBits += 6;

            if (valBits >= 0) {
                result.push_back((val >> valBits) & 0xFF);
                valBits -= 8;
            }
        }

        return result;
    }

    std::string CryptoManager::GenerateRandomKey(size_t length) {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "!@#$%^&*()-_=+[]{}|;:,.<>?";

        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);

        std::string key;
        key.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            key += charset[distribution(generator)];
        }

        return key;
    }

} // namespace Unicorn::Crypto