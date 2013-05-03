
create_directory() {
    rm -rf distribution/${DIST_PROFILE}
    mkdir -p distribution/${DIST_PROFILE}/sysroot
    mkdir -p distribution/${DIST_PROFILE}/image
}

check_profile() {
    if [ ! -r ${PROFILE_FILE} ]; then
        echo "Invalid profile ${DIST_PROFILE}"
        exit 1
    fi
}

handle_profile() {
    echo "====> Building floppy image for profile ${DIST_PROFILE}"

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
