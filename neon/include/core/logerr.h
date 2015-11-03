/*
 * Logging errors
 */
#pragma once

#define LOGGER
#define LOGGER_REOPEN_STDERR
#define LOGGER_REOPEN_STDOUT
//#define LOGGER_FILE_OUPUT
#define LOGGER_SDL_LOG_RESET_OUTPUT
//#define LOGGER_SHOW_FILE_LINE

#ifdef LOGGER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL_log.h>

enum LogLevel {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
};

#define LOG_INIT(logname) log_init(logname)
#define LOG_SET_LEVEL(level) log_set_level(level)
#define LOG_QUIETLY() log_set_level(LOG_LEVEL_ERROR)
#define LOG_NOISY() log_set_level(LOG_LEVEL_INFO)

#ifdef LOGGER_SHOW_FILE_LINE

#define __FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define LOG_ERROR(frmt, ...) SDL_LogError(SDL_LOG_CATEGORY_ERROR, "(%s %d) " frmt, __FILE, __LINE__, __VA_ARGS__)
#define LOG_CRITICAL(frmt, ...) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "(%s %d) " frmt, __FILE, __LINE__, __VA_ARGS__)
#define LOG_WARNING(frmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "(%s %d) " frmt, __FILE, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(frmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "(%s %d) " frmt, __FILE, __LINE__, __VA_ARGS__)

#else

//#define LOG_ERROR(frmt, ...) SDL_LogError(SDL_LOG_CATEGORY_ERROR, frmt, __VA_ARGS__)
//#define LOG_CRITICAL(frmt, ...) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, frmt, __VA_ARGS__)
//#define LOG_WARNING(frmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, frmt, __VA_ARGS__)
//#define LOG_DEBUG(frmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, frmt, __VA_ARGS__)

#define LOG_ERROR(frmt, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, frmt, __VA_ARGS__)
#define LOG_CRITICAL(frmt, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, frmt, __VA_ARGS__)
#define LOG_WARNING(frmt, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, frmt, __VA_ARGS__)
#define LOG_DEBUG(frmt, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, frmt, __VA_ARGS__)

/*#define LOG_ERROR(frmt, ...) fprintf(stderr, frmt, __VA_ARGS__)
#define LOG_CRITICAL(frmt, ...) fprintf(stderr, frmt, __VA_ARGS__)
#define LOG_WARNING(frmt, ...) fprintf(stdout, frmt, __VA_ARGS__)
#define LOG_DEBUG(frmt, ...) fprintf(stdout, frmt, __VA_ARGS__)*/

#endif

#define LOG(frmt, ...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, frmt, __VA_ARGS__)

void log_init(const char *filepath);
void log_set_level(unsigned int level);

#else

#define LOG_INIT()
#define LOG_ERROR(frmt, ...) (void)0
#define LOG_CRITICAL(frmt, ...) (void)0
#define LOG_WARNING(frmt, ...) (void)0
#define LOG_DEBUG(frmt, ...) (void)0
#define LOG(frmt, ...) (void)0

#endif // LOGGER

#define checkif(test, message, ...) do { if (test) { \
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, message, __VA_ARGS__); \
    exit(EXIT_FAILURE);} \
    } while(0)

#define checkif_return(test, ret, message, ...) do { if (test) { \
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, message, __VA_ARGS__);\
    return ret;}\
    } while(0)

#define checkif_do(test, ...) do { if (test) { \
    __VA_ARGS__; }\
    } while(0)
