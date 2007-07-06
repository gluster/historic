#!/bin/sh

cd $(dirname $0)
aclocal
autoconf
automake --add-missing --copy --foreign

openssl genrsa -out ./slurm.key 1024
openssl rsa -in ./slurm.key -pubout -out ./slurm.cert
