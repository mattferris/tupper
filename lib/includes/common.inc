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
        tup_debug 3 "key $key not found in $cfg"
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
        tup_debug 3 "key $key not found in $cfg"
    fi
}

# set conf key
tup_conf_set() {
    local cfg=$1
    local key=$2
    shift; shift
    if ! (cat "$cfg" | grep -v "^$key " > "$cfg.tmp"); then
        tup_debug 3 "key $key not set in $cfg"
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
        tup_debug 3 "key $key not found in $cfg"
    fi
    mv "$cfg.tmp" "$cfg"
}

# load user conf
tup_conf_userload() {
    local user=$1
    local userconf="~/$user/.tupper/conf"
    local userconf=$(eval echo ${userconf//>})
    if [[ -f "$userconf" ]]; then
        TUP_USER_NAME=$(tup_conf get path "$userconf" name)
        TUP_USER_EMAIL=$(tup_conf get path "$userconf" email)
    fi
    if [[ -z $TUP_USER_NAME ]]; then
        TUP_USER_NAME=$(whoami)
    fi
    if [[ -z $TUP_USER_EMAIL ]]; then
        TUP_USER_EMAIL="$TUP_USER_NAME@$(hostname -f 2>/dev/null || hostname -s)"
    fi
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

# print error messages
tup_error() {
    local code=$1
    shift
    echo "error: $@" >&2
    exit $code
}

# trap SIGERR
tup_trap_error() {
    echo "error: caught error while running: $BASH_COMMAND" >&2
    exit 4
}

# print warning messages
tup_warn() {
    local code=$1
    shift
    echo "warning: $@" >&2
    exit $code
}

# print notice messages
tup_notice() {
    echo "notice: $@" >&2
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

# return next parsed argument
tup_getopt_required=()
tup_getopt_optional=()
tup_getopt_matched=()
tup_getopt_args=($@)
tup_getopt_name=""
tup_getopt_value=""
tup_getopt_pos=0
tup_getopt_posargs=()
tup_getopt_return=0
tup_getopt() {

    # bail if no remaining arguments
    if [[ ${#tup_getopt_args[@]} = 0 ]]; then
        tup_debug 4 "tup_getopt: no more arguments"
        tup_getopt_return=1
        return 1
    fi

    # determine next argument
    local cur=${tup_getopt_args[0]}
    if [[ $cur = "--" ]]; then
        tup_debug 4 "tup_getopt: found --, end of arguments"
        tup_getopt_return=1
        return 1
    elif [[ ${cur:0:2} = "--" ]]; then
        tup_getopt_name=${cur#--}
        tup_debug 4 "tup_getopt: found option: $cur"
    else
        local positional=1
        let tup_getopt_pos=++tup_getopt_pos
        tup_getopt_name=$tup_getopt_pos
        tup_getopt_value=$cur
        tup_debug 4 "tup_getopt: found positional argument \$$tup_getopt_pos: $tup_getopt_value"
    fi

    tup_getopt_args=(${tup_getopt_args[@]:1})

    local matched=0
    local optpsec=""

    # have we already matched this argument?
    for i in ${!tup_getopt_matched[@]}; do
        if [[ "${tup_getopt_matched[$i]}" = "$tup_getopt_name" ]]; then
            tup_debug 4 "tup_getopt: duplicate option: $tup_getopt_name"
            tup_getopt_return=3
            return 3
        fi
    done

    # is required?
    for i in ${!tup_getopt_required[@]}; do
        local name=${tup_getopt_required[$i]} 
        if [[ $name = $tup_getopt_name || $name = "${tup_getopt_name}:" ]]; then
            local matched=1
            local optspec=${tup_getopt_required[$i]}
            tup_getopt_matched=(${tup_getopt_matched[@]} $tup_getopt_name)
            unset tup_getopt_required[$i]
            tup_debug 4 "tup_getopt: required option: $tup_getopt_name"
        fi
    done

    # is optional?
    if [[ ! $matched = 1 ]]; then
        for i in ${!tup_getopt_optional[@]}; do
            local name=${tup_getopt_optional[$i]}
            if [[ $name = $tup_getopt_name || $name = "${tup_getopt_name}:" ]]; then
                local matched=1
                local optspec=${tup_getopt_optional[$i]}
                tup_getopt_matched=(${tup_getopt_matched[@]} $tup_getopt_name)
                unset tup_getopt_optional[$i]
                tup_debug 4 "tup_getopt: optional option: $tup_getopt_name"
            elif [[ $positional = 1 && $name = "@" ]]; then
                local matched=1
                tup_getopt_matched=(${tup_getopt_matched[@]} $tup_getopt_name)
                tup_debug 4 "tup_getopt: wildcard positional argument: $tup_getopt_name"
            fi
        done
    fi

    # did we match an undefined option?
    if [[ $matched = 0 ]]; then
        if [[ $positional = 1 ]]; then
            tup_debug 4 "tup_getopt: unexpected positional argument: $tup_getopt_value"
            tup_getopt_return=5
            return 5
        else
            tup_debug 4 "tup_getopt: option is undefined: $tup_getopt_name"
            tup_getopt_return=4
            return 4
        fi
    fi

    # requires an additional argument?
    if [[ "${optspec: -1}" = ":" ]]; then
        tup_debug 4 "tup_getopt: option requires value: $tup_getopt_name"
        if [[ ${#tup_getopt_args[@]} = 0 ]]; then
            tup_debug 4 "tup_getopt: unable to get option value: exhausted argument list"
            tup_getopt_return=2
            return 2
        fi
        tup_getopt_value=${tup_getopt_args[0]}
        tup_getopt_args=(${tup_getopt_args[@]:1})
    fi 

    if [[ $positional = 1 ]]; then
        tup_getopt_posargs=(${tup_getopt_posargs[@]} $tup_getopt_value)
        tup_debug 4 "tup_getopt: saving positional argument $tup_getopt_name: $tup_getopt_value"
    fi

    tup_getopt_return=0
    return 0
}

# handle tup_getopt errors
tup_error_getopt() {
    case $tup_getopt_return in
        0) ;;
        1) ;;
        2) tup_error 1 "missing value for option: --$tup_getopt_name" ;;
        3) tup_error 1 "option already specified: --$tup_getopt_name" ;;
        4) tup_error 2 "invalid option specified: --$tup_getopt_name" ;;
        5) tup_error 2 "unexpected argument: $tup_getopt_value" ;;
        *) tup_error 4 "encountered unknown error while parsing arguments: code $tup_getopt_return" ;;
    esac
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


#
# object listings
#

tup_list_objs() {
    local dir="$TUP_STORAGE_DIR/$1"
    if [[ ! -d "$dir" ]]; then
        tup_error 4 "tup_list_objs: invalid object dir: $dir"
    fi
    find "$dir" -mindepth 2 -maxdepth 2 -exec basename '{}' \;
    return $?
}

tup_list_containers() {
    tup_list_objs containers
    return $?
}

tup_list_images() {
    tup_list_objs images
    return $?
}

tup_list_layers() {
    tup_list_objs layers
    return $?
}

tup_list_volumes() {
    tup_list_objs volumes
}


#
# common formats
#

tup_date() {
    date --date="$1" +"%b %e %Y %R"
}
