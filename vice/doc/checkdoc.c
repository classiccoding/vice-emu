
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ITEM
{
    char *string;
    struct _ITEM *next;
    int flags;
} ITEM;

ITEM *list_findstr(ITEM *list, char *str)
{
    ITEM *itm = list;
    while (itm) {
//        printf(">%s|%s\n",itm->string, str);
        if (itm->string) {
            if (!strcasecmp(itm->string, str)) {
                return itm;
            }
        }
        itm = itm->next;
    }
    return NULL;
}

ITEM *list_addstr(ITEM *list, char *str)
{
    ITEM *itm;

    if ((str == NULL) || (*str == 0)) {
        return list;
    }

    itm = list_findstr(list, str);
    if (itm) {
        return itm;
    }
    itm = malloc(sizeof(ITEM));
    itm->flags=0;
    itm->string = strdup(str);
    itm->next = list->next;
    list->next = itm;
    return itm;
}

int skipblank(FILE *f)
{
    int c;
    while(!feof(f)) {
        c = fgetc(f);
        if (c == EOF) {
            return 0;
        }
        if ((c != ' ') && (c != '\n') && (c != '\t')) {
            break;
        }
    }
    return c;
}

int skipuntil(FILE *f, int s)
{
    int c;
    while(!feof(f)) {
        c = fgetc(f);
        if (c == s) {
            break;
        }
    }
    return c;
}

int getstr(FILE *f, char *str)
{
    int c;
    while(!feof(f)) {
        c = fgetc(f);
        if ((c == EOF) || (c == ' ') || (c == '\n') || (c == '\t') || (c == ',') || (c == '=')) {
            break;
        }
        *str++ = c;
    }
    *str = 0;
    return c;
}

#define IS_PLUS4        (1<<0)
#define IS_CBM2         (1<<1)
#define IS_B500         (1<<2)
#define IS_PET          (1<<3)
#define IS_VIC20        (1<<4)
#define IS_DTV          (1<<5)
#define IS_C128         (1<<6)
#define IS_VSID         (1<<7)
#define IS_C64          (1<<8)
#define IS_C64SC        (1<<9)

#define IS_CARTCONV     (1<<10)
#define IS_PETCAT       (1<<11)
#define IS_C1541        (1<<12)

const char *emustring[0x10] = {
    "PLUS4",
    "CBM-II",
    "CBM-II-5x0",
    "PET",
    "VIC20",
    "C64DTV",
    "C128",
    "VSID",
    "C64",
    "C64SC",
    "cartconv",
    "petcat",
    "c1541"
};

ITEM reslistrc = { NULL, NULL, 0};
ITEM reslisttex = { NULL, NULL, 0};
ITEM reslisttexitm = { NULL, NULL, 0};
ITEM reslistnew = { NULL, NULL, 0};

ITEM optlistvice = { NULL, NULL, 0};
ITEM optlisttex = { NULL, NULL, 0};
ITEM optlisttex2 = { NULL, NULL, 0};
ITEM optlisttexitm = { NULL, NULL, 0};
ITEM optlistnew = { NULL, NULL, 0};

int readtexi(FILE *tf)
{
    int c,cl = 0;
    char tmp[0x100];
    char *t;

    while(!feof(tf)) {
        c = fgetc(tf);
        if (c == '@') {
            fscanf(tf, "%s ", tmp);
            cl = 0;
            if (!strcmp(tmp, "vindex")) {
                c = getstr(tf, tmp);
//                printf("resource '%s'\n",tmp);
                list_addstr(&reslisttex, tmp);
            } else if (!strcmp(tmp, "cindex")) {
                c = getstr(tf, tmp);
                if ((tmp[0] == '-') || (tmp[0] == '+')) {
                    list_addstr(&optlisttex, tmp);
//                    printf("option '%s' ",tmp);
                    if (c == ',') {
                        tmp[0] = skipblank(tf);
                        c = getstr(tf, &tmp[1]);
                        list_addstr(&optlisttex, tmp);
//                        printf("/ '%s' ",tmp);
                    }
//                    printf("\n");
                } else {
                    list_addstr(&optlisttex2, tmp);
                }
            } else if (!strcmp(tmp, "item")) {
                c = getstr(tf, tmp);
                if ((tmp[0] == '-') || (tmp[0] == '+')) {
                    list_addstr(&optlisttexitm, tmp);
                } else {
                    list_addstr(&reslisttexitm, tmp);
                }
            } else if (!strcmp(tmp, "itemx")) {
                c = getstr(tf, tmp);
                if ((tmp[0] == '-') || (tmp[0] == '+')) {
                    list_addstr(&optlisttexitm, tmp);
                } else {
                    list_addstr(&reslisttexitm, tmp);
                }
            } else if (!strcmp(tmp, "c")) {
                skipuntil(tf, '\n');
            }
        }
    }
}

