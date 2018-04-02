#!/bin/bash

statusfile=$(mktemp)
xterm -e sh -c 'sudo pacman -Syu; echo $? > '$statusfile
status=$(cat $statusfile)
rm $statusfile
exit $status
