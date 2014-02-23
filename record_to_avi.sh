#!/bin/bash

glc-capture -j -a 'hw:0,48000,2' -o ../capture.glc ./a.out

glc-play ../capture.glc -y 1 -o - | mencoder -demuxer y4m - \
    -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=3000 -o ../capture.avi
