%module stfl
%{
#include "stfl.h"
%}

extern struct stfl_form *stfl_create(const char *text);
extern void stfl_free(struct stfl_form *f);

extern const char *stfl_run(struct stfl_form *f, int timeout);
extern void stfl_return();

extern const char *stfl_get(struct stfl_form *f, const char *name);
extern void stfl_set(struct stfl_form *f, const char *name, const char *value);

extern const char *stfl_get_focus(struct stfl_form *f);
extern void stfl_set_focus(struct stfl_form *f, const char *name);

extern char *stfl_quote(const char *text);
extern char *stfl_dump(struct stfl_form *f, const char *name, const char *prefix, int focus);
extern void stfl_import(struct stfl_form *f, const char *name, const char *mode, const char *text);
extern const char *stfl_lookup(struct stfl_form *f, const char *path, const char *newname);

extern const char *stfl_error();
extern void stfl_error_action(const char *mode);

// FIXME:
// - Automatically call stfl_free() when variables are cleaned up
// - Free return values of stfl_quote() and stfl_dump()

