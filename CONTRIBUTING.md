# contributing

**Contributions will open by project version `1.0.0`.**

This file describes the rules for contributing to this project.

## 1 introduction

### 1.1 general rules
the use of generative AI to modify this project (whether that's modifying
assets, text files or source code) is completely prohibited.

### 1.2 language
this code-base is programmed in the `C89` language, and `GLSL 4.30 (core)` for
shaders.

## 2 code-base rules

### 2.1 general
- maximum line width is 80 characters as a soft-limit and 120 characters as a
  hard-limit, except in the very rare case a symbol name is 120+ characters
  long.
- in markdown files (`.md`), 80 characters is a hard-limit (except for text that
  can not be broken into multiple lines, like URLs or demonstrations of long
  text).
- indent size is 4.
- tabs are forbidden, all whitespace must use spaces.

### 2.2 includes
includes from within the code-base should be written as a relative path, and in
quotes (`""`) as oppose to in angle brackets (`<>`):
```
#include "../common/types.h"    /* yes */
#include <common/types.h>       /* no */
```

includes are categorized into alike groups, categories are separated by a
single empty line:
```
#include "../common/engine_info.h"
#include "../common/types.h"

#include "../file_io/file_io.h"
#include "../logger/logger.h"
#include "../math/math.h"
#include "../shaders/shader_types.h"

#include "../h/time.h"

#include "engine.h"

#include "../external/stb_image_write.h"

#include <stdio.h>
#include <inttypes.h>
```

categories and their order, from top to bottom:
- `common/` module includes (in `src/common/`).
- other modules' includes (modules are described in `README.md`).
- `h/` directory includes (in `src/h/`, will be removed soon).
- module's own includes (if the including file is within a module).
- external includes (extensions and/or plug-ins, usually in `src/external/`).
- system includes.

includes that do not belong to any of the previous categories must be grouped
in their own category, and can be put in any order between the other categories,
but system includes must always come last.

within each category, includes are ordered alphabetically.

### 2.3 code syntax
all symbol names must be lower-case, words are separated by underscores:
```
i32 good_variable_name;
i32 badVariableName;
i32 BadVariableName;
i32 Bad_Variable_Name;

struct good_struct_name {};
union good_union_name {};
enum good_enum_name {};
```

leading/trailing underscores are not allowed as they are reserved by the C standard:
```
i32 good_variable_name;
i32 _bad_variable_name;
i32 __bad_variable_name__;
```

constants and enum members must be upper-case, words are separated by
underscores:
```
const i32 GOOD_CONST_NAME = 0;
const i32 bad_const_name = 0;

enum fsl_constants
{
    GOOD_CONSTANT_NAME,
    bad_constant_name
}; /* fsl_constants */
```

defined structs should have their name appear at the bottom, kinda similar to
typedeffed structs:
```
typedef struct fsl_foo
{
} fsl_foo;

struct fsl_foo
{
}; /* fsl_foo */
```

same applies to unions and enums:
```
typedef union fsl_foo
{
} fsl_foo;

union fsl_foo
{
}; /* fsl_foo */

typedef enum fsl_bar
{
} fsl_bar;

enum fsl_bar
{
}; /* fsl_bar */
```

the pointer symbol (`*`) must go with the name, not the type:
```
i32 *good_pointer;
i32* bad_pointer;
```

symbols that must have `internal` at the end:
```
/* ---- header.h: ----------------------------------------------------------- */

FSLAPI i32 variable_y;              /* exposed by the API, not internal */
extern i32 variable_x_internal;     /* not exposed by the API, internal */

/* ---- source.c: ----------------------------------------------------------- */

i32 variable_1 = 0;                 /* exposed by the API, not internal */
i32 variable_2_internal = 0;        /* not exposed by the API, internal */
i32 variable_3_internal = 0;        /* declared in source file only, internal */
static i32 variable_4_internal = 0; /* declared in source file only, internal */

i32 function(void)
{
    /* local scope shouldn't say `internal`: */
    i32 good_local_variable_name = 0;
    i32 bad_local_variable_name_internal = 0;

    return 0;
}
```

that of course applies to functions functions (function-like macro wrappers):
```
FSLAPI i32 fsl_stringf();
extern i32 stringf_internal();

#define fsl_mem_arena_pop() \
    fsl_mem_arena_pop_func_internal();
```

