#pragma once

#define NEON_API extern

#define UNUSED(x) (void)(x)
#define countof(x) (sizeof (x) / sizeof ((x)[0]))

#define SIZEOF_FLEXIBLE(type, member, length) \
  (offsetof(type, member) + (length) * sizeof ((type *)0)->member[0])

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL_platform.h>

#ifdef __WINDOWS__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static inline char*
strtok_r(char *str, const char *delim, char **nextp) {
    if (str == NULL)
        str = *nextp;

    str += strspn(str, delim);

    if (*str == '\0')
        return NULL;

    char *ret = str;

    str += strcspn(str, delim);

    if (*str)
        *str++ = '\0';

    *nextp = str;

    return ret;
}

static inline size_t
utf8_strlen(const char *ptr) {
    return MultiByteToWideChar(CP_UTF8, ptr, NULL, 0);
}

static inline size_t
utf8_towcs(const char *s, wchar_t *ws, size_t max_count) {
    size_t len = MultiByteToWideChar(CP_UTF8, s, NULL, 0) + 1;

    if (!ws)
        return len;

    if (len > max_count)
        return MultiByteToWideChar(CP_UTF8, s, ws, max_count);

    MultiByteToWideChar(CP_UTF8, s, ws, len);

    return len;
}
#endif // __WINDOWS__

#if defined(__LINUX__) || defined(__ANDROID__)
#include <wchar.h>

static inline size_t
utf8_strlen(const char *ptr) {
    size_t result = 0;
    const char* end = ptr + strlen(ptr);

    mbstate_t mb;
    memset(&mb, 0, sizeof mb);

    while (ptr < end) {
        int next = mbrlen(ptr, end - ptr, &mb);
        if (next == -1) {
            perror("utf8_strlen");
            break;
        }
        ptr += next;
        ++result;
    }

    return result;
}

static inline size_t
utf8_towcs(const char *s, wchar_t *ws, size_t max_count) {
    mbstate_t state;
    memset(&state, 0, sizeof state);

    size_t len = 1 + mbsrtowcs(NULL, &s, 0, &state);

    if (!ws)
        return len;

    if (len > max_count)
        mbsrtowcs(&ws[0], &s, max_count, &state);

    return mbsrtowcs(&ws[0], &s, len, &state);
}

static inline size_t
utf8_chars_len(const char *ptr, uint8_t *chars) {
    size_t result = 0;
    const char* end = ptr + strlen(ptr);

    mbstate_t mb;
    memset(&mb, 0, sizeof mb);

    while (ptr < end) {
        int next = mbrlen(ptr, end - ptr, &mb);
        if (next == -1) {
            perror("utf8_chars_len");
            break;
        }
        chars[result] = next;
        ptr += next;
        ++result;
    }

    return result;
}
#endif // __LINUX__
