#!/bin/sh
CONVERT="iconv -f ISO-8859-2 -t UTF-8"

# maybe it could be easier to use pattern matching instead of find

convertFile() {
    local file
    f=$1
#    echo "===> $f"
    cp $f $f.latin2
    $CONVERT $f.latin2 > $f
    if [ $? != 0 ]
    then
        echo "ABORT $PWD  $f"
        exit -1
    fi
}

convertFiles() {
    for f in `find . -maxdepth 1  -and \( -name "*.c" -or -name "*.h" \)`
    do
        convertFile $f
    done
}

recur() {
    local dir
    dir=$1
    convertFiles
    for d in `find . -maxdepth 1 -and -type d`
    do
        case $d in
            .)
                ;;
            ..)
                ;;
            ./.svn)
                ;;
            *)
                cd $d
                recur "$dir/$d"
                cd ..
                ;;
        esac
    done
}

recur .
