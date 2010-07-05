#! /bin/sh
#libtoolize --force --copy --automake
aclocal
#autoheader
autoconf
#automake --add-missing --copy

for i in mkinstalldirs install-sh; do
  cp `locate $i | grep "/\$i$" | head -n 1` .
  chmod +x $i
done
