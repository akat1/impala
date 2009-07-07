#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <term.h>
#include <sys/ascii.h>
#include <sys/termios.h>

#define ERR 0
#define TC_ITEMS_MAX 256

static char *_termdesc[TC_ITEMS_MAX] = {NULL, };
static void _tgetent(char *data);

void
_tgetent(char *data)
{
    // wyczyscic aktualne..
    for(int i=0; i<TC_ITEMS_MAX; i++) {
        char *X = _termdesc[i];
        _termdesc[i] = NULL;
        if(X)
            free(X);
    }
    // i ladujemy nowe
    char *c = data;
    if(!*c || *c == '\n')
        return;
    char buf[128];
    char *c2 = buf;
    bool newLine = FALSE;
    int nextCap = 0;
    while(c && *c) {
        while(isspace(*c))
            c++;
        if(*c == '.') {
            c = strchr(c, ':');
            if(!c)
                return;
            c++;
        }
        // to mamy nasze cosio
        // moga byc tu jeszcze bledy odnosnie traktowania nowej linii
        while(*c && *c!=':' && *c!='\n') {
            switch(*c) {
                case '#':
                    if(newLine) {
                        c = strchr(c, '\n');
                        if(c)
                            c++;
                    } else
                        *(c2++) = *(c++);
                    break;
                case '\\':
                    c++;
                    newLine = FALSE;
                    switch(*c) {
                        case '\\':
                        case '^':
                        case ':':   *(c2++) = *c;          break;
                        case 'E':   *(c2++) = ESC;         break;
                        case 'n':   *(c2++) = NL;          break;
                        case 'r':   *(c2++) = CR;          break;
                        case 't':   *(c2++) = HT;          break;
                        case 'b':   *(c2++) = BS;          break;
                        case 'f':   *(c2++) = FF;          break;
                        case '\n':  newLine = TRUE;     break;
                        default: {
                            if(isdigit(*c)) { //octal
                                unsigned char newc = 0;
                                for(int i=0; i<3; i++)
                                    newc = newc*8 + *(c++) - '0';
                                *(c2++) = newc;
                                c--;
                            }
                        }
                    }
                    c++;
                    break;
                case '^':
                    newLine = FALSE;
                    c++;
                    *(c2++) = CTRL(*c);
                    c++;
                    break;
                default:
                    newLine = FALSE;
                    *(c2++) = *(c++);
            }
        }
        if(*c)
            c++;
        *c2 = '\0';
        if(c2!=buf) {
            _termdesc[nextCap++] = strdup(buf);
            c2 = buf;
        }
    }

    //PC, UP, BC
    BC = "\b";
    
}

int
tgetent(char *bp, const char *name)
{
    char *tinfo = strdup(getenv("TERMCAP"));
    char *c = tinfo;
    while(*c) {
        //zjedzmy puste znaki
        while(isspace(*c))
            c++;
        if(*c == '#') { //komentarz... wezmy kolejna linie
            c = strchr(c, '\n');
            if(!c) {
                free(tinfo);
                return 0;   //not found
            }
            c++;
            continue;
        }
        //pierwsze pole - nazwy
        char *header = strsep(&c, ":");
        while(header!=NULL) {
            char *tname = strsep(&header, "|");
//            printf("Name in db: \'%s\', looking for \'%s\'\n", tname, name);
            if(!strcmp(name, tname)) { //trafilismy
                _tgetent(c);
                free(tinfo);
//                printf("Found!\n");
                return 1;
            }
        }
        //to nie ten rekord -> przewijamy
        while(c!=NULL) {
            char *newc = strchr(c, '\n');
            if(!newc) {
                free(tinfo);
                return 0;
            }
            c = newc;
            if(c>tinfo && c[-1] == '\\') //escaped \n
                continue;
            break;
        }
    }
    free(tinfo);
    return 0;
}

int
tgetflag(char id[2])
{
    char **tc = _termdesc;
    while(*tc) {
        if((*tc)[0] == id[0] &&
           (*tc)[1] == id[1])
            return 1;
        tc++;
    }
    return 0;
}

int
tgetnum(char id[2])
{
    char **tc = _termdesc;
    while(*tc) {
        if((*tc)[0] == id[0] &&
           (*tc)[1] == id[1] &&
           (*tc)[2] == '#') 
            return atoi(&(*tc)[3]);
        tc++;
    }
    return -1;
}

char *
tgetstr(char id[2], char **area)
{
    char **tc = _termdesc;
    while(*tc) {
        if((*tc)[0] == id[0] &&
           (*tc)[1] == id[1] &&
            (*tc)[2] == '=') {
            char *res = &(*tc)[3];
            strcpy(*area, res);
            res = *area;
            *area += strlen(res)+1;
            return res;
        }
        tc++;
    }
    return NULL;
}

char *
tgoto(char *cap, int col, int row)
{
    static char res_buf[128];
    int args[2] = {col, row};
    char *c2 = res_buf;
    char *c = cap;
    int argNum = 1;
    while(*c) {
        switch(*c) {
            case '%':
                c++;
                switch(*c) {
                    case 'r': argNum++; argNum%=2;  break;
                    case '%': *(c2++) = '%';        break;
                    case 'i': args[0]++; args[1]++; break; //napewno oba?
                    case '+': {
                        char cc = *(++c);
                        args[0]+=cc; args[1]+=cc;
                        ///@todo jeszcze % robimy?
                    } break;
                    case '>': {
                        char cc = *(++c);
                        char dd = *(++c);
                        if(args[0] > cc)
                            args[0] += dd;
                        if(args[1] > cc)
                            args[1] += dd;
                    } break;
                    case 'd':
                    case '3':
                    case '2': {
                        int len = 1;
                        if(isdigit(*c))
                            len = *c - '0';
                        char buf[16];
                        int x = 15;
                        buf[x--] = '\0';
                        buf[x-1] = '0';
                        int arg = args[argNum++];
                        argNum %= 2;
                        while(arg>0) {
                            buf[x--] = '0' + arg % 10;
                            arg /= 10;
                        }
                        while(15-x < len)
                            buf[x--] = '0';
                        x++;
                        while(x<15)
                            *(c2++) = buf[x++];
                        break;
                    }
                    case '.': {
                        int arg = args[argNum++];
                        argNum %= 2;
                        *(c2++) = arg;
                    } break;
                };
                c++;
                break;
            default:
                *(c2++) = *(c++);
        }
    }
    *c2 = '\0';
    return res_buf;
}

int tputs(const char *str, int affcnt, int (*putfunc)(int))
{
    int delay = 0;
    while(isdigit(*str)) {
        delay = 10*delay + *(str++) - '0';
    }
    if(*str == '.') { //zawsze moze byc czy tylko z *?
        str++;
        delay = 10*delay + *(str++) - '0';
    } else
        delay *= 10;
    //delay w 0.1ms
    bool delay_per_line = FALSE;
    if(*str == '*') {
        str++;
        delay_per_line = TRUE;
    }
    for(int i=0; str[i]; i++)
        putfunc(str[i]&0x7f);
    ///@todo waitns(100*delay* (delay_per_line)?affcnt:1);
    return 0;
}
