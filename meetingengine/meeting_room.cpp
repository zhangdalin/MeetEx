#include "meeting_room.h"
#include "meeting_def.h"
#include "local_user.h"
#include "remote_user.h"
#include "media_engine.h"
#include "media_util.h"

#include <QDebug>

MeetingRoom::MeetingRoom()
    : QObject(nullptr)
    , livekit::Room()
    , url_(LIVEKIT_URL)
    , token_(LIVEKIT_TOKEN)
    , e2ee_key_(LIVEKIT_E2EE_KEY)
    , options_({0})
    , localUser_(nullptr)
    , state_(RoomState::DISCONNECTED) {
    this->setDelegate(this);
}

MeetingRoom::~MeetingRoom() {
    if (state_ == RoomState::CONNECTED) {
        qInfo() << __FUNCTION__ << "MeetingRoom destructor: disconnecting from room...";
        disconnect();
    }
}

void MeetingRoom::setRoomOptions(bool auto_subscribe, bool dynacast, bool e2ee, bool single_peer_connection) {
    options_.auto_subscribe = auto_subscribe;
    options_.dynacast = dynacast;
    options_.single_peer_connection = single_peer_connection;

    if (e2ee) {
        livekit::E2EEOptions encryption;
        encryption.encryption_type = livekit::EncryptionType::GCM;
        // Optional shared key: if empty, we enable E2EE without setting a shared
        // key. (Advanced use: keys can be set/ratcheted later via
        // E2EEManager/KeyProvider.)
        if (!e2ee_key_.empty()) {
            encryption.key_provider_options.shared_key = toBytes(e2ee_key_);
        }
        options_.encryption = encryption;
        if (!e2ee_key_.empty()) {
            qDebug() << __FUNCTION__ << "[E2EE] enabled : (shared key length=" << e2ee_key_.size() << ")";
        } else {
            qDebug() << __FUNCTION__ << "[E2EE] enabled: (no shared key set)";
        }
    }

    qInfo() << __FUNCTION__ << "set room option with Url:" << QString::fromStdString(url_) << "\n"
        << "Token:" << QString::fromStdString(token_) << "\n"
        << "Auto Subscribe:" << (auto_subscribe ? "enabled" : "disabled") << "\n"
        << "Dynacast:" << (dynacast ? "enabled" : "disabled") << "\n"
        << "Single Peer Connection:" << (single_peer_connection ? "enabled" : "disabled") << "\n"
        << "E2EE:" << (e2ee ? "enabled" : "disabled");
}

bool MeetingRoom::connect(){
    state_ = RoomState::CONNECTING;
    bool res = Connect(url_, token_, options_);
    qInfo() << __FUNCTION__ << "Connect result is " << (res ? "successful" : "failed");
    if ( !res ){
        state_ = RoomState::DISCONNECTED;
        qCritical() << __FUNCTION__ << "Failed to connect to room";
        livekit::shutdown();
        return false;
    }

    state_ = RoomState::CONNECTED;

    auto info = room_info();
    qInfo() << __FUNCTION__ << "Connected to room:"
        << "SID: " << (info.sid ? *info.sid : "(none)")
        << "Name: " << info.name
        << "Metadata: " << info.metadata
        << "Max participants: " << info.max_participants
        << "Num participants: " << info.num_participants
        << "Num publishers: " << info.num_publishers
        << "Active recording: " << (info.active_recording ? "yes" : "no")
        << "Empty timeout (s): " << info.empty_timeout
        << "Departure timeout (s): " << info.departure_timeout
        << "Lossy DC low threshold: " << info.lossy_dc_buffered_amount_low_threshold
        << "Reliable DC low threshold: " << info.reliable_dc_buffered_amount_low_threshold
        << "Creation time (ms): " << info.creation_time;

    return res;
}

bool MeetingRoom::disconnect() {
    if (state_ == RoomState::DISCONNECTED) {
        qWarning() << __FUNCTION__ << "MeetingRoom::disconnect() called but already disconnected";
        return true;
    }
    setDelegate(nullptr);
    qInfo() << __FUNCTION__ << "Disconnected from room";
    localUser_.reset();
    state_ = RoomState::DISCONNECTED;

    return true;
}

std::shared_ptr<LocalUser>& MeetingRoom::getLocalUser() {
    if (state_ != RoomState::CONNECTED) {
        qWarning() << __FUNCTION__ << "getLocalUser() called but not connected to room";
        // Return empty shared_ptr to indicate no local user available
        return localUser_;
    }
    else if (!localUser_) {
        localUser_ = std::make_shared<LocalUser>(this->localParticipant());
    }
    return localUser_;
}

