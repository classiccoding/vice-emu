/*
 * po2table - Automatic translation system helper tool, this program will
 *            convert a po file to a table that will be included in the
 *            gen_translate_c.c file.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef _SCO_ELF
#include <strings.h>
#endif

#define UNKNOWN                        0
#define FOUND_COMMENT                  1
#define FOUND_TRANSLATIONS_FILE_NAME   2
#define FOUND_EMPTY_LINE               3
#define FOUND_MSGID                    4
#define FOUND_MSGSTR                   5
#define FOUND_STRING                   6
#define FOUND_EMPTY_STRING             7
#define FOUND_FUZZY                    8
#define FOUND_EOF                      9

static char line_buffer[512];

typedef struct file_table_s {
    char *filename;
    FILE *filehandle;
    int fuzzy;
    int empty;
} file_table_t;

static file_table_t files[] = {
    { "da.po", NULL, 0, 0 },
    { "de.po", NULL, 0, 0 },
    { "fr.po", NULL, 0, 0 },
    { "hu.po", NULL, 0, 0 },
    { "it.po", NULL, 0, 0 },
    { "ko.po", NULL, 0, 0 },
    { "nl_iso.po", NULL, 0, 0 },
    { "pl_iso.po", NULL, 0, 0 },
    { "ru.po", NULL, 0, 0 },
    { "sv_iso.po", NULL, 0, 0 },
    { "tr.po", NULL, 0, 0 },
    { NULL, NULL, 0, 0 }
};

static FILE *outfile = NULL;

static int po2table_getline(FILE *file)
{
    char c = 0;
    int counter = 0;

    while (c != '\n' && !feof(file) && counter < 511) {
        c = fgetc(file);
        line_buffer[counter++] = c;
    }
    line_buffer[counter] = 0;

    if (feof(file)) {
        return FOUND_EOF;
    }

    if (line_buffer[0] == '#' && line_buffer[1] == '~') {
        return FOUND_EOF;
    }

    if (line_buffer[0] == '#' && line_buffer[1] == ' ') {
        return FOUND_COMMENT;
    }

    if (line_buffer[0] == '#' && line_buffer[1] == '.') {
        return FOUND_COMMENT;
    }

    if (line_buffer[0] == '#' && line_buffer[1] == ':') {
        return FOUND_TRANSLATIONS_FILE_NAME;
    }

    if (line_buffer[0] == '"' && line_buffer[1] != '"') {
        return FOUND_STRING;
    }

    if (line_buffer[0] == '"' && line_buffer[1] == '"') {
        return FOUND_EMPTY_STRING;
    }

    if (line_buffer[0] == '\n') {
        return FOUND_EMPTY_LINE;
    }

    if (!strncasecmp(line_buffer, "msgid", 5)) {
        return FOUND_MSGID;
    }

    if (!strncasecmp(line_buffer, "msgstr", 6)) {
        return FOUND_MSGSTR;
    }

    if (!strncasecmp(line_buffer, "#, fuzzy", 8)) {
        return FOUND_FUZZY;
    }

    if (line_buffer[0] == '#' && line_buffer[1] == ',') {
        return FOUND_COMMENT;
    }

    return UNKNOWN;
}

static void close_all(void)
{
    int i;

    for (i = 0; files[i].filename != NULL; i++) {
        if (files[i].filehandle != NULL) {
            fclose(files[i].filehandle);
        }
    }

    if (outfile != NULL) {
        fclose(outfile);
    }
}

int main(int argc, char *argv[])
{
    int found = UNKNOWN;
    int i;
    int failed = 0;
    int eof_found = 0;

    for (i = 0; files[i].filename != NULL; i++) {
        files[i].filehandle = fopen(files[i].filename, "rb");
        if (files[i].filehandle == NULL) {
            failed++;
        }
    }
        
    outfile = fopen("po_table.h", "wb");
    if (outfile == NULL) {
        failed++;
    }

    if (failed) {
        printf("Could not open all files\n");
        close_all();
        return 1;
    }

    fprintf(outfile, "/* autogenerated file, do NOT edit!!! */\n\n");
    fprintf(outfile, "typedef struct lang_table_s {\n");
    fprintf(outfile, "    char *msgid;\n");
    fprintf(outfile, "    char *msgstr_da;\n");
    fprintf(outfile, "    char *msgstr_de;\n");
    fprintf(outfile, "    char *msgstr_fr;\n");
    fprintf(outfile, "    char *msgstr_hu;\n");
    fprintf(outfile, "    char *msgstr_it;\n");
    fprintf(outfile, "    char *msgstr_ko;\n");
    fprintf(outfile, "    char *msgstr_nl;\n");
    fprintf(outfile, "    char *msgstr_pl;\n");
    fprintf(outfile, "    char *msgstr_ru;\n");
    fprintf(outfile, "    char *msgstr_sv;\n");
    fprintf(outfile, "    char *msgstr_tr;\n");
    fprintf(outfile, "} lang_table_t;\n\n");
    fprintf(outfile, "static lang_table_t text[] = {\n");

    while (!feof(files[0].filehandle) && eof_found == 0) {
        for (i = 0; files[i].filename != NULL; i++) {
            found = UNKNOWN;
            files[i].fuzzy = 0;
            while (found != FOUND_TRANSLATIONS_FILE_NAME && found != FOUND_EOF) {
                found = po2table_getline(files[i].filehandle);
            }
            if (found == FOUND_EOF) {
                eof_found = 1;
                break;
            }
            while (found != FOUND_MSGID) {
                found = po2table_getline(files[i].filehandle);
                if (found == FOUND_FUZZY) {
                    files[i].fuzzy = 1;
                }
            }
            if (i == 0) {
                fprintf(outfile, "    { ");
                fwrite(line_buffer + 6, 1, strlen(line_buffer + 6), outfile);
                found = po2table_getline(files[i].filehandle);
                while (found != FOUND_MSGSTR) {
                    fwrite(line_buffer, 1, strlen(line_buffer), outfile);
                    found = po2table_getline(files[i].filehandle);
                }
                fprintf(outfile, ",\n      ");
            }
            while (found != FOUND_MSGSTR) {
                found = po2table_getline(files[i].filehandle);
            }
            if (files[i].fuzzy) {
                fprintf(outfile, (files[i + 1].filename == NULL) ? "\"\" }," : "\"\",\n      ");
                while (found != FOUND_EMPTY_LINE && found != FOUND_EOF) {
                    found = po2table_getline(files[i].filehandle);
                }
                if (found == FOUND_EOF) {
                    eof_found = 1;
                }
            } else {
                fwrite(line_buffer + 7, 1, strlen(line_buffer + 7), outfile);
                found = po2table_getline(files[i].filehandle);
                while (found != FOUND_EMPTY_LINE && found != FOUND_EOF) {
                    fwrite(line_buffer, 1, strlen(line_buffer), outfile);
                    found = po2table_getline(files[i].filehandle);
                }
                if (found == FOUND_EOF) {
                    eof_found = 1;
                }
                if (files[i + 1].filename != NULL) {
                    fprintf(outfile, ",\n      ");
                } else {
                    fprintf(outfile, " },\n");
                }
            }
        }
    }
    fprintf(outfile, "    { ");
    for (i = 0; files[i].filename != NULL; i++) {
        if (i != 0) {
            fprintf(outfile, ", ");
        }
        fprintf(outfile, "NULL");
    }
    fprintf(outfile, " }\n};\n");
    close_all();
    return 0;
}
