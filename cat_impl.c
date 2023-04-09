#include <glib.h>

#include "cat_impl.h"


struct _Cat {
    /* Implement me */
};


/**
 * CatInit:
 * @mode: mode that cat operates in
 * @catlines: number of lines to store
 * @linenumber: whether line numbers should be stored
 *
 * Allocates and initializes Cat structure. The @mode specifies
 * which mode should the Cat work in. For modes other than
 * CAT_MODE_COPY, the @catlines argument specifies corresponding
 * number argument, i.e. for CAT_MODE_HEAD how many lines since
 * the beginning should be stored, for CAT_MODE_TAIL how many
 * lines before EOF should be stored, and finally for
 * CAT_MODE_NTH it's every N-th line.
 *
 * If @linenumber is true, then lines should be prefixed with
 * their line number.
 *
 * Use CatFree() to free the structure.
 *
 * Returns: An allocated Cat structure.
 */
Cat *CatInit(CatMode mode,
             unsigned long long catlines,
             bool linenumber)
{
    /* Implement me */
}


/**
 * CatAddLine:
 * @cat: Cat structure
 * @line: Line to add
 *
 * Adds a line into the Cat structure. It can be assumed that
 * @line contains no newline character.
 *
 * If @linenumber was true in CatInit(), then a line should be
 * prefixed with its line number in the following format:
 *
 *   ${lineNr}: ${line}
 *
 * Otherwise, the line should be stored as is.
 */
void CatAddLine(Cat *cat,
                const char *line)
{
    /* Implement me */
}


/**
 * CatGetLines:
 * @cat: Cat structure
 * @lines: where to store a copy of lines
 *
 * Obtain a copy of lines stored earlier by CatAddLine().
 * May be called multiple times.
 *
 * Returns: number of lines returned in @lines.
 */
size_t CatGetLines(Cat *cat,
                   char ***lines)
{
    /* Implement me */
}


/**
 * CatFree:
 * @cat: Cat structure
 *
 * Frees a structure previously allocated via CatInit().
 * Accepts NULL.
 */
void CatFree(Cat *cat)
{
    /* Implement me */
}
