#! /bin/sh
set -x
aclocal -I autotools
libtoolize --force --copy
autoheader
automake --add-missing --copy
autoconf