int readvicerc(FILE *tf, char *emu, int tag)
{
    char tmp[0x100];
    int c;
    ITEM *itm;
    fseek(tf,0,SEEK_SET);
    while(!feof(tf)) {
        skipuntil(tf, '[');
        c = fscanf(tf,"%s",tmp);
        if (c < 1) {
            break;
        }
        tmp[strlen(tmp)-1]=0;
//        printf("tag %d '%s'\n",c,tmp);
        if (!strcmp(emu, tmp)) {
            break;
        }
    }

    while(!feof(tf)) {
        c = skipblank(tf);
        if (c == '[') {
            break;
        }
        tmp[0] = c;
        c = getstr(tf, &tmp[1]);
//        printf("resource '%s'\n",tmp);
        itm = list_addstr(&reslistrc, tmp);
        itm->flags |= tag;
        skipuntil(tf, '\n');
    }

}

int readviceopt(FILE *tf, char *emu, int tag)
{
    char tmp[0x100];
    int c;
    ITEM *itm;
//    printf("reading opts for '%s'\n",emu);
    fseek(tf,0,SEEK_SET);
    while(!feof(tf)) {
        skipuntil(tf, '[');
        c = fscanf(tf,"%s",tmp);
        if (c < 1) {
            break;
        }
        tmp[strlen(tmp)-1]=0;
//        printf("tag %d '%s'\n",c,tmp);
        if (!strcasecmp(emu, tmp)) {
            break;
        }
    }

    while(!feof(tf)) {
        c = skipblank(tf);
        if (c == '[') {
            break;
        }
//            printf("option '%c'\n",c);

        if ((c=='-')||(c=='+')) {
            tmp[0] = c;
            c = getstr(tf, &tmp[1]);

//            printf("option '%s'\n",tmp);
            itm = list_addstr(&optlistvice, tmp);
            itm->flags |= tag;
        } else {
            c = getstr(tf, &tmp[0]);
//            printf("not option '%s'\n",tmp);
        }
        skipuntil(tf, '\n');
    }

}

void printlist(ITEM *list, char *hdr, int flags)
{
    int i = 0,ii;
    while (list) {
//                    printf("1'%s'\n", list->string);
        if (list->string) {
            if (flags) {
//                  printf("2'%s'\n", list->string);
                if (list->flags == flags) {
                    if (i == 0) {
                        printf("\n[%s]\n\n", hdr);i++;
                    }
                    printf("%s\n", list->string);
                }
            } else {
                if (1
                    && (list->flags != IS_C64SC)
                    && (list->flags != IS_C64)
                    && (list->flags != IS_VSID)
                    && (list->flags != IS_C128)
                    && (list->flags != IS_DTV)
                    && (list->flags != IS_VIC20)
                    && (list->flags != IS_PET)
                    && (list->flags != IS_B500)
                    && (list->flags != IS_PLUS4)
                    && (list->flags != IS_CBM2)
                    && (list->flags != IS_CARTCONV)
                    && (list->flags != IS_PETCAT)
                    && (list->flags != IS_C1541)
                    ){
                    if (i == 0) {
                        printf("\n%s\n\n", hdr);i++;
                    }
                    printf("%-40s", list->string);
                    for (i=0;i<10;i++) {
                        if (list->flags & (1<<i)) {
                            printf("%s  ", emustring[i]);
                        } else {
                            for (ii=0;ii<(strlen(emustring[i])+2);ii++) {
                                printf(" ");
                            }
                        }
                    }
                    printf("\n");
                }
            }
        }
        list = list->next;
    }
}