it also applies to files, but differently; any files meant to be used only by
the engine should have `internal` at the end:
```
/* src/
 * | - string/
 * |   | - string.c
 * |   | - string.h
 * |   | - string_internal.h   <- for the engine only
 * | - ui/
 */
```

declarations should not mix with code, all declarations must be at the top of
a block, whether globally (top of file) or locally (top of function):
```
i32 function(void)
{
    i32 good_declaration_order = 1;

    if (good_declaration_order)
    {
    }

    i32 bad_declaration_order = 0;

    if (bad_declaration_order)
    {
    }
}
```

all declarations must be initialized:
```
    i32 good_init_value = 0;
    i32 bad_init_value;
```

### 2.4 function signatures
the return type, function name, first opening parenthesis and any arguments
that can fit the rest of the line should be in one line, otherwise only
arguments can be put into new lines:
```
i32 function_name(i32 parameter, i32 other_parameter);

i32 function_name_that_is_very_very_long(i32 very_very_long_parameter,
    i32 other_very_very_long_parameter);

i32 function_name_for_a_function_that_does_absolutely_nothing_that_is_very_very_long(
    i32 very_very_long_parameter, i32 other_very_very_long_parameter);
```

arguments that take pointers as read-only should be declared `const`:
```
/**
 *  this function modifies `pointer`, but shouldn't modify `other_pointer`.
 */
i32 function_name(i32 *pointer, const i32 *other_pointer);
```

### 2.5 code formatting
angle brackets should be on a new line so the opening and closing brackets
align vertically, which makes blocks easier to parse visually.
```
i32 function(void)
{
    if (good) /* good block */
    {
    }

    if (bad) { /* bad block */
    }
}
```

trailing `else` should be on a new line:
```
    if (good)
    {
    }
    else /* good `else` */
    {
    }

    if (bad) {
    } else { /* bad `else` */
    }
```

there shouldn't be padding around enclosing symbols' contents:
```
    if (condition)              /* good */
    if ( condition )            /* bad */

    i32 array[] = {1, 2, 3};    /* good */
    i32 array[] = { 1, 2, 3 };  /* bad */
```

when applicable, pre-increment is preferred over post-increment:
```
    for (; i < 10; ++i)         /* yes */
    for (; i < 10; i++)         /* no */
    x = y++;                    /* this is fine, since `++y` can change the
                                 * result here */
```

trailing commas are forbidden (because C89):
```
typedef enum enumerator
{
    ENUMERATION_0,
    ENUMERATION_1,
    ENUMERATION_2,
    ENUMERATION_3   /* no comma at the final element */
} enumerator;
```

### 2.6 symbol naming

#### 2.6.1 keywords
the engine's abbreviation "`fsl`" always comes first if the symbol is exposed
by the API:
```
FSLAPI u32 fsl_engine_init();       /* good name */
FSLAPI u32 engine_init();           /* bad name */
```

the module name/abbreviation comes after the engine's abbreviation:
```
i32 fsl_asset_metadata_get();       /* the module is `assets/` */
i32 fsl_mem_arena_push();           /* the module is `memory/` */
```

the subject comes before the operation by default:
```
i32 fsl_asset_metadata_get();       /* good format */
i32 fsl_get_asset_metadata();       /* bad format */
i32 fsl_asset_get_metadata();       /* bad format */
```

any scope-specific keywords should come last:
```
i32 fsl_mem_arena_push_internal();  /* scope-specific keyword is `internal` */
```

any functions meant to be called via a function pointer, typedeffed function
signatures and function-like macro wrappers should have the keyword `func` at
the end:
```
void ui_event_enter_func_internal(); /* called via a function pointer array */

typedef void (*fsl_ui_callback_func)(fsl_ui_event event, void *data);

#define fsl_mem_handle_get(handle) \
    fsl_mem_handle_get_func_internal(handle)
```

#### 2.6.2 order of keywords based on operation
operation categories:
- action (e.g., `fsl_asset_name_get()`, `fsl_mem_arena_push()`).
- request (e.g., `fsl_request_screenshot()`).
- query (e.g., `fsl_engine_is_running()`).
- callback (e.g., `fsl_callback_framebuffer_size()`).

