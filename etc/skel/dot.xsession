# $SecBSD: dot.xsession,v 0.5 2024/08/21 05:14:40 purplerain Exp $
#
# This file is for testing purposes only.

export LANG=en_US.UTF-8
export LC_CTYPE=en_US.UTF-8

xset b 0 0 0

if [ -x /usr/local/bin/dbus-launch -a -z "${DBUS_SESSION_BUS_ADDRESS}" ]; then
	eval `dbus-launch --sh-syntax --exit-with-x11`
fi

#cwm + xenodm + termbar
#vi ~/.cwmrc to create default cwm configuration file
#configure according to your needs and save :wq!
#manual: https://man.openbsd.org/cwm.1
#uncomment lines below
#xterm -geometry 320x1+0+0 -fa "Spleen:size=16" -bg "black" \
#      -fg grey -fs 12 -name bar -class bar \
#      -T bar -e ~/.bar &
#
#cwm
#rcctl enable xenodm
#rcctl start xenodm
#reboot


#XFCE Desktop + xenodm
#pkg_add -v xfce xfce-extras consolekit2
#uncomment line below
#/usr/local/bin/startxfce4 --with-ck-launch
#rcctl enable messagebus xenodm
#rcctl start messagebus xenodm
#reboot


#KDE Plasma + xenodm
#pkg_add -v kde-plasma
#uncomment lines below
#export XDG_RUNTIME_DIR=/tmp/run/$(id -u)
#if [ ! -d $XDG_RUNTIME_DIR ]; then
#    mkdir -m 700 -p $XDG_RUNTIME_DIR
#fi
#
#export QT_FORCE_STDERR_LOGGING=1
#export XDG_CURRENT_DESKTOP=KDE
#export DESKTOP_SESSION=plasma
#export QT_FORCE_STDERR_LOGGING=1
#/usr/local/bin/startplasma-x11 > ~/.startplasma-x11.log 2>&1
#rcctl enable messagebus xenodm
#rcctl start messagebus xenodm
#reboot

#Known issues
#
#The KDE compositor does not seem to run properly by default. It is recommended
#to deactivate this under: "System Setting" -> "Display Configuration" ->
#"Compositor"

#No compositor seem to run by default.
#You can't unlock your session when the screensaver has locked it. I get the
#graphical message "Something wrong happened. Run # ck-unlock-session ...".
#So you will end up with a freezen session that you have to completely kill.
#It is recommended to disable KDE screen locking for now.
#The first start may take a little longer.
#The sound mixer Kmix works only on the main channel.
