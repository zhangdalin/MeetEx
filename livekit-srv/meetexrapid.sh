#!/bin/bash

ps aux | grep 'ffmpeg' | grep -v grep | awk '{print $2}' | xargs kill -9
ps aux | grep 'lk room' | grep -v grep | awk '{print $2}' | xargs kill -9
rm -rf /tmp/videoplayback*

#lk token create --url wss://www.exrapid.cn:8443 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --join --room meetex --identity darin-host --valid-for 16800h
#sleep 2

nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback1.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback1_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback1_audio.sock > /dev/null 2>&1 &
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity videoplayback1 --publish h264:///tmp/videoplayback1_video.sock --publish opus:///tmp/videoplayback1_audio.sock meetex > /dev/null 2>&1 &
sleep 5
nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback2.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback2_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback2_audio.sock > /dev/null 2>&1 &
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity videoplayback2 --publish h264:///tmp/videoplayback2_video.sock --publish opus:///tmp/videoplayback2_audio.sock meetex > /dev/null 2>&1 &
sleep 5
nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback3.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback3_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback3_audio.sock > /dev/null 2>&1 &
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity videoplayback3 --publish h264:///tmp/videoplayback3_video.sock --publish opus:///tmp/videoplayback3_audio.sock meetex > /dev/null 2>&1 &
sleep 5
nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback4.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback4_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback4_audio.sock > /dev/null 2>&1 &
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity videoplayback4 --publish h264:///tmp/videoplayback4_video.sock --publish opus:///tmp/videoplayback4_audio.sock meetex > /dev/null 2>&1 &