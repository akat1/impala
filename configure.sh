#!/bin/sh
# $Id$


work() {
    if [ -e Makefile ]
    then
        touch .depend
    fi
}


recur() {
    for entry in *
    do
        if [ -d $entry ]
        then
            echo "===> $cpath$entry (config)"
            cd $entry
            work
            cpath="$cpath$entry/" recur
            cd ..
        fi
    done
}

cpath="" recur
