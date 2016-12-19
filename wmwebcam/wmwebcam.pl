#!/usr/bin/perl

# this script checks if the ppp connection is on and then stamps the
# images and sends them to the webserver

# check if the ppp is on before stamping&sending, otherwise don't do
# anything
$areweonline = `grep "ppp" /proc/net/route`;

if (length $areweonline != 0) { # THIS IS JUST MY CONFIGURATION,
                                # CHANGE TO YOUR OWN

system "stamp";    # stamp reads /tmp/wmwebcam.jpg and outputs
                   # /tmp/webcam.jpg with some info (has to be
                   # configured to do so)
# get stamp from:
# http://sourceforge.net/projects/stamp

# NOTE: stamp has the ability to send the image to server via ftp, but I
# prefer using scp.

system "scp -q /tmp/webcam.jpg SOME_HOST:public_html/webcam.jpg";
                   # replace the previous line to suit your needs
                   # or disable if you use stamp's own ftpsend
} else {
# don't do anything
}