order of keywords in "`action`" category:
```
i32 fsl_asset_name_get();

i32 fsl_asset_name_get_internal();
/*  |   |     |    |   |
 *  |   |     |    |   scope qualifier, if applicable.
 *  |   |     |    operation (e.g., `get`/`set`/`update`/`destroy`/etc..).
 *  |   |     subject (e.g., `name`/`metadata`/etc..).
 *  |   module's abbreviation, if applicable (e.g., `asset`/`log`/`mem`/etc..).
 *  engine's abbreviation.
 */

i32 fsl_render_settings_update();
```

order of keywords in "`request`" category:
```
i32 fsl_request_asset_name();
/*  |   |       |     |   |
 *  |   |       |     |   scope qualifier (there's none here).
 *  |   |       |     subject.
 *  |   |       module's abbreviation.
 *  |   operation (`request`), requests are a high priority operation, therefore
 *  |   the operation comes first.
 *  engine's abbreviation.
 */

i32 fsl_request_screenshot();
```

however, functions that respond to requests should follow the "`action`"
category:
```
/* using `respond` instead of `process` here to make intent clear */
i32 fsl_screenshot_request_respond();
```

order of keywords in "`query`" category:
```
b8 fsl_engine_is_running();
/* |   |      |  |      |
 * |   |      |  |      scope qualifier.
 * |   |      |  predicate (`running`/`available`/`done`/etc..).
 * |   |      predicate prefix (`is`/`has`/`can`/etc..).
 * |   module's abbreviation.
 * engine's abbreviation.
 */
```

order of keywords in "`callback`" category:
```
void fsl_callback_framebuffer_size();
/*   |   |        |               |
 *   |   |        |               scope qualifier.
 *   |   |        |
 *   |   |        subject.
 *   |   `callback` keyword.
 *   engine's abbreviation.
 */
```

`func` is another type of callback keyword, but used differently:
```
typedef void (*fsl_ui_callback_func)();
/*             |   |  |        |   |
 *             |   |  |        |   scope qualifier.
 *             |   |  |        `func` keyword.
 *             |   |  `callback` is the subject in this case.
 *             |   module's abbreviation.
 *             engine's abbreviation.
 */
```

both `callback` and `func` are functions called via pointers, but a `callback`
is specific to functions passed to the `engine/` module and listened to every
frame, and only on the main thread.

#### 2.6.3 conventional keywords
`get`/`set`:
```
i32 fsl_engine_metadata_get();
i32 fsl_engine_metadata_set();
```

`init`/`free` for initialization/freeing:
```
i32 fsl_font_init();
i32 fsl_font_free();

i32 fsl_engine_init();
i32 fsl_engine_close(); /* special case, because I thought it was cool */
```

`start`/`stop` for repeatable operations, after initialization:
```
i32 fsl_text_start(); /* sets up text rendering batch */

/* closes text rendering batch to prepare for drawing, without freeing
 * resources.
 */
i32 fsl_text_stop();
```

`create`/`destroy`:
```
i32 fsl_texture_handle_create();
i32 fsl_texture_handle_destroy();
```

`push`/`pop` for inserting and removing data to/from memory:
```
i32 fsl_mem_arena_push();
i32 fsl_mem_arena_pop();
```

`clear` for erasing memory, `delete` can also be used when applicable:
```
i32 fsl_mem_clear();

/* also erases memory, but it's not the primary purpose */
i32 fsl_object_delete();
```

other keywords:
`bind`, `blit`, `render`, `draw`, etc..

## 3 documentation

### 3.1 textual rules
all text must be lower-case except for abbreviations and names that need it:
```
/* examples of keywords that should be upper-case:
 * FSL:     `Fossil` abbreviation.
 * GLFW:    the `GLFW` framework name.
 * TODO:    special case, must always be upper-case.
 * David:   someone's name (I picked a random name).
 */
```

### 3.1 regular comments
all comments must be written as block comments (`/* comment */`):
```
    if (good) /* good comment */
    if (bad) // bad comment
```

a "todo" must be written on the same line as a significant (or the relevant)
piece of code, it shows clearer context when using search tools:
```
    if (good) /* TODO: please fix this code */
    {
    }

    /* TODO: please fix this code */
    if (bad)
    {
    }
```

