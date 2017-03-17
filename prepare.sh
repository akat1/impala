#!/bin/sh

work() {
    if [ -e Makefile ]
    then
        touch .depend
    fi
    if [ -e syscall.h ]
    then
        touch systab.h
    fi
    if [ -e gensystab.awk ]
    then
        awk -f gensystab.awk < systab.in
        touch -am systab.in
    fi
}

recur() {
    for entry in *
    do
        if [ -d $entry ]
        then
            echo "===> $cpath$entry (config)"
            cd $entry
            if [ $? != 0 ]
            then
                echo "-!- can't cd to $cpath$entry"
                continue
            fi
            work
            cpath="$cpath$entry/" recur
            cd ..
        fi
    done
}

cpath="" recur
