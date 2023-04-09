#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <stdbool.h>

#include "cat_impl.h"

static void
printUsage(const char *progname)
{
    fprintf(stderr, "Usage: %s [OPTION]... [FILE]...\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, " -h | --help               display this help text and exit\n");
    fprintf(stderr, " -n | --line-number        print line number with output lines\n");
    fprintf(stderr, " -H | --head N             print first N lines\n");
    fprintf(stderr, " -T | --tail N             print last N lines\n");
    fprintf(stderr, " -N | --nth N              print every N-th line\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "With no FILE, or when FILE is -, read standard input.\n");
    fprintf(stderr, "FILE may be given multiple times.\n");
    fprintf(stderr, "Exit status is 0 if any line was printed, 1 otherwise;\n");
}


static void
printUsageShort(const char *progname)
{
    fprintf(stderr, "Usage: %s [OPTION]... [FILE]...\n", progname);
    fprintf(stderr, "Try %s --help for more information\n", progname);
}


static int
parseULL(unsigned long long *r,
         const char *str)
{
    errno = 0;
    *r = strtoull(str, NULL, 10);

    if (errno != 0) {
        fprintf(stderr, "Unable to parse: %s\n", str);
        return -1;
    }

    return 0;
}


static int
processFile(Cat *cat,
            const char *file)
{
    FILE *fp = NULL;
    char *line = NULL;
    size_t linelen = 0;
    int ret = -1;

    if (!file || strcmp(file, "-") == 0)
        file = "/dev/stdin";

    if (!(fp = fopen(file, "r"))) {
        perror(file);
        goto cleanup;
    }

    while (1) {
        ssize_t n;

        n = getline(&line, &linelen, fp);
        if (n < 0) {
            if (feof(fp)) {
                break;
            } else {
                perror(file);
                goto cleanup;
            }
        }

        if (n > 0 && line[n-1] == '\n')
            line[n-1] = '\0';

        CatAddLine(cat, line);
    }

    ret = 0;
 cleanup:
    if (fp)
        fclose(fp);
    free(line);
    return ret;
}


static void
printCat(Cat *cat)
{
    char **lines = NULL;
    size_t nlines = 0;
    size_t i;

    nlines = CatGetLines(cat, &lines);

    for (i = 0; i < nlines; i++) {
        printf("%s\n", lines[i]);
        g_free(lines[i]);
    }

    g_free(lines);
}


int main(int argc, char *argv[])
{
    int ret = EXIT_FAILURE;
    Cat *cat = NULL;
    CatMode catMode = CAT_MODE_COPY;
    unsigned long long catlines = 0;
    bool linenumber = false;
    int headSet = 0;
    int tailSet = 0;
    int nthSet = 0;
    int i;
    struct option opts[] = {
        { "help", no_argument, NULL, 'h' },
        { "line-number", no_argument, NULL, 'n' },
        { "head", required_argument, NULL, 'H' },
        { "tail", required_argument, NULL, 'T' },
        { "nth", required_argument, NULL, 'N' },
        {0, 0, 0, 0}
    };

    while (1) {
        int optidx = 0;
        int c;

        c = getopt_long(argc, argv, "+hnH:T:N:", opts, &optidx);

        if (c == -1)
            break;

        switch (c) {
        case 'h':
            printUsage(argv[0]);
            exit(EXIT_SUCCESS);

        case 'n':
            linenumber = true;
            break;

        case 'H':
            if (parseULL(&catlines, optarg) < 0)
                goto cleanup;
            headSet = 1;
            catMode = CAT_MODE_HEAD;
            break;

        case 'T':
            if (parseULL(&catlines, optarg) < 0)
                goto cleanup;
            tailSet = 1;
            catMode = CAT_MODE_TAIL;
            break;

        case 'N':
            if (parseULL(&catlines, optarg) < 0)
                goto cleanup;
            nthSet = 1;
            catMode = CAT_MODE_NTH;
            break;

        case '?':
        default:
            printUsageShort(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (headSet + tailSet + nthSet > 1) {
        fprintf(stderr, "Only one of -H/-T/-N can be set.\n");
        goto cleanup;
    }

    cat = CatInit(catMode, catlines, linenumber);
    if (!cat)
        goto cleanup;

    if (optind == argc) {
        if (processFile(cat, NULL) < 0)
            goto cleanup;
    } else {
        for (i = optind; i < argc; i++) {
            if (processFile(cat, argv[i]) < 0)
                goto cleanup;
        }
    }

    printCat(cat);

    ret = EXIT_SUCCESS;
 cleanup:
    CatFree(cat);
    return ret;
}
