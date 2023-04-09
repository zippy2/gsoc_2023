#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>

#include "cat_impl.h"

static unsigned long testCounter = 0;


static const char *
translateTestRet(int rc)
{
    static int term = -1;

    if (term < 0)
        term = isatty(STDOUT_FILENO);

    if (term > 0) {
        if (rc == 0)
            return  "\e[32mOK\e[0m\n";
        else
            return "\e[31m\e[1mFAILED\e[0m\n";
    } else {
        if (rc == 0)
            return "OK\n";
        else
            return "FAILED\n";
    }
}


static int
callTest(const char *name,
         int (*body)(const void *data),
         const void *data)
{
    int rc;

    testCounter++;
    printf("%2lu) %-65s ... ", testCounter, name);
    fflush(stdout);

    rc = body(data);

    printf(translateTestRet(rc));

    return rc == 0 ? 0 : -1;
}


static int
testSimpleFree(const void *data G_GNUC_UNUSED)
{
    CatFree(NULL);
    return 0;
}


static int
testSimpleAllocAndFree(const void *data G_GNUC_UNUSED)
{
    Cat *cat = NULL;

    cat = CatInit(CAT_MODE_COPY, 0, false);

    CatFree(cat);
    return 0;
}


static int
testAdvancedAddLine(const void *data G_GNUC_UNUSED)
{
    Cat *cat = NULL;
    char *line1 = g_strdup("hello");
    char *line2 = g_strdup("world");
    char **lines = NULL;
    size_t nlines = 0;
    size_t i;
    int ret = -1;

    cat = CatInit(CAT_MODE_TAIL, 1, false);

    CatAddLine(cat, line1);
    g_clear_pointer(&line1, g_free);

    nlines = CatGetLines(cat, &lines);
    if (nlines != 1) {
        fprintf(stderr, "Unexpected number of lines: %zu\n", nlines);
        goto cleanup;
    }

    if (strcmp(lines[0], "hello") != 0) {
        fprintf(stderr, "Invalid line. Expected 'hello' got '%s'\n", lines[0]);
        goto cleanup;
    }

    g_free(lines[0]);
    g_free(lines);

    CatAddLine(cat, line2);
    g_clear_pointer(&line2, g_free);

    nlines = CatGetLines(cat, &lines);
    if (nlines != 1) {
        fprintf(stderr, "Unexpected number of lines: %zu\n", nlines);
        goto cleanup;
    }

    if (strcmp(lines[0], "world") != 0) {
        fprintf(stderr, "Invalid line. Expected 'world' got '%s'\n", lines[0]);
        goto cleanup;
    }

    ret = 0;
 cleanup:
    for (i = 0; i < nlines; i++) {
        g_free(lines[i]);
    }
    g_free(lines);
    g_free(line2);
    g_free(line1);
    CatFree(cat);
    return ret;
}



typedef struct _testData testData;
struct _testData {
    CatMode mode;
    unsigned long long catlines;
    bool linenumber;
    const char **input;
    const char **output;
};

static int
testCat(const void *opaque)
{
    const testData *data = opaque;
    Cat *cat = NULL;
    char **actual = NULL;
    size_t nactual = 0;
    size_t i;
    int ret = -1;

    cat = CatInit(data->mode, data->catlines, data->linenumber);

    for (i = 0; data->input[i]; i++) {
        CatAddLine(cat, data->input[i]);
    }

    nactual = CatGetLines(cat, &actual);

    for (i = 0; i < nactual; i++) {
        if (!data->output[i]) {
            fprintf(stderr, "Unexpected line: '%s'\n", actual[i]);
            goto cleanup;
        }

        if (strcmp(actual[i], data->output[i]) != 0) {
            fprintf(stderr, "Invalid line at index %zu: expected '%s', got '%s'\n",
                    i, data->output[i], actual[i]);
            goto cleanup;
        }
    }

    if (data->output[i] != 0) {
        fprintf(stderr, "Returned lines ended early, expected '%s', got nothing\n",
                data->output[i]);
        goto cleanup;
    }

    ret = 0;
 cleanup:
    for (i = 0; i < nactual; i++) {
        g_free(actual[i]);
    }
    g_free(actual);
    CatFree(cat);
    return ret;
}


