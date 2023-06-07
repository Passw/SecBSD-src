# $SecBSD: dot.xsession,v 0.2 2022/12/21 00:00:00 secbsd Exp $
#
# This file is for testing purposes only.

xset b 0 0 0

if [ -x /usr/local/bin/dbus-launch -a -z "${DBUS_SESSION_BUS_ADDRESS}" ]; then
	eval `dbus-launch --sh-syntax --exit-with-x11`
fi

# XFCE Desktop + xenodm
# pkg_add -v xfce xfce-extras consolekit2
# uncomment line below
#exec /usr/local/bin/startxfce4 --with-ck-launch
# rcctl enable messagebus xenodm
# rcctl start messagebus xenodm
# reboot
