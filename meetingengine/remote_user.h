#ifndef REMOTE_USER_H
#define REMOTE_USER_H  

#include "livekit/livekit.h"

class RemoteUser {
public:
    explicit RemoteUser(livekit::RemoteParticipant* participant);

    std::string identity() const;
    std::string name() const;
    std::string metadata() const;

private:
    livekit::RemoteParticipant* participant_;

};

#endif // REMOTE_USER_H