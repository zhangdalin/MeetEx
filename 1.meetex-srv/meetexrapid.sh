#!/bin/bash

if pgrep -f ffmpeg >/dev/null 2>&1; then
  echo "Found ffmpeg process(es), killing..."
  pkill -9 -f ffmpeg
else
  echo "No ffmpeg process found."
fi

if pgrep -f 'lk room' >/dev/null 2>&1; then
  echo "Found lk room process(es), killing..."
  pkill -9 -f 'lk room'
else
  echo "No lk room process found."
fi

rm -rf /tmp/videoplayback*_video.sock
rm -rf /tmp/videoplayback*_audio.sock
rm -rf /tmp/lk_*.log

nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback1.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback1_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback1_audio.sock > /dev/null 2>&1 &
echo "Started ffmpeg for videoplayback1.mp4"
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity 'Optimus' --publish h264:///tmp/videoplayback1_video.sock --publish opus:///tmp/videoplayback1_audio.sock meetex > /tmp/lk_Optimus.log 2>&1 &
echo "Started lk room join for Optimus"
sleep 5
nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback2.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback2_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback2_audio.sock > /dev/null 2>&1 &
echo "Started ffmpeg for videoplayback2.mp4"
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity 'Bumblebee‌' --publish h264:///tmp/videoplayback2_video.sock --publish opus:///tmp/videoplayback2_audio.sock meetex > /tmp/lk_Bumblebee.log 2>&1 &
echo "Started lk room join for Bumblebee"
sleep 5
nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback3.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback3_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback3_audio.sock > /dev/null 2>&1 &
echo "Started ffmpeg for videoplayback3.mp4"
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity 'Megatron‌' --publish h264:///tmp/videoplayback3_video.sock --publish opus:///tmp/videoplayback3_audio.sock meetex > /tmp/lk_Megatron.log 2>&1 &
echo "Started lk room join for Megatron"
sleep 5
nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback4.mp4 -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback4_video.sock -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback4_audio.sock > /dev/null 2>&1 &
echo "Started ffmpeg for videoplayback4.mp4"
sleep 5
nohup lk room join --url ws://127.0.0.1:7880 --api-key APIbZ3hZTiHdPXN --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B --identity 'Starscream‌' --publish h264:///tmp/videoplayback4_video.sock --publish opus:///tmp/videoplayback4_audio.sock meetex > /tmp/lk_Starscream.log 2>&1 &
echo "Started lk room join for Starscream"