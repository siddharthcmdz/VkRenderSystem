#pragma once

#if !defined(RS_FILE) && defined(_WIN32)
#define RS_EXPORT __declspec(dllimport)
#elif defined(_WIN32)
#define RS_EXPORT __declspec(dllexport)
#endif


#if defined(VK_USE_PLATFORM_IOS_MVK)
#define RS_EXPORT __attribute__((visibility("default")))
#endif
