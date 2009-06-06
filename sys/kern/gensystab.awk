#!/usr/bin/awk -f
# $Id$

function printHc(file) {
    print "/* PLIK GENEROWANY AUTOMATYCZNIE, WSZELKIE ZMIANY BÊD¡ NADPISANE! */" > file;
    print "/* Generator: gensystab.awk */" >> file;
    print "" >> file;
}

function printHhash(file) {
    print "# PLIK GENEROWANY AUTOMATYCZNIE, WSZELKIE ZMIANY BÊD¡ NADPISANE!" > file
    print "# Generator: gensystab.awk" >> file;
    print "" >> file;
}

# Je¿eli AWK dzia³a jak PERL, tzn ¿e arugmenty s± widziane jak tablica to mo¿na chyba
# tê konstrukcjê ulepszyæ -- wieczyk.
function fprint(file, msg, a0, a1, a2, a3 ) {
    printf (msg,a0,a1,a2,a3) >> file;
}

BEGIN {
    n = 0;
    systab_c = "systab.c";
    systab_h = "../sys/systab.h";
    systab_m = "Makefile.sc";
    printHc(systab_c);
    printHc(systab_h);
    printHhash(systab_m);
}

/^#/ {
}

/^SYS / {
    systab[n++] = $2;
}

END {
    fprint(systab_h, "#ifndef __SYS_SYSTAB_H\n");
    fprint(systab_h, "#define __SYS_SYSTAB_H\n\n");
    fprint(systab_h, "enum {\n");
    fprint(systab_c, "#include <sys/types.h>\n\n");
    fprint(systab_m, "SC_SRCS= \\\n");
    for (i = 0; i < n; i++) {
        fprint(systab_h, "    SYS_%s,\n", systab[i]);
        fprint(systab_c, "sc_handler_f sc_%s;\n", systab[i]);
        fprint(systab_m, "\tsc/%s.c\\\n", systab[i]);
    }
    fprint(systab_h, "    SYS_MAX\n};\n");
    fprint(systab_h, "\n#endif /* __SYS_SYSTAB_H */\n");
    fprint(systab_c, "\n");
    fprint(systab_m, "\n");
    fprint(systab_c, "static sc_handler_f *syscall_table[] = {\n");
    for(i = 0; i < n; i++) {
        fprint(systab_c, "    sc_%s,\n", systab[i]);
    }
    fprint(systab_c, "    NULL\n};\n");
}


