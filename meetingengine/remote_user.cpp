#include "remote_user.h"

RemoteUser::RemoteUser(livekit::RemoteParticipant* participant)
    : participant_(participant) {
    if (!participant_) {
        throw std::invalid_argument("RemoteUser participant must not be null");
    }
}

std::string RemoteUser::identity() const {
    return participant_->identity();
}

std::string RemoteUser::name() const {
    return participant_->name();
}

std::string RemoteUser::metadata() const {
    return participant_->metadata();
}
