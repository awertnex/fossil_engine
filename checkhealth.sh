#!/usr/bin/env bash

# script to check for text formatting errors in source files, must be run before
# adding a version tag to a commit.

ERR_SUCCESS=1
ERR=$ERR_SUCCESS

TARGETS=(
    src/
    tests/
    build.c
)

DIR_ROOT="$(dirname "$(realpath -e $0)")"
cd $DIR_ROOT

function logerror()
{
    printf "\033[91m"
    printf "[ERROR][$1]: $2"
    printf "\033[0m\n"
}

function logsuccess()
{
    printf "\033[32m"
    printf "[SUCCESS]: $1"
    printf "\033[0m\n"
}

function logtrace()
{
    printf "\033[33m"
    printf "[TRACE]: $1"
    printf "\033[0m\n"
}

function fsl_grep()
{
    local err="$ERR_SUCCESS"
    for i in $2; do
        grep --color -rn "$1" "$i"
        err=$?
        if [[ $err != $ERR_SUCCESS ]]; then
            ERR=$err
        fi
    done
}

# ---- section: check health ------------------------------------------------- #

logtrace "Checking for Tabs.."
fsl_grep $'\t' "$TARGETS"

logtrace "Checking for Trailing Spaces.."
fsl_grep " $" "$TARGETS"

logtrace "Checking for \"e.g.\" Syntax Errors.."
fsl_grep "[^(]e\.g\." "$TARGETS"
fsl_grep "e\.g\.[^,]" "$TARGETS"

# ---- section: log health --------------------------------------------------- #

if [[ $ERR == $ERR_SUCCESS ]]; then
    logsuccess "Source Healthy!"
else
    logerror $ERR "Please Fix!"
fi
