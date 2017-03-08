prepare_dirs() {
    WORKDIR=distribution/${DIST_PROFILE}
    SYSPACK_DIR=${WORKDIR}/syspack
    IMAGE_DIR=${WORKDIR}/image
    rm -rf ${WORKDIR}
    mkdir -p ${SYSPACK_DIR}
    mkdir -p ${SYSPACK_DIR}/bin
    mkdir -p ${SYSPACK_DIR}/sbin
    mkdir -p ${SYSPACK_DIR}/demos
    mkdir -p ${SYSPACK_DIR}/tmp
    mkdir -p ${SYSPACK_DIR}/var/tmp
    mkdir -p ${IMAGE_DIR}
    mkdir -p ${IMAGE_DIR}/boot
    mkdir -p ${IMAGE_DIR}/boot/grub
    mkdir -p ${IMAGE_DIR}/impala
}

check_profile() {
    if [ ! -r ${PROFILE_FILE} ]; then
        echo "Invalid profile ${DIST_PROFILE}"
        exit 1
    fi
}

prepare_root() {
    cp image/_floppy.img ${WORKDIR}/floppy.img
    cp COPYRIGHT ${IMAGE_DIR}
    cp sys/kern/impala.gz ${IMAGE_DIR}/boot/
    cp image/root/boot/grub/menu.lst ${IMAGE_DIR}/boot/grub
    cp image/root/boot/grub/grub.cfg ${IMAGE_DIR}/boot/grub
    cp image/root/boot/grub2.img ${IMAGE_DIR}/boot/
    mv ${WORKDIR}/syspack.tar.gz ${IMAGE_DIR}/impala/
    cp usr/sbin/preinit/preinit ${IMAGE_DIR}/impala/
    cp usr/bin/tar/tar ${IMAGE_DIR}/impala/
    cp usr/bin/minigzip/minigzip ${IMAGE_DIR}/impala/

}

prepare_syspack() {
    cp -r usr/etc ${SYSPACK_DIR}
    for entry in `cat ${PROFILE_FILE}`; do
         file=`echo ${entry} | sed -e 's/:.*/'/`
         exe_name=`echo ${entry} | sed -e 's/.*://'`
         if [ -z ${exe_name} ]; then
            exe_name=${file}/`basename ${file}`
         fi
         echo "install ${exe_name} as ${file}"
         cp ./usr/${exe_name} ${SYSPACK_DIR}/${file}
    done
    (cd ${SYSPACK_DIR}; tar --format=ustar -zcf ../syspack.tar.gz .)
}

build_image() {
    mcopy -s -i ../floppy.img impala ::/impala
    mcopy -s -i ../floppy.img boot/impala.gz ::/boot/
    mcopy -s -i ../floppy.img boot/grub/menu.lst ::/boot/grub/
    for f in `find . -maxdepth 1 -type f`; do
        if [ $f != "." ]; then
            mcopy -i ../floppy.img $f ::$f
        fi
    done
}

handle_profile() {
    echo "====> Building floppy image for profile ${DIST_PROFILE}"

    prepare_dirs
    prepare_syspack
    prepare_root
    (cd ${IMAGE_DIR}; build_image)
}

if [ -z ${DIST_PROFILE} ]; then
    for PROFILE_FILE in tools/distribution/profiles/*; do
        DIST_PROFILE=`basename ${PROFILE_FILE}`
        handle_profile 
    done
else
    PROFILE_FILE="tools/distribution/profiles/${DIST_PROFILE}"
    handle_profile 
fi

check_profile
