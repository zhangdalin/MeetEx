#include "meeting_room.h"
#include "meeting_def.h"
#include "local_user.h"
#include "remote_user.h"
#include "media_engine.h"

#include <QDebug>

static std::vector<uint8_t> toBytes(const std::string &s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

MeetingRoom::MeetingRoom()
    : livekit::Room()
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
        qInfo() << "MeetingRoom destructor: disconnecting from room...";
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
            qDebug() << "[E2EE] enabled : (shared key length=" << e2ee_key_.size() << ")";
        } else {
            qDebug() << "[E2EE] enabled: (no shared key set)";
        }
    }

    qInfo() << "set room option with Url:" << QString::fromStdString(url_) << "\n"
        << " Token:" << QString::fromStdString(token_) << "\n"
        << " Auto Subscribe:" << (auto_subscribe ? "enabled" : "disabled") << "\n"
        << " Dynacast:" << (dynacast ? "enabled" : "disabled") << "\n"
        << " Single Peer Connection:" << (single_peer_connection ? "enabled" : "disabled") << "\n"
        << " E2EE:" << (e2ee ? "enabled" : "disabled");
}

bool MeetingRoom::connect(){
    state_ = RoomState::CONNECTING;
    bool res = Connect(url_, token_, options_);
    qInfo() << "Connect result is " << (res ? "successful" : "failed");
    if ( !res ){
        state_ = RoomState::DISCONNECTED;
        qCritical() << "Failed to connect to room";
        livekit::shutdown();
        return false;
    }

    state_ = RoomState::CONNECTED;

    auto info = room_info();
    qInfo() << "Connected to room:\n"
        << "  SID: " << (info.sid ? *info.sid : "(none)") << "\n"
        << "  Name: " << info.name << "\n"
        << "  Metadata: " << info.metadata << "\n"
        << "  Max participants: " << info.max_participants << "\n"
        << "  Num participants: " << info.num_participants << "\n"
        << "  Num publishers: " << info.num_publishers << "\n"
        << "  Active recording: " << (info.active_recording ? "yes" : "no") << "\n"
        << "  Empty timeout (s): " << info.empty_timeout << "\n"
        << "  Departure timeout (s): " << info.departure_timeout << "\n"
        << "  Lossy DC low threshold: " << info.lossy_dc_buffered_amount_low_threshold << "\n"
        << "  Reliable DC low threshold: " << info.reliable_dc_buffered_amount_low_threshold << "\n"
        << "  Creation time (ms): " << info.creation_time << "\n";

    return res;
}

bool MeetingRoom::disconnect() {
    if (state_ == RoomState::DISCONNECTED) {
        qWarning() << "MeetingRoom::disconnect() called but already disconnected";
        return true;
    }
    setDelegate(nullptr);
    qInfo() << "Disconnected from room";
    localUser_.reset();
    state_ = RoomState::DISCONNECTED;

    return true;
}

std::shared_ptr<LocalUser>& MeetingRoom::getLocalUser() {
    if (state_ != RoomState::CONNECTED) {
        qWarning() << "getLocalUser() called but not connected to room";
        // Return empty shared_ptr to indicate no local user available
        return localUser_;
    }
    else if (!localUser_) {
        localUser_ = std::make_shared<LocalUser>(localParticipant());
    }
    return localUser_;
}

std::shared_ptr<RemoteUser> MeetingRoom::getRemoteUser(const std::string &identity) {
    if (state_ != RoomState::CONNECTED) {
        qWarning() << "getRemoteUser() called but not connected to room";
        return nullptr;
    }
    auto remote_participant = remoteParticipant(identity);
    if (!remote_participant) {
        qWarning() << "No remote participant found with identity: " << QString::fromStdString(identity);
        return nullptr;
    }
    return std::make_shared<RemoteUser>(remote_participant);
}

void MeetingRoom::setParticipantJoinedCallback(std::function<void(const QString &, const QString &)> callback) {
    participant_joined_callback_ = std::move(callback);
}

void MeetingRoom::setTrackSubscribedCallback(std::function<void(const QString &, const QString &, const QString &, int)> callback) {
    track_subscribed_callback_ = std::move(callback);
}


void MeetingRoom::onParticipantConnected(livekit::Room &room, const livekit::ParticipantConnectedEvent &ev) {
    Q_UNUSED(room);
    if (!ev.participant) {
        qWarning() << "[MeetingRoom] participant connected event without participant";
        return;
    }

    const QString participant_identity = QString::fromStdString(ev.participant->identity());
    const QString participant_name = QString::fromStdString(ev.participant->name());
    qDebug() << "[MeetingRoom] participant connected: identity="
        << participant_identity
        << " name=" << participant_name;

    if (participant_joined_callback_) {
        participant_joined_callback_(participant_identity, participant_name);
    }
}

void MeetingRoom::onTrackSubscribed(livekit::Room &room, const livekit::TrackSubscribedEvent &ev) {
    Q_UNUSED(room);
    const QString participant_identity = ev.participant
        ? QString::fromStdString(ev.participant->identity())
        : QStringLiteral("<unknown>");
    const QString track_sid = ev.publication
        ? QString::fromStdString(ev.publication->sid())
        : QStringLiteral("<unknown>");
    const QString track_name = ev.publication
        ? QString::fromStdString(ev.publication->name())
        : QStringLiteral("<unknown>");
    qDebug() << "[MeetingRoom] track subscribed: participant_identity="
        << participant_identity
        << " track_sid=" << track_sid
        << " name=" << track_name;

    if (ev.track) {
        qDebug() << " kind=" << static_cast<int>(ev.track->kind());
    }
    if (ev.publication) {
        qDebug() << " source=" << static_cast<int>(ev.publication->source());
    }
    
    if (ev.track && track_subscribed_callback_) {
        const int track_kind = ev.track ? static_cast<int>(ev.track->kind()) : 0;
        track_subscribed_callback_(track_sid, track_name, participant_identity, track_kind);
    }

    // If this is a VIDEO track, create a VideoStream and attach to renderer
    if (ev.track && ev.track->kind() == livekit::TrackKind::KIND_VIDEO) {
        livekit::VideoStream::Options opts;
        opts.format = livekit::VideoBufferType::RGBA;
        auto video_stream = livekit::VideoStream::fromTrack(ev.track, opts);
        if (!video_stream) {
            qCritical() << "Failed to create VideoStream for track " << track_sid;
            return;
        }

        if (!MediaEngine::instance().startVideoRender(video_stream, track_sid.toStdString())) {
            qCritical() << "MeetingRoom::startVideoRender failed for track " << track_sid;
        }
    }

    else if (ev.track && ev.track->kind() == livekit::TrackKind::KIND_AUDIO) {
        livekit::AudioStream::Options opts;
        auto audio_stream = livekit::AudioStream::fromTrack(ev.track, opts);
        if (!audio_stream) {
            qCritical() << "Failed to create AudioStream for track " << track_sid;
            return;
        }
        if (!MediaEngine::instance().startAudioSpeaker(audio_stream)) {
            qCritical() << "MeetingRoom::startAudioSpeaker failed for track " << track_sid;
        }
    }
}