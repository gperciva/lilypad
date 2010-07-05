#! /bin/sh
#libtoolize --force --copy --automake
aclocal
#autoheader
autoconf
#automake --add-missing --copy
cp $(locate mkinstalldirs | grep /mkinstalldirs$ | head -1) .
chmod +x mkinstalldirs
