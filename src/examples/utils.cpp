//
// Created by zgd on 11/7/22.
//

#include "utils.h"
#include <time.h>

constexpr int64_t SEC_TO_MS = 1000;
constexpr int64_t MS_TO_NS = 1000000;
constexpr int64_t SEC_TO_US = 1000000;
constexpr int64_t US_TO_NS = 1000;

#ifdef __APPLE__
constexpr clockid_t CLOCK_TYPE = CLOCK_UPTIME_RAW;
#else
constexpr clockid_t CLOCK_TYPE = CLOCK_BOOTTIME;
#endif

int64_t Utils::GetNowMs() {
  struct timespec t;
  (void)clock_gettime(CLOCK_TYPE, &t);
  return t.tv_sec * SEC_TO_MS + t.tv_nsec / MS_TO_NS;
}

int64_t Utils::GetNowUs() {
  struct timespec t;
  (void)clock_gettime(CLOCK_TYPE, &t);
  return t.tv_sec * SEC_TO_US + t.tv_nsec / US_TO_NS;
}