void checkresources(void)
{
    ITEM *list1, *itm, *itm2;
    int i;

    printf("\nchecking resources...\n\n");

    printf("\nThe following resources are incorrectly marked '@cindex'\n"
           "fix them first to use '@vindex' and then check again:\n\n");

    list1 = &reslistrc;
    while (list1) {
//        printf("check: %s\n", list1->string);
        if (list1->string) {
            itm = list_findstr(&optlisttex2, list1->string);
            if (itm) {
                printf("%s\n", list1->string);
            }
        }
        list1 = list1->next;
    }

    printf("\nThe following resources do not appear in '@vindex'.\n"
           "fix them first to use '@vindex' and then check again:\n\n");
    printf("note: each resource should get a seperate '@vindex' entry\n"
           "      in the form '@vindex resourcename'.\n\n");

    list1 = &reslisttexitm;
    i = 0;
    while (list1) {
//        printf("check: %s\n", list1->string);
        if (list1->string) {
            itm = list_findstr(&reslistrc, list1->string);
            if (itm) {
                itm = list_findstr(&reslisttex, list1->string);
                if (!itm) {
                    printf("%s\n", list1->string);
                    i++;
                }
            }
        }
        list1 = list1->next;
    }
    if (i == 0) {
        printf("none - well done.\n");
    }

    list1 = &reslistrc;
    while (list1) {
//        printf("check: %s\n", list1->string);
        if (list1->string) {
            itm = list_findstr(&reslisttex, list1->string);
            if (itm) {
//                printf("found resource: %s\n", list1->string);
            } else {
//                printf("'%s'\n", list1->string);
                itm2=list_addstr(&reslistnew, list1->string);
//                printf("'%s'\n", itm2->string);
                itm2->flags=list1->flags;
            }
        }
        list1 = list1->next;
    }
    printf("\n\nThe following resources appear in vicerc but not in the documentation, so\n"
           "they might be missing in the documentation:\n\n");
    printlist(&reslistnew, "global", 0);
    printlist(&reslistnew, "C64SC", IS_C64SC);
    printlist(&reslistnew, "C64", IS_C64);
    printlist(&reslistnew, "VSID", IS_VSID);
    printlist(&reslistnew, "C128", IS_C128);
    printlist(&reslistnew, "C64DTV", IS_DTV);
    printlist(&reslistnew, "VIC20", IS_VIC20);
    printlist(&reslistnew, "PET", IS_PET);
    printlist(&reslistnew, "CBM-II-5x0", IS_B500);
    printlist(&reslistnew, "CBM-II", IS_CBM2);
    printlist(&reslistnew, "PLUS4", IS_PLUS4);

    printf("\nThe following resources appear in the documentation but not in vicerc, so\n"
           "they might be outdated or spelled incorrectly:\n\n");

    list1 = &reslisttex;

    while (list1) {
//        printf("check: %s\n", list1->string);
        if (list1->string) {
            itm = list_findstr(&reslistrc, list1->string);
            if (itm) {
//                printf("found: %s\n", list1->string);
            } else {
                printf("%s\n", list1->string);
            }
        }
        list1 = list1->next;
    }
}

void checkoptions(void)
{
    ITEM *list1, *itm, *itm2;
    int i;

    printf("\nchecking command line options...\n\n");

    printf("\nThe following look like options, but they do not appear in '@cindex'.\n"
           "fix them first to use '@cindex' and then check again:\n\n");

    list1 = &optlisttexitm;
    i = 0;
    while (list1) {
//        printf("check: %s\n", list1->string);
        if (list1->string) {
            itm = list_findstr(&optlisttex, list1->string);
            if (!itm) {
                if (strcmp(list1->string,"--") != 0) {
                    printf("%s\n", list1->string);
                    i++;
                }
            }
        }
        list1 = list1->next;
    }
    if (i == 0) {
        printf("none - well done.\n");
    }

    list1 = &optlistvice;
    i = 0;
    while (list1) {
//        printf("check: %s\n", list1->string);
        if (list1->string) {
            itm = list_findstr(&optlisttex, list1->string);
            if (itm) {
//                printf("found option: %s\n", list1->string);
            } else {
//                printf("'%s'\n", list1->string);
                itm2=list_addstr(&optlistnew, list1->string);
//                printf("'%s'\n", itm2->string);
                itm2->flags=list1->flags;
                i++;
            }
        }
        list1 = list1->next;
    }
    printf("\n\nThe following options appear in vice but not in the documentation, so\n"
           "they might be missing in the documentation (%d):\n\n", i);
    printlist(&optlistnew, "global", 0);
    printlist(&optlistnew, "C64SC", IS_C64SC);
    printlist(&optlistnew, "C64", IS_C64);
    printlist(&optlistnew, "VSID", IS_VSID);
    printlist(&optlistnew, "C128", IS_C128);
    printlist(&optlistnew, "C64DTV", IS_DTV);
    printlist(&optlistnew, "VIC20", IS_VIC20);
    printlist(&optlistnew, "PET", IS_PET);
    printlist(&optlistnew, "CBM-II-5x0", IS_B500);
    printlist(&optlistnew, "CBM-II", IS_CBM2);
    printlist(&optlistnew, "PLUS4", IS_PLUS4);

    printlist(&optlistnew, "petcat", IS_PETCAT);
    printlist(&optlistnew, "cartconv", IS_CARTCONV);
    printlist(&optlistnew, "c1541", IS_C1541);

    printf("\n\nThe following options appear in the documentation but not in vice, so\n"
           "they might be outdated or spelled incorrectly:\n\n");

    list1 = &optlisttex;
    i = 0;
    while (list1) {
//        printf("check: %s\n", list1->string);
        if (list1->string) {
            itm = list_findstr(&optlistvice, list1->string);
            if (itm) {
//                printf("found: %s\n", list1->string);
            } else {
                printf("%-40s", list1->string);
                if(0
                    || !strcmp(list1->string, "-xsync")
                    || !strcmp(list1->string, "+xsync")
                    || !strcmp(list1->string, "-mitshm")
                    || !strcmp(list1->string, "+mitshm")
                  ) {
                    printf("(might be disabled)");
                } else {
                    i++;
                }
                printf("\n");
            }
        }
        list1 = list1->next;
    }
    printf("\n");

    if (i == 0) {
        printf("none - well done.\n");
    }
}

