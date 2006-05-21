#!/bin/sh

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
