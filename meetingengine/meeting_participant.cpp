#include "meeting_participant.h"

#include "livekit/participant.h"

MeetingParticipant::MeetingParticipant(const livekit::Participant *participant, bool isLocal)
    : isLocal_(isLocal) {
    syncFromLivekit(participant);
}

bool MeetingParticipant::isValid() const {
    return !participantId_.trimmed().isEmpty();
}

const QString& MeetingParticipant::id() const {
    return participantId_;
}

const QString& MeetingParticipant::name() const {
    return participantName_;
}

const QString& MeetingParticipant::metadata() const {
    return participantMetadata_;
}

const QString& MeetingParticipant::audioTrackSid() const {
    return audioTrackSid_;
}

const QString& MeetingParticipant::videoTrackSid() const {
    return videoTrackSid_;
}

bool MeetingParticipant::isLocal() const {
    return isLocal_;
}

void MeetingParticipant::setId(const QString &id) {
    participantId_ = id;
}

void MeetingParticipant::setName(const QString &name) {
    participantName_ = name;
}

void MeetingParticipant::setMetadata(const QString &metadata) {
    participantMetadata_ = metadata;
}

void MeetingParticipant::setAudioTrackSid(const QString &trackSid) {
    audioTrackSid_ = trackSid;
}

void MeetingParticipant::setVideoTrackSid(const QString &trackSid) {
    videoTrackSid_ = trackSid;
}

void MeetingParticipant::setIsLocal(bool isLocal) {
    isLocal_ = isLocal;
}

void MeetingParticipant::syncFromLivekit(const livekit::Participant *participant) {
    if (!participant) {
        return;
    }

    // if no name provided, use participant ID as name fallback
    participantId_ = QString::fromStdString(participant->identity());
    if (participant->name().empty()) {
        participantName_ = participantId_;
    } else {
        participantName_ = QString::fromStdString(participant->name());
    }
    participantMetadata_ = QString::fromStdString(participant->metadata());
}

void MeetingParticipant::clear() {
    participantId_.clear();
    participantName_.clear();
    participantMetadata_.clear();
    audioTrackSid_.clear();
    videoTrackSid_.clear();
}
