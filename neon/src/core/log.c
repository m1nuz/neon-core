#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL_log.h>

#include "core/logerr.h"

static FILE *log_errors;
static FILE *log_messages;
static unsigned int log_level = LOG_LEVEL_INFO;

#ifdef LOGGER_FILE_OUPUT
static void
log_cleanup(void) {
    if (log_messages && (log_errors != log_messages)) {
        fclose(log_messages);
        log_messages = NULL;
    }

    if(log_errors) {
        fclose(log_errors);
        log_errors = NULL;
    }
}
#endif

static void
log_output(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    (void)userdata;
    (void)category;

    switch(priority) {
    case SDL_LOG_PRIORITY_ERROR:
    case SDL_LOG_PRIORITY_CRITICAL:
        fputs("ERROR: ", log_errors);
        fputs(message, log_errors);
        fputs("\n", log_errors);
        break;
    case SDL_LOG_PRIORITY_VERBOSE:
    case SDL_LOG_PRIORITY_DEBUG:
        if (log_level < LOG_LEVEL_DEBUG)
            return;

        fputs("DEBUG: ", log_messages);
        fputs(message, log_messages);
        fputs("\n", log_messages);
        break;
    case SDL_LOG_PRIORITY_WARN:
    case SDL_LOG_PRIORITY_INFO:
    default:
        if (log_level < LOG_LEVEL_INFO)
            return;

        //fputs("INFO: ", log_messages);
        fputs(message, log_messages);
        fputs("\n", log_messages);
    }
}

extern void
log_init(const char *filepath) {
    log_errors = stderr;
    log_messages = stdout;

    if (filepath) {
#ifdef LOGGER_REOPEN_STDERR
        if (freopen(filepath, "w", stderr) == NULL) {
            perror("Log reopen stderr");
            exit(EXIT_SUCCESS);
        }

        log_errors = stderr;
#endif

#ifdef LOGGER_REOPEN_STDOUT
        if (freopen(filepath, "w", stdout) == NULL) {
            perror("Log reopen stdout");
            exit(EXIT_SUCCESS);
        }

        log_messages = stdout;
#endif

#ifdef LOGGER_FILE_OUPUT
        if ((log_errors = fopen(filepath, "w")) == NULL) {
            perror("Log init");
            exit(EXIT_SUCCESS);
        }

        log_messages = log_errors;
        setvbuf(log_errors, NULL, _IONBF, 0);
        atexit(log_cleanup);
#endif
    }

#ifdef LOGGER_SDL_LOG_RESET_OUTPUT
    SDL_LogSetOutputFunction(log_output, NULL);
#endif
}

extern void
log_set_level(unsigned int level) {
    log_level = level;
}