int main(int argc G_GNUC_UNUSED,
         char *argv[] G_GNUC_UNUSED)
{
    int ret = 0;

#define TEST(name, func, data) \
    do { \
        if (callTest(name, func, data) < 0) \
        ret = -1; \
    } while (0)

#define TEST_GENERIC(name, mode, catlines, linenumber, input, output) \
    do { \
        testData data = { mode, catlines, linenumber, input, output }; \
        TEST(name, testCat, &data); \
    } while (0)

#define TEST_COPY(name, linenumber, input, output) \
    TEST_GENERIC(name, CAT_MODE_COPY, 0, linenumber, input, output)

#define TEST_HEAD(name, catlines, linenumber, input, output) \
    TEST_GENERIC(name, CAT_MODE_HEAD, catlines, linenumber, input, output)

#define TEST_TAIL(name, catlines, linenumber, input, output) \
    TEST_GENERIC(name, CAT_MODE_TAIL, catlines, linenumber, input, output)

#define TEST_NTH(name, catlines, linenumber, input, output) \
    TEST_GENERIC(name, CAT_MODE_NTH, catlines, linenumber, input, output)

    TEST("simple free", testSimpleFree, NULL);
    TEST("simple alloc and free", testSimpleAllocAndFree, NULL);
    TEST("advanced add line", testAdvancedAddLine, NULL);

    TEST_COPY("copy 1", false,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "hello", "world", NULL }));

    TEST_COPY("copy 2", false,
              ((const char*[]){ "", NULL }),
              ((const char*[]){ "", NULL }));

    TEST_COPY("copy 3", true,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "1: hello", "2: world", NULL }));

    TEST_HEAD("head 1", 1, false,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "hello",  NULL }));

    TEST_HEAD("head 1 linenr", 1, true,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "1: hello",  NULL }));

    TEST_HEAD("head 2", 2, false,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "hello", "world", NULL }));

    TEST_HEAD("head 2 linenr", 2, true,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "1: hello", "2: world", NULL }));

    TEST_HEAD("head 3", 3, false,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "hello", "world", NULL }));

    TEST_TAIL("tail 1", 1, false,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "world", NULL }));

    TEST_TAIL("tail 1 linenr", 1, true,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "2: world", NULL }));

    TEST_TAIL("tail 2", 2, false,
              ((const char*[]){ "hello", "world", NULL }),
              ((const char*[]){ "hello", "world", NULL }));

    TEST_TAIL("tail 2 linenr", 2, true,
              ((const char*[]){ "lorem", "ipsum", "dolor", "sit", "amet", "hello", "world", NULL }),
              ((const char*[]){ "6: hello", "7: world", NULL }));

    TEST_NTH("nth 1", 2, false,
              ((const char*[]){ "lorem", "ipsum", "dolor", "sit", "amet", "hello", "world", NULL }),
              ((const char*[]){ "ipsum", "sit", "hello", NULL }));

    TEST_NTH("nth 1 linenr", 2, true,
              ((const char*[]){ "lorem", "ipsum", "dolor", "sit", "amet", "hello", "world", NULL }),
              ((const char*[]){ "2: ipsum", "4: sit", "6: hello", NULL }));

    TEST_NTH("nth 2", 2, false,
              ((const char*[]){ "", "lorem", "", "ipsum", "", "dolor", "", "sit", "", "amet", "", "hello", "", "world", NULL }),
              ((const char*[]){ "lorem", "ipsum", "dolor", "sit", "amet", "hello", "world", NULL }));

    TEST_NTH("nth 2 linenr", 2, true,
              ((const char*[]){ "", "lorem", "", "ipsum", "", "dolor", "", "sit", "", "amet", "", "hello", "", "world", NULL }),
              ((const char*[]){ "2: lorem", "4: ipsum", "6: dolor", "8: sit", "10: amet", "12: hello", "14: world", NULL }));

    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
