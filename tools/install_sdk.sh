#!/bin/sh

# wget http://ftp.gnu.org/gnu/gcc/gcc-3.4.0/gcc-3.4.0.tar.gz

BINUTILS_VERSION="2.23"
GCC_VERSION="4.2.0"
SDK_PATH=${HOME}/ImpalaSDK


print_msg () {
    echo "[33;7;1m$@[0m"
}

print_section () {
    print_msg "====> " $@
}

prepare_binutils_src () {
    print_section "Downloading binutils-${BINUTILS_VERSION} sources"
    wget -c http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz
    echo -n "Extracting..."
    rm -rf binutils-${BINUTILS_VERSION}
    tar zxf binutils-${BINUTILS_VERSION}.tar.gz
    echo "DONE"
}

prepare_gcc_src () {
    print_section "Downloading gcc-core-${GCC_VERSION} sources"
    wget -c http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-core-${GCC_VERSION}.tar.bz2
    echo -n "Extracting..."
    rm -rf gcc-${GCC_VERRSION}
    tar jxf gcc-core-${GCC_VERSION}.tar.bz2
    echo "DONE"
}

in_binutils_src () {
    cd binutils-${BINUTILS_VERSION}
    eval $@
    cd ..
}

in_gcc_obj () {
    objdir=gccobjdir_$1
    shift
    rm -rf ${objdir}
    mkdir -p ${objdir}
    cd ${objdir}
    eval $@
    cd ..
}

binutils_build_command () {
    ./configure --prefix=${SDK_PATH} --target=$1
    (make && make install)
    if [ ! $? -eq 0 ]; then
        exit 1
    fi
    make distclean
}

gcc_build_command () {
    export PATH=${SDK_PATH}/bin:${PATH}
    ../gcc-${GCC_VERSION}/configure --prefix=${SDK_PATH} --target=$1 --enable-languages=c --disable-libssp --disable-threads --disable-tls  --disable-quadmath --disable-libgomp
    (make && make install)
    if [ ! $? -eq 0 ]; then
        exit 1
    fi
    make distclean
}

build_binutils_for_target ()
{
    target=$1
    print_section "Building and installing binutils-${BINUTILS_VERSION} for ${target}"
    in_binutils_src binutils_build_command "${target}" > output.out 2> output.err
}

build_gcc_for_target ()
{
    target=$1
    print_section "Building and installing gcc-core-${GCC_VERSION} for ${target}"
    in_gcc_obj "${target}" gcc_build_command "${target}" #> output.out 2> output.err
}

build_binutils () {
    build_binutils_for_target "i386-aout"
    build_binutils_for_target "i386-elf"
}

build_gcc () {
    build_gcc_for_target "i386-aout"
    build_gcc_for_target "i386-elf"
}

print_msg "#### ImpalaSDK"


echo Output of build commands are redirected to output.out and output.err files.
echo Desired binutils version is ${BINUTILS_VERSION}.
echo Desired gcc version is ${GCC_VERSION}.
echo Installation path is ${SDK_PATH}.

#prepare_binutils_src
prepare_gcc_src

#build_binutils
build_gcc

print_msg "Done, go and replace your operating system by Impala"
