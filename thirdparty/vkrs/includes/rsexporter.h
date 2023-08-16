#pragma once

#ifndef RS_FILE
#define RS_EXPORT __declspec(dllimport)
#else
#define RS_EXPORT __declspec(dllexport)
#endif
