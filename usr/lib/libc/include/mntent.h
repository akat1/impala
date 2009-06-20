#ifndef __MNTENT_H__
#define __MNTENT_H__

// chwilowy zrzut z glibc
// chcemy mieæ linuksowy interfejs do tego, czy jaki¶ inny / w³asny?

struct mntent {
    char *mnt_fsname;
    char *mnt_dir;
    char *mnt_type;
    char *mnt_opts;
//    int mnt_freq;
//    int mnt_passno;
};


FILE *setmntent (const char *file, const char *mode);

struct mntent *getmntent (FILE *stream);


#endif