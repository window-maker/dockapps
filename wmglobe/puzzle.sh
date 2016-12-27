#!/bin/sh
#just for the fun, solve the puzzle and put it on the clip :)

if [ -x ./wmglobe ] 
then
WMG="nice ./wmglobe"
else
WMG="nice wmglobe"
fi

HOP=" -delay 0.1 -pos -25 10 -dlat 0.5 -dlong -5 -zoom 1.8 -accel 240"

$WMG $HOP  -fun 32 32 &
$WMG $HOP  -fun -32 32 &
$WMG $HOP  -fun 32 -32 &
$WMG $HOP  -fun -32 -32 &
$WMG  -map ./wmgmap.gif -delay 0.05 -dlong 25 -pos 0 0 -accel 10000&
