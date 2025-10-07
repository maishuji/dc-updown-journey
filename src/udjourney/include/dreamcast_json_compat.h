#pragma once

#ifdef PLATFORM_DREAMCAST

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <climits>
#include <cstdarg>
#include <cstdio>

// Provide missing C library functions for Dreamcast/KallistiOS

namespace std {

// Fallback implementation of strtold using strtod
inline long double strtold(const char* str, char** endptr) noexcept {
    return static_cast<long double>(strtod(str, endptr));
}

// Fallback implementation of strtoull using strtoul
inline unsigned long long strtoull(const char* str, char** endptr, int base) noexcept {
    // For Dreamcast, we'll use the available strtoul and handle overflow gracefully
    errno = 0;
    unsigned long result = strtoul(str, endptr, base);
    
    // If we hit the limit of unsigned long, we've likely overflowed
    if (result == ULONG_MAX && errno == ERANGE) {
        // Return the max value we can represent
        return static_cast<unsigned long long>(result);
    }
    
    return static_cast<unsigned long long>(result);
}

// Fallback implementation of strtoll using strtol  
inline long long strtoll(const char* str, char** endptr, int base) noexcept {
    // For Dreamcast, we'll use the available strtol and handle overflow gracefully
    errno = 0;
    long result = strtol(str, endptr, base);
    
    // If we hit the limits of long, we've likely overflowed
    if ((result == LONG_MAX || result == LONG_MIN) && errno == ERANGE) {
        // Return the max/min value we can represent
        return static_cast<long long>(result);
    }
    
    return static_cast<long long>(result);
}

// Fallback implementation for snprintf
inline int snprintf(char* buffer, size_t count, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, count, format, args);
    va_end(args);
    return result;
}

} // namespace std

#endif // PLATFORM_DREAMCAST