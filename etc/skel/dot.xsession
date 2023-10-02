# $SecBSD: dot.xsession,v 0.3 2023/10/02 03:12:22 purplerain Exp $
#
# This file is for testing purposes only.

xset b 0 0 0

if [ -x /usr/local/bin/dbus-launch -a -z "${DBUS_SESSION_BUS_ADDRESS}" ]; then
	eval `dbus-launch --sh-syntax --exit-with-x11`
fi

# cwm + xenodm + termbar
# vi ~/.cwmrc to create default cwm configuration file
# configure according to your needs and save :wq!
# manual: https://man.openbsd.org/cwm.1
# uncomment the next line to the line exec cwm
#exec xterm -geometry 320x1+0+0 -fa "Spleen:size=16" -bg "black" \
#      -fg grey -fs 12 -name termbar -class termbar \
#      -T termbar -e ~/.termbar &
#
#exec cwm
# rcctl enable xenodm
# rcctl start xenodm
# reboot


# MATE Desktop + xenodm
# pkg_add -v mate
# uncomment line below
#exec mate-session
# rcctl enable messagebus xenodm
# rcctl start messagebus xenodm
# reboot


# XFCE Desktop + xenodm
# pkg_add -v xfce xfce-extras consolekit2
# uncomment line below
#exec /usr/local/bin/startxfce4 --with-ck-launch
# rcctl enable messagebus xenodm
# rcctl start messagebus xenodm
# reboot
