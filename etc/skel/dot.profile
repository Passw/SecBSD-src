# $OpenBSD: dot.profile,v 1.8 2022/08/10 07:40:37 tb Exp $
#
# sh/ksh initialization

PATH=$HOME/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/X11R6/bin:/usr/local/bin:/usr/local/sbin
export PS1="\[\033[38;5;69m\]┌─(\[\033[38;5;15m\]\u\[\033[38;5;69m\]@\[\033[38;5;15m\]\h\[\033[38;5;69m\])\[\033[38;5;69m\]─[\[\033[38;5;69m\]\w\[\033[38;5;69m\]]\n\[\033[38;5;69m\]└─$\[$(tput sgr0)\] "
export PATH HOME TERM
