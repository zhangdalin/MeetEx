#ifndef MEDIA_UTIL_H
#define MEDIA_UTIL_H

#include <string>
#include <vector>

#include "livekit/livekit.h"

std::vector<uint8_t> toBytes(const std::string &s);
const char* trackKindToString(livekit::TrackKind kind);
const char* trackSourceToString(livekit::TrackSource source);

#endif // MEDIA_UTIL_H