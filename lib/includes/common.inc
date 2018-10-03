#!/bin/bash
#
# tupper - A container image manager
#
# Copyright (c) 2018 Matt Ferris <matt@bueller.ca>
# Licensed under BSD 2-clause license
# github.com/mattferris/tupper/blob/master/License.txt
#


#
# configuration
#

# conf wrapper
tup_conf() {
    local action=$1
    local type=$2
    local id=$3

    shift; shift; shift

    case $type in
        container) local cfg="$TUP_STORAGE_DIR/containers/$(tup_path "$id")/conf";;
        image) local cfg="$TUP_STORAGE_DIR/images/$(tup_path "$id")/conf";;
        layer) local cfg="$TUP_STORAGE_DIR/layers/$(tup_path "$id")/conf";;
        path) local cfg=$id;;
        volume) local cfg="$TUP_STORAGE_DIR/volumes/$(tup_path "$id")/conf";;
        *) echo "error: bad config object type: $type" >&2; exit 4;;
    esac

    if [[ ! -f "$cfg" ]]; then
        echo "error: config file doesn't exist: $cfg" >&2
        exit 4
    fi

    tup_debug 2 "tup_conf_${action} $cfg $@"
    tup_conf_${action} "$cfg" $@
}

# add value to key
tup_conf_add() {
    local cfg=$1
    local key=$2
    shift; shift
    local val=$(tup_conf_get $cfg $key)
    tup_conf_set $cfg $key "$val $@"
}

# get conf value
tup_conf_get() {
    local cfg=$1
    local key=$2
    if ! (grep "^$key " "$cfg" | cut -d\  -f2-); then
        tup_debug 3 "debug: key $key not found in $cfg"
    fi
}

# remove value from key
tup_conf_remove() {
    local cfg=$1
    local key=$2
    shift; shift
    local arr=($(tup_conf_get $cfg $key))
    for k in $@; do
        for i in ${!arr[*]}; do
            if [[ "${arr[$i]}" = "$k" ]]; then
                unset arr[$i]
            fi
        done
    done
    tup_conf_set "$cfg" $key ${arr[*]}
}

# list matching sub keys and values
tup_conf_search() {
    local cfg=$1
    local key=$2
    if ! (grep "^$key\." "$cfg"); then
        tup_debug 3 "debug: key $key not found in $cfg"
    fi
}

# set conf key
tup_conf_set() {
    local cfg=$1
    local key=$2
    shift; shift
    if ! (cat "$cfg" | grep -v "^$key " > "$cfg.tmp"); then
        tup_debug 3 "debug: key $key not set in $cfg"
    fi
    echo "$key $@" >> "$cfg.tmp"
    mv "$cfg.tmp" "$cfg"
}

# show all keys and values
tup_conf_show() {
    local cfg=$1
    cat "$cfg"
}

# list sub keys
tup_conf_subkeys() {
    local cfg=$1
    local key=$2
    grep "^$key\." "$cfg" | cut -d\  -f1 | cut -b$(echo -n $key.. | wc -c)- | cut -d. -f1 | uniq
}

# unset conf key
tup_conf_unset() {
    local cfg=$1
    local key=$2
    if ! (cat "$cfg" | grep -v "^$key " > "$cfg.tmp"); then
        tup_debug 3 "debug: key $key not found in $cfg"
    fi
    mv "$cfg.tmp" "$cfg"
}


#
# debugging and error reporting
#

# print debug messages
tup_debug() {
    if [[ ! -z "$TUP_DEBUG" && $1 -le $TUP_DEBUG ]]; then
        shift
        echo "debug: $@" >&2
    fi
}

# trap SIGERR
tup_error() {
    echo "error: caught error while running: $BASH_COMMAND" >&2
    exit 4
}


#
# object tests
#

# check if layer exists
tup_object_exists() {
    if [[ -d "$TUP_STORAGE_DIR/${object}s/$(tup_path $2)" ]]; then
        return 0
    else
        return 1
    fi
}


#
# argument parsing
#

# parse argument list
tup_parseargs() {
    while [[ ! -z "$@" ]]; do
        local arg=$1
        if (echo $arg | grep -P '^--[a-z]+' 2>&1 >/dev/null); then
            local name=$(echo $arg | cut -b3- | tr - _)
            tup_debug 4 "tup_parseargs: found arg: $name"
            eval "tuparg_${name}_set=1"
        elif [[ ! -z "$name" ]]; then
            tup_debug 4 "tup_parseargs: found arg value for $name: $arg" 
            eval "tuparg_${name}='$arg'"
            local name=""
        fi
        shift
    done
}

# produce a path from an object ID
tup_path() {
    echo $(echo $1 | cut -b1-2)/$1
}


#
# hashing and checksums
#

# generate a checksum for a directory
tup_sum_dir() {
    (cd $1; tar c * | sha256sum | cut -d\  -f1)
}

# generate a checksum for a file
tup_sum_file() {
    sha256sum "$1" | cut -d\  -f1
}

# generate a checksum for a string
tup_sum_str() {
    echo -n "$@" | sha256sum | cut -d\  -f1
}