char *vicercname;
char *viceoptname;
char *vicetexiname;
int checkopt = 0;
int checkres = 0;

int main(int argc, char *argv[])
{
FILE *tf;

    if (argc != 5) {
        printf("usage: checkdoc [-all | -opt | -res] texifile vicerc optsfile\n");
        exit(-1);
    }

    if (!strcmp(argv[1],"-all")) {
        checkopt++;
        checkres++;
    }
    if (!strcmp(argv[1],"-opt")) {
        checkopt++;
    }
    if (!strcmp(argv[1],"-res")) {
        checkres++;
    }

    vicetexiname = argv[2];
    vicercname = argv[3];
    viceoptname = argv[4];

    tf = fopen(vicetexiname,"rb");
    if (!tf) {
        fprintf(stderr, "error: couldn't open %s.\n", vicetexiname);
        exit(-1);
    }
    printf("reading %s.\n", vicetexiname);
    readtexi(tf);
    fclose(tf);

    tf = fopen(vicercname,"rb");
    if (!tf) {
        fprintf(stderr, "error: couldn't open %s.\n", vicercname);
        exit(-1);
    }
    printf("reading %s.\n", vicercname);
    readvicerc(tf,"PLUS4", IS_PLUS4);
    readvicerc(tf,"CBM-II", IS_CBM2);
    readvicerc(tf,"CBM-II-5x0", IS_B500);
    readvicerc(tf,"PET", IS_PET);
    readvicerc(tf,"VIC20", IS_VIC20);
    readvicerc(tf,"C64DTV", IS_DTV);
    readvicerc(tf,"C128", IS_C128);
    readvicerc(tf,"VSID", IS_VSID);
    readvicerc(tf,"C64", IS_C64);
    readvicerc(tf,"C64SC", IS_C64SC);
    fclose(tf);

    tf = fopen(viceoptname,"rb");
    if (!tf) {
        fprintf(stderr, "error: couldn't open %s.\n", viceoptname);
        exit(-1);
    }
    printf("reading %s.\n", viceoptname);
    readviceopt(tf,"PLUS4", IS_PLUS4);
    readviceopt(tf,"CBM-II", IS_CBM2);
    readviceopt(tf,"CBM-II-5x0", IS_B500);
    readviceopt(tf,"PET", IS_PET);
    readviceopt(tf,"VIC20", IS_VIC20);
    readviceopt(tf,"C64DTV", IS_DTV);
    readviceopt(tf,"C128", IS_C128);
    readviceopt(tf,"VSID", IS_VSID);
    readviceopt(tf,"C64", IS_C64);
    readviceopt(tf,"C64SC", IS_C64SC);
    readviceopt(tf,"petcat", IS_PETCAT);
    readviceopt(tf,"cartconv", IS_CARTCONV);
    readviceopt(tf,"c1541", IS_C1541);
    fclose(tf);

    if (checkres) {
        checkresources();
    }
    if (checkopt) {
        checkoptions();
    }
    return 0;
}
