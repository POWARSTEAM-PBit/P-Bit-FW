#pragma once
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
extern std::map<std::string, std::vector<uint8_t>> native_prefs;
extern unsigned native_writes;
extern bool native_write_failure;
class Preferences {
public:
    bool begin(const char*, bool) { return true; }
    void end() {}
    template <class T> T get(const char* key, T fallback) {
        const auto found = native_prefs.find(key);
        if (found == native_prefs.end() || found->second.size() != sizeof(T)) return fallback;
        T result; std::memcpy(&result, found->second.data(), sizeof(T)); return result;
    }
    template <class T> size_t put(const char* key, T value) { return putBytes(key, &value, sizeof(T)); }
    int getInt(const char* key, int fallback) { return get(key, fallback); }
    bool getBool(const char* key, bool fallback) { return get(key, fallback); }
    uint32_t getUInt(const char* key, uint32_t fallback) { return get(key, fallback); }
    uint8_t getUChar(const char* key, uint8_t fallback) { return get(key, fallback); }
    size_t putInt(const char* key, int value) { return put(key, value); }
    size_t putBool(const char* key, bool value) { return put(key, value); }
    size_t putUInt(const char* key, uint32_t value) { return put(key, value); }
    size_t putUChar(const char* key, uint8_t value) { return put(key, value); }
    bool isKey(const char* key) { return native_prefs.count(key) != 0; }
    size_t getBytesLength(const char* key) { auto found = native_prefs.find(key); return found == native_prefs.end() ? 0 : found->second.size(); }
    size_t getBytes(const char* key, void* destination, size_t count) {
        auto found = native_prefs.find(key);
        if (found == native_prefs.end() || found->second.size() > count) return 0;
        std::memcpy(destination, found->second.data(), found->second.size()); return found->second.size();
    }
    size_t putBytes(const char* key, const void* data, size_t count) {
        ++native_writes; if (native_write_failure) return 0;
        const auto* bytes = static_cast<const uint8_t*>(data); native_prefs[key] = std::vector<uint8_t>(bytes, bytes + count); return count;
    }
    bool remove(const char* key) { ++native_writes; if (native_write_failure) return false; native_prefs.erase(key); return true; }
    bool clear() { native_prefs.clear(); return true; }
};
