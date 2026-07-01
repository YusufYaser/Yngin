#include <stdio.h>

#define YNGIN_LOGGER_STRINGIZE_DETAIL(x) #x
#define YNGIN_LOGGER_STRINGIZE(x) YNGIN_LOGGER_STRINGIZE_DETAIL(x)

#ifdef _DEBUG
#ifdef LOGGER_NAME
#define DEBUG(fmt, ...) do { \
printf("[Yngin Debug] [" YNGIN_LOGGER_STRINGIZE(LOGGER_NAME) "] " fmt "\n", ##__VA_ARGS__); \
} while (0)
#else
#define DEBUG(fmt, ...) do { \
printf("[Yngin Debug] " fmt "\n", ##__VA_ARGS__); \
} while (0)
#endif
#else
#define DEBUG(fmt, ...) ((void)0)
#endif