### 3.2 symbols
all function and global symbol documentation must follow javadoc-style doxygen
formatting:
```
/**
 *  good format
 *  @good_markup_command
 */

/*!
 *  bad format
 *  \bad_markup_command
 */
```

text inside a doxygen comment should be aligned to 4 characters.
```
/**
 *  @brief short description.   <- good alignment
 * @brief short description.    <- bad alignment
 */
```

long text in a bullet point should return under the start of the text, not the
bullet point:
```
/**
 *  - some note that takes
 *    two lines.                <- good alignment
 *  - some note that takes
 *  two lines.                  <- bad alignment
 */
```

any contiguous body of text that take more than one line should be padded with
empty lines:
```
/**
 *  - some note.
 *  - another note.
 *
 *  - a long note that takes
 *    two lines and requires padding on top and bottom.
 *
 *  - more notes.
 *  - more notes.
 */
```

descriptions that surpass the 80 character limit should be put on multiple lines
and put on top of the subject instead of next to it and should employ full sized
block formatting, with padding around the description and the subject
(whitespace at the top and bottom):
```
typedef struct fsl_foo
{
    i32 var_1; /* var_1 description */
    i32 var_2; /* var_2 description */

    /**
     *  var_3 description extends too far to contain on one line, and it's been
     *  put on multiple lines.
     */
    i32 var_3;

} fsl_foo;
```

this block comes at the top of every file, after the short license placed at the
very top:
```
/**
 *  @file file_name.c
 *
 *  @brief quick description of file.
 */
```

typedeffed structs, unions and enums should be documented like this:
```
/**
 *  @brief short description of struct.
 *
 *  some text.
 *  ...
 */
typedef struct fsl_foo
{
    i32 var_1; /* var_1 description */
    i32 var_2;

    /*
     *  var_3 description.
     */
    i32 var_3;

    struct fsl_bar
    {
        i32 var_4;
    };

} fsl_foo;
```

structs/unions/enums within other structs/unions should have padding around
them:
```
struct fsl_foo
{
    i32 var_1;
    i32 var_2;

    struct fsl_bar
    {
    }; /* fsl_bar */

    i32 var_3;
    i32 var_4;
}; /* fsl_foo */
```

similar markup commands should be next to each other, unless one extends more
than one line:
```
/**
 *  @brief short description.
 *
 *  more text ...
 *  ...
 *
 *  - some note.
 *  - some note.

 *  - some very long note
      that requires two lines.

 *  - some note.
 *
 *  @param first_parameter description of first_parameter.
 *  @param second_parameter description of second_parameter.

 *  @param third_parameter description of third_parameter, this one is quite
 *  long which requires two lines.
 *
 *  @return return value description.
 */
i32 func(i32 first_parameter, i32 second_parameter, i32 third_parameter);
```

internal symbols should be marked as such in the documentation, even if the
symbol name says it:
```
/**
 *  @internal
 *
 *  @brief ...
 */
extern fsl_mem_arena mem_arena_internal;
```

## 4 file structure

all source and header files must have the license at the very top, followed
by the doxygen identifier block for the file, separated by an empty line:
```
/**
 *  license text
 *  ...
 */

/**
 *  @file file_name.extension
 *
 *  @brief quick description of file's purpose.
 */
```

for header files, ifdef guards are employed instead of "`pragma once`", and come
after the file's identifier:
```
 *  ...
 */

#ifndef FSL_FILE_NAME_H
#define FSL_FILE_NAME_H
```

every `endif` should mention its pair's text:
```
#if DEBUG_BUILD
do_the_thing
#endif /* DEBUG_BUILD */
```

in the case of header guards, the `endif` should be separated from the content
above it by an empty line:
```

#endif /* FSL_FILE_NAME_H */
```

there shouldn't be an extra empty line either at the top or at the bottom of a
file (IDEs like VS Code do insert an extra empty line at the bottom by default,
in this case it's fine, but in vim/nvim, by default it won't show the extra
empty line but it is still saved to the file, so do not add one).

## 5 lastly
thank you for reading these rules and for contributing, and have fun
programming.

I'm open to suggestions if I should change anything in these rules and
guidelines, add anything or remove anything.
