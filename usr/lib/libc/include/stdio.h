#ifndef __STDIO_H
#define __STDIO_H

#define FILENAME_MAX 4096
#define BUFSIZ 8192

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
#define NULL (void*)0
#define EOF (-1)

struct FILE {
    int fd;
    int status;
    int err;
    char buf[BUFSIZ];
};
typedef struct FILE FILE;

#define _FST_OPEN    1
#define _FST_LINEBUF 2
#define _FST_NOBUF   4

extern FILE _stdF[3];

#define stdin (&_stdF[0])
#define stdout (&_stdF[1])
#define stderr (&_stdF[2])

#include <sys/types.h>
//typedef fpos_t 
//typedef uint32_t size_t;



// void     clearerr(FILE *);
// char    *ctermid(char *);
int      fclose(FILE *);
// FILE    *fdopen(int, const char *);
// int      feof(FILE *);
// int      ferror(FILE *);
int      fflush(FILE *);
int      fgetc(FILE *);
// int      fgetpos(FILE *, fpos_t *);
// char    *fgets(char *, int, FILE *);
int      fileno(FILE *);
// void     flockfile(FILE *);
FILE    *fopen(const char *, const char *);
int      fprintf(FILE *, const char *, ...);
int      fputc(int, FILE *);
int      fputs(const char *, FILE *);
// size_t   fread(void *, size_t, size_t, FILE *);
// FILE    *freopen(const char *, const char *,
//             FILE *);
// int      fscanf(FILE *, const char *, ...);
// int      fseek(FILE *, long, int);
// int      fseeko(FILE *, off_t, int);
// int      fsetpos(FILE *, const fpos_t *);
// long     ftell(FILE *);
// off_t    ftello(FILE *);
// int      ftrylockfile(FILE *);
// void     funlockfile(FILE *);
size_t   fwrite(const void *, size_t, size_t, FILE *);
// int      getc(FILE *);
int      getchar(void);
// int      getc_unlocked(FILE *);
// int      getchar_unlocked(void);
// char    *gets(char *);
// int      pclose(FILE *);
void     perror(const char *);
// FILE    *popen(const char *, const char *);
int      printf(const char *, ...);
int      putc(int, FILE *);
int      putchar(int);
// int      putc_unlocked(int, FILE *);
// int      putchar_unlocked(int);
int      puts(const char *);
// int      remove(const char *);
// int      rename(const char *, const char *);
// void     rewind(FILE *);
// int      scanf(const char *, ...);
// void     setbuf(FILE *, char *);
// int      setvbuf(FILE *, char *, int, size_t);
int      snprintf(char *, size_t, const char *, ...);
int      sprintf(char *, const char *, ...);
int      sscanf(const char *, const char *, ...);
// char    *tempnam(const char *, const char *);
// FILE    *tmpfile(void);
// char    *tmpnam(char *);
// int      ungetc(int, FILE *);
int      vfprintf(FILE *, const char *, va_list);
// int      vfscanf(FILE *, const char *, va_list);
// int      vprintf(const char *, va_list);
// int      vscanf(const char *, va_list);
int      vsnprintf(char *, size_t, const char *, va_list);
// int      vsprintf(char *, const char *, va_list);
int      vsscanf(const char *, const char *, va_list arg);



#endif
