#include <stdio.h>

#define YNGIN_LOGGER_STRINGIZE_DETAIL(x) #x
#define YNGIN_LOGGER_STRINGIZE(x) YNGIN_LOGGER_STRINGIZE_DETAIL(x)

#ifdef LOGGER_NAME
#	define LOGGER_NAME_STR "[" YNGIN_LOGGER_STRINGIZE(LOGGER_NAME) "]"
#else
#	define LOGGER_NAME_STR ""
#endif

#ifdef _DEBUG

#define TRACE(fmt, ...) do { \
printf("[Yngin Trace] " LOGGER_NAME_STR " " fmt "\n", ##__VA_ARGS__); \
} while (0)

#define DEBUG(fmt, ...) do { \
printf("[Yngin Debug] " LOGGER_NAME_STR " " fmt "\n", ##__VA_ARGS__); \
} while (0)

#else
#define TRACE(fmt, ...) ((void)0)
#define DEBUG(fmt, ...) ((void)0)
#endif
