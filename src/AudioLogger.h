#pragma once

#include <Print.h>

class DevNullOut : public Print
{
public:
	size_t write(uint8_t) override
	{
		return 1;
	}
};

extern DevNullOut silencedLogger;

// Global `audioLogger` is initialized to &silencedLogger
// It can be initialized anytime to &Serial or any other Print:: derivative instance.
extern Print* audioLogger;

#define AUDIO_LOG(fmt, ...)                                                                                          \
	do {                                                                                                               \
		if(audioLogger != nullptr) {                                                                                   \
			audioLogger->printf(_F("%s: " fmt), __PRETTY_FUNCTION__, ##__VA_ARGS__);                                   \
		}                                                                                                              \
	} while(0)

#define AUDIO_ERROR(fmt, ...)  AUDIO_LOG("ERROR" fmt, ##__VA_ARGS__)
#define AUDIO_INFO(fmt, ...)  AUDIO_LOG(fmt, ##__VA_ARGS__)
