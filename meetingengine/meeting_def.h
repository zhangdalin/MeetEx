#ifndef MEETING_DEF_H
#define MEETING_DEF_H

#define APP_NAME "MeetEx"

// get token
/*

lk token create --url wss://www.exrapid.cn:8443 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --join --room meetex --identity darin --valid-for 16800h
valid for (mins):  1008000
Token grants:
{
  "roomJoin": true,
  "room": "meetex"
}

Project URL: wss://www.exrapid.cn:8443
Access token: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE4MzY2MzgyNzEsImlkZW50aXR5IjoiZGFyaW4iLCJpc3MiOiJBUEliWjNoWlRpSGRQWE4iLCJuYW1lIjoiZGFyaW4iLCJuYmYiOjE3NzYxNTgyNzEsInN1YiI6ImRhcmluIiwidmlkZW8iOnsicm9vbSI6Im1lZXRleCIsInJvb21Kb2luIjp0cnVlfX0.Qe117UmfWEH2rZ7UhVRGjjV989SwoH-FDIw1F8SGgSA

*/

#define LIVEKIT_URL "wss://www.exrapid.cn:8443"
#define LIVEKIT_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE4MzY2MzgyNzEsImlkZW50aXR5IjoiZGFyaW4iLCJpc3MiOiJBUEliWjNoWlRpSGRQWE4iLCJuYW1lIjoiZGFyaW4iLCJuYmYiOjE3NzYxNTgyNzEsInN1YiI6ImRhcmluIiwidmlkZW8iOnsicm9vbSI6Im1lZXRleCIsInJvb21Kb2luIjp0cnVlfX0.Qe117UmfWEH2rZ7UhVRGjjV989SwoH-FDIw1F8SGgSA"
#define LIVEKIT_E2EE_KEY ""

enum class TrackKind {
    UNKNOWN = 0,
    AUDIO = 1,
    VIDEO = 2,
};

#endif // MEETING_DEF_H