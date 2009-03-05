# Wariant konfiguracji srodowiska dla mnie, uzywam platformy
# 64bitowej i musze wybrac nie-systemowe GNU binutils.

# Pawel Wieczorek

export IMPALA_ARCH=x86
export IMPALA_SRCROOT=$PWD
export IMPALA_MK=$IMPALA_SRCROOT/mk
export BINUTILS_PREFIX=/usr/local/cross-i386-elf/bin/i386-elf-

