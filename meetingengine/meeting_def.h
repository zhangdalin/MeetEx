#ifndef MEETING_DEF_H
#define MEETING_DEF_H

#define APP_NAME "MeetEx"

// get token
/*

lk token create --url wss://www.exrapid.cn:8443 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --join --room exmeet --identity darin --valid-for 168h
valid for (mins):  10080
Token grants:
{
  "identity": "darin",
  "name": "darin",
  "video": {
    "roomJoin": true,
    "room": "exmeet"
  }
}

Project URL: wss://www.exrapid.cn:8443
Access token: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NzUwMzQ0NzUsImlkZW50aXR5IjoiZGFyaW4iLCJpc3MiOiJBUEliWjNoWlRpSGRQWE4iLCJuYW1lIjoiZGFyaW4iLCJuYmYiOjE3NzQ0Mjk2NzUsInN1YiI6ImRhcmluIiwidmlkZW8iOnsicm9vbSI6ImV4bWVldCIsInJvb21Kb2luIjp0cnVlfX0.NOErAjVrQjYOiM-vnvN39MqgfNJbphXOOXSxLdSv-0E

*/

#define LIVEKIT_URL "wss://www.exrapid.cn:8443"
#define LIVEKIT_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NzUwMzQ0NzUsImlkZW50aXR5IjoiZGFyaW4iLCJpc3MiOiJBUEliWjNoWlRpSGRQWE4iLCJuYW1lIjoiZGFyaW4iLCJuYmYiOjE3NzQ0Mjk2NzUsInN1YiI6ImRhcmluIiwidmlkZW8iOnsicm9vbSI6ImV4bWVldCIsInJvb21Kb2luIjp0cnVlfX0.NOErAjVrQjYOiM-vnvN39MqgfNJbphXOOXSxLdSv-0E"
#define LIVEKIT_E2EE_KEY ""

#endif // MEETING_DEF_H