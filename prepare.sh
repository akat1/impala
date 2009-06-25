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
	./gensystab.awk < systab.in
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

mkdir -p output/dist/bin output/dist/sbin output/dist/etc output/impala
