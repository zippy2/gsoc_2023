#pragma once

#include <stdbool.h>

typedef enum {
    CAT_MODE_COPY,
    CAT_MODE_HEAD,
    CAT_MODE_TAIL,
    CAT_MODE_NTH
} CatMode;

typedef struct _Cat Cat;

Cat *CatInit(CatMode mode,
             unsigned long long catlines,
             bool linenumber);

void CatAddLine(Cat *cat,
                const char *line);

size_t CatGetLines(Cat *cat,
                   char ***lines);

void CatFree(Cat *cat);