std::shared_ptr<RemoteUser> MeetingRoom::getRemoteUser(const std::string &id) {
    if (state_ != RoomState::CONNECTED) {
        qWarning() << __FUNCTION__ << "called but not connected to room";
        return nullptr;
    }
    auto remote_participant = this->remoteParticipant(id);
    if (!remote_participant) {
        qWarning() << __FUNCTION__ << "No remote participant found with id: " << QString::fromStdString(id);
        return nullptr;
    }
    return std::make_shared<RemoteUser>(remote_participant);
}

std::vector<std::shared_ptr<RemoteUser>> MeetingRoom::getRemoteUsers() {
    std::vector<std::shared_ptr<RemoteUser>> remote_users;
    if (state_ != RoomState::CONNECTED) {
        qWarning() << __FUNCTION__ << "called but not connected to room";
        return remote_users;
    }
    auto remote_participants = this->remoteParticipants();
    for (const auto& participant : remote_participants) {
        if (participant) {
            qDebug() << __FUNCTION__ << "remote user: name=" << QString::fromStdString(participant->name())
                    << "id=" << QString::fromStdString(participant->identity());
            remote_users.push_back(std::make_shared<RemoteUser>(participant.get()));
        }
    }
    return remote_users;
}

void MeetingRoom::onParticipantConnected(livekit::Room &room, const livekit::ParticipantConnectedEvent &ev) {
    Q_UNUSED(room);
    if (!ev.participant) {
        qWarning() << __FUNCTION__ << "participant connected event without participant";
        return;
    }

    const QString participant_id = QString::fromStdString(ev.participant->identity());
    const QString participant_name = QString::fromStdString(ev.participant->name());
    qDebug() << __FUNCTION__ << "participant connected: id="
        << participant_id
        << "name=" << participant_name;

    emit sigParticipantJoined(participant_id, participant_name);
}

void MeetingRoom::onParticipantsUpdated(livekit::Room &room, const livekit::ParticipantsUpdatedEvent &ev) {
    Q_UNUSED(room);
    for (const auto& participant : ev.participants) {
        if (participant) {
            qDebug() << __FUNCTION__ << "participant id=" << QString::fromStdString(participant->identity())
                << "name=" << QString::fromStdString(participant->name());
        }
    }
}

void MeetingRoom::onTrackSubscribed(livekit::Room &room, const livekit::TrackSubscribedEvent &ev) {
    Q_UNUSED(room);
    const QString participant_id = ev.participant
        ? QString::fromStdString(ev.participant->identity())
        : QStringLiteral("<unknown>");
    const QString track_sid = ev.publication
        ? QString::fromStdString(ev.publication->sid())
        : QStringLiteral("<unknown>");
    const QString track_name = ev.publication
        ? QString::fromStdString(ev.publication->name())
        : QStringLiteral("<unknown>");
    qDebug() << __FUNCTION__ << "track subscribed: participant_id="
        << participant_id
        << "track_sid=" << track_sid
        << "name=" << track_name
        << "kind=" << (ev.track ? trackKindToString(ev.track->kind()) : "<unknown>")
        << "source=" << (ev.publication ? trackSourceToString(ev.publication->source()) : "<unknown>");
    
    if (ev.track) {
        const int track_kind = static_cast<int>(ev.track->kind());
        emit sigTrackSubscribed(track_sid, track_name, participant_id, track_kind);
    }

    // If this is a VIDEO track, create a VideoStream and attach to renderer
    if (ev.track && ev.track->kind() == livekit::TrackKind::KIND_VIDEO) {
        livekit::VideoStream::Options opts;
        opts.format = livekit::VideoBufferType::RGBA;
        auto video_stream = livekit::VideoStream::fromTrack(ev.track, opts);
        if (!video_stream) {
            qCritical() << __FUNCTION__ << "Failed to create VideoStream for track " << track_sid;
            return;
        }

        if (!MediaEngine::instance().startVideoRender(video_stream, track_sid.toStdString())) {
            qCritical() << __FUNCTION__ << "video render failed for track " << track_sid;
        }
    }

    else if (ev.track && ev.track->kind() == livekit::TrackKind::KIND_AUDIO) {
        livekit::AudioStream::Options opts;
        auto audio_stream = livekit::AudioStream::fromTrack(ev.track, opts);
        if (!audio_stream) {
            qCritical() << __FUNCTION__ << "Failed to create AudioStream for track " << track_sid;
            return;
        }
        if (!MediaEngine::instance().startAudioPlay(audio_stream, track_sid.toStdString())) {
            qCritical() << __FUNCTION__ << "audio play failed for track " << track_sid;
        }
    }
}