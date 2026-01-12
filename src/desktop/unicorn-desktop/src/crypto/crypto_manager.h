#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Unicorn::Crypto {

    // Compression algorithms
    enum class CompressionLevel {
        NoCompression = 0,
        BestSpeed = 1,
        Balanced = 6,
        BestCompression = 9
    };

    // Encryption algorithms
    enum class EncryptionAlgorithm {
        None,
        XOR,      // Simple XOR cipher (for basic obfuscation)
        AES256    // Future: Full AES implementation
    };

    struct CompressionResult {
        bool success = false;
        std::vector<uint8_t> data;
        size_t originalSize = 0;
        size_t compressedSize = 0;
        float compressionRatio = 0.0f;
        std::string error;
    };

    struct EncryptionResult {
        bool success = false;
        std::vector<uint8_t> data;
        std::string error;
    };

    class CryptoManager {
    public:
        static CryptoManager& Get();

        void Init();
        void Shutdown();

        // ========================================
        // Compression Functions
        // ========================================

        // Compress data using ZLIB
        CompressionResult Compress(
            const std::vector<uint8_t>& data,
            CompressionLevel level = CompressionLevel::Balanced
        );

        // Compress string
        CompressionResult CompressString(
            const std::string& text,
            CompressionLevel level = CompressionLevel::Balanced
        );

        // Decompress data
        CompressionResult Decompress(const std::vector<uint8_t>& compressedData);

        // Decompress to string
        std::string DecompressToString(const std::vector<uint8_t>& compressedData);

        // ========================================
        // Encryption Functions (Basic)
        // ========================================

        // Simple XOR encryption (for basic obfuscation)
        EncryptionResult EncryptXOR(
            const std::vector<uint8_t>& data,
            const std::string& key
        );

        // Decrypt XOR
        EncryptionResult DecryptXOR(
            const std::vector<uint8_t>& encryptedData,
            const std::string& key
        );

        // ========================================
        // Hashing Functions
        // ========================================

        // Simple hash (CRC32 from zlib)
        uint32_t HashCRC32(const std::vector<uint8_t>& data);
        uint32_t HashCRC32(const std::string& text);

        // ========================================
        // Utility Functions
        // ========================================

        // Convert bytes to hex string
        static std::string ToHex(const std::vector<uint8_t>& data);

        // Convert hex string to bytes
        static std::vector<uint8_t> FromHex(const std::string& hex);

        // Base64 encode
        static std::string Base64Encode(const std::vector<uint8_t>& data);

        // Base64 decode
        static std::vector<uint8_t> Base64Decode(const std::string& encoded);

        // Generate random key
        static std::string GenerateRandomKey(size_t length = 32);

    private:
        CryptoManager() = default;
        ~CryptoManager();

        bool m_Initialized = false;
    };

} // namespace Unicorn::Crypto