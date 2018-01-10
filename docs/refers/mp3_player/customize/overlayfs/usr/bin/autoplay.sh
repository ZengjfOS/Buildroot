#!/bin/busybox ash

# A very simplistic music player. The best (only?) way to stop the playback is
# to kill it, which is precisely why there's autoplay_killall.sh as well.

# This is free and unencumbered software released into the public domain.

# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means. Read the full "license" at http://unlicense.org/

# Written by Miloš Polakovič

find "$1" -iname "*.mp3" | shuf > /tmp/current_playlist.txt
MP3_COUNT=$(cat /tmp/current_playlist.txt | wc -l)
if [ $MP3_COUNT -eq 0 ] ; then
   echo "[autoplay] no MP3s found in $1!" > /dev/kmsg
else
   echo "[autoplay] found $MP3_COUNT MP3s in $1" > /dev/kmsg
   echo "[autoplay] starting playback (stop with autoplay_killall.sh)">/dev/kmsg
   /usr/bin/autoplay_killall.sh  # stop running playback (if any)
   mpg123 -@ /tmp/current_playlist.txt
fi
