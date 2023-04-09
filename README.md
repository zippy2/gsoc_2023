Google Summer of Code 2023 Coding exercise
==========================================

### Goal: Implement a library that's similar to cat/head/tail

Your task is to provide the missing implementation for functions and structures
in `cat_impl.c`. The basic usage might look like this:

```
  Cat *cat = NULL;

  cat = CatInit(CAT_MODE_TAIL, 10, false);

  CatAddLine(cat, line); /* Might be repeated several times */

  nlines = CatGetLines(cat, &lines);

  CatFree(cat);
```

Take a look at `cat.c` or `cat_test.c` for more examples.

### Steps:

The design of `_Cat` structure is completely up to you, just like algorithms
used. You can introduce new internal functions or macros you want. You can use
standard C library and GLib. If you allocate any memory, do not forget to free
it once no longer needed.

Please make sure your code passes tests. You can run test suite by:

```
  meson test -C _build/
```

_Note: Replace `_build` with your build directory._

Feel free to ask any questions.
