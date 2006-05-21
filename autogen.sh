#!/bin/sh

aclocal
autoconf
automake --add-missing

mkdir -p tarballs
#wget -Ptarballs/ -nH ftp://ftp.zresearch.com/pub/gluster/external/*

