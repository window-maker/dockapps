#!/bin/sh
#just for the fun, solve the puzzle and put it on the clip :)

if [ -x ./wmglobe ]
then
WMG="nice ./wmglobe"
else
WMG="nice wmglobe"
fi

HOP=" -bord 0 -delay 0.1 -pos -25 10 -dlat 0.5 -dlong -5 -zoom 1.8 -accel 240 -stable"

$WMG $HOP  -fun 32 32 &
$WMG $HOP  -fun -32 32 &
$WMG $HOP  -fun 32 -32 &
$WMG $HOP  -fun -32 -32 &
[ -f wmgmap.jpeg ] && $WMG -nimap ./wmgmap.jpeg  -delay 0.05 -dlat 2 -dlong -20 -pos 0 -20 -accel 21600 -zoom 0.85 -dawn 0 -stable &
