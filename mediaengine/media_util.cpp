#include "media_util.h"

std::vector<uint8_t> toBytes(const std::string &s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

const char* trackKindToString(livekit::TrackKind kind) {
    switch (kind) {
    case livekit::TrackKind::KIND_AUDIO:
        return "KIND_AUDIO";
    case livekit::TrackKind::KIND_VIDEO:
        return "KIND_VIDEO";
    case livekit::TrackKind::KIND_UNKNOWN:
    default:
        return "KIND_UNKNOWN";
    }
}

const char* trackSourceToString(livekit::TrackSource source) {
    switch (source) {
    case livekit::TrackSource::SOURCE_CAMERA:
        return "SOURCE_CAMERA";
    case livekit::TrackSource::SOURCE_MICROPHONE:
        return "SOURCE_MICROPHONE";
    case livekit::TrackSource::SOURCE_SCREENSHARE:
        return "SOURCE_SCREENSHARE";
    case livekit::TrackSource::SOURCE_SCREENSHARE_AUDIO:
        return "SOURCE_SCREENSHARE_AUDIO";
    case livekit::TrackSource::SOURCE_UNKNOWN:
    default:
        return "SOURCE_UNKNOWN";
    }
}