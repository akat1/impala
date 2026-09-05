export IMPALA_ARCH=${IMPALA_ARCH:="amd64"}
export IMPALA_SRCROOT=$PWD
export IMPALA_MK=$IMPALA_SRCROOT/mk
export IMPALA_OUTPUT=$IMPALA_SRCROOT/output
export IMPALA_SDK=${HOME}/ImpalaSDK

if [ -z "${ELF_PREFIX+x}" ]; then
    if [ -x "${IMPALA_SDK}/bin/x86_64-elf-gcc" ]; then
        export ELF_PREFIX="${IMPALA_SDK}/bin/x86_64-elf-"
    else
        export ELF_PREFIX="x86_64-linux-gnu-"
    fi
fi

if [ -z "${AOUT_PREFIX+x}" ]; then
    if [ "${IMPALA_ARCH}" = "amd64" ]; then
        export AOUT_PREFIX="${ELF_PREFIX}"
    else
        export AOUT_PREFIX="${IMPALA_SDK}/bin/i386-aout-"
    fi
fi
