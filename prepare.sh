#!/bin/sh
# $Id$


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
        touch systab.c
        sleep 1
        touch -am systab.in
    fi
}


recur() {
    for entry in *
    do
        if [ -d $entry ]
        then
            # echo "===> $cpath$entry (config)"
            cd $entry
            work
            cpath="$cpath$entry/" recur
            cd ..
        fi
    done
}

cpath="" recur

mkdir -p output/bin output/sbin output/usr/include output/etc output/boot/grub output/lib output/dev
