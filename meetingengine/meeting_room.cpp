#include "meeting_room.h"
#include "meeting_def.h"
#include "local_user.h"
#include "remote_user.h"

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


void MeetingRoom::onParticipantConnected(livekit::Room &room, const livekit::ParticipantConnectedEvent &ev) {
    qDebug() << "[Room] participant connected: identity="
        << QString::fromStdString(ev.participant->identity())
        << " name=" << QString::fromStdString(ev.participant->name());
}

void MeetingRoom::onTrackSubscribed(livekit::Room &room, const livekit::TrackSubscribedEvent &ev) {
    const char *participant_identity =
        ev.participant ? ev.participant->identity().c_str() : "<unknown>";
    const std::string track_sid =
        ev.publication ? ev.publication->sid() : "<unknown>";
    const std::string track_name =
        ev.publication ? ev.publication->name() : "<unknown>";
    qDebug() << "[Room] track subscribed: participant_identity="
        << QString::fromStdString(participant_identity)
        << " track_sid=" << QString::fromStdString(track_sid)
        << " name=" << QString::fromStdString(track_name);
    if (ev.track) {
        qDebug() << " kind=" << static_cast<int>(ev.track->kind());
    }
    if (ev.publication) {
        qDebug() << " source=" << static_cast<int>(ev.publication->source());
    }

    // If this is a VIDEO track, create a VideoStream and attach to renderer
    if (ev.track && ev.track->kind() == livekit::TrackKind::KIND_VIDEO) {
        livekit::VideoStream::Options opts;
        opts.format = livekit::VideoBufferType::RGBA;
        auto video_stream = livekit::VideoStream::fromTrack(ev.track, opts);
        if (!video_stream) {
            qCritical() << "Failed to create VideoStream for track " << QString::fromStdString(track_sid);
            return;
        }

        // todo:
        // MainThreadDispatcher::dispatch([this, video_stream] {
        //     if (!media_mgr_->initRenderer(video_stream)) {
        //         qCritical("SDLMediaManager::startRenderer failed for track");
        //     }
        // });
    }

    else if (ev.track && ev.track->kind() == livekit::TrackKind::KIND_AUDIO) {
        livekit::AudioStream::Options opts;
        auto audio_stream = livekit::AudioStream::fromTrack(ev.track, opts);

        // todo:
        // MainThreadDispatcher::dispatch([this, audio_stream] {
        //     if (!media_mgr_->startSpeaker(audio_stream)) {
        //         qCritical("SDLMediaManager::startSpeaker failed for track");
        //     }
        // });
    }
}