#!/bin/sh
# Copyright (C) 2006 Z RESEARCH Inc. <http://www.zresearch.com>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#  

_init ()
{
    TMP_DIR=$(mktemp -d /tmp/gex.XXXXXX) || { 
	local ERR=$?;
	echo "mktemp failed ${ERR}" >&2;
	return ${ERR};
    }
    trap _fini 0;
}

_fini ()
{
    rm -rf ${TMP_DIR};
}

err ()
{
    echo "$@" >&2;
}

main ()
{
    local gex_file="$1";
    local err;

    [ -f ${gex_file} ] || {
	err "${gex_file} not a file";
	return 1;
    }

    [ -z "$(tar tzvf ${gex_file} ./runme 2>/dev/null)" ] && {
	err "${gex_file} is not a valid gluster package";
	return 1;
    }
	
    err=$(tar -C "${TMP_DIR}" -xzf "${gex_file}" 2>&1) || {
	err "${gex_file} corrupted? error='${err}'";
	return 1;
    }

    cd "${TMP_DIR}";

    ./runme;

    cd -;

    rm -rf ${TMP_DIR};

    exec kick-start;
}

_init "$@" && main "$@" ; read
