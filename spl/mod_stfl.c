/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *  
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *
 *  mod_stfl.c: STFL bindings for SPL
 */

/**
 * STFL module
 *
 * This module provides bindings to the Structured Terminal Forms
 * Language/Library (STFL).
 */

#include "stfl.h"

#include <spl.h>
#include <stdlib.h>
#include <locale.h>

static struct stfl_ipool *ipool = 0;

extern void SPL_ABI(spl_mod_stfl_init)(struct spl_vm *vm, struct spl_module *mod, int restore);
extern void SPL_ABI(spl_mod_stfl_done)(struct spl_vm *vm, struct spl_module *mod);

static void handler_stfl_form_node(struct spl_task *task, struct spl_vm *vm, struct spl_node *node, struct spl_hnode_args *args, void *data)
{
	if (args->action == SPL_HNODE_ACTION_PUT) {
		if (node->hnode_data)
			stfl_free((struct stfl_form *)node->hnode_data);
	}
}

static struct stfl_form *clib_get_stfl_form(struct spl_task *task)
{
	struct spl_node *n = spl_cleanup(task, spl_clib_get_node(task));
	struct stfl_form *f = n ? n->hnode_data : 0;

	if (!f || !n->hnode_name || strcmp(n->hnode_name, "stfl_form")) {
		spl_report(SPL_REPORT_RUNTIME, task,
				"Expected a STFL-Form as 1st argument!\n");
		return 0;
	}

	return f;
}

static struct spl_node *spl_new_nullable_ascii(const char *text)
{
	return text ? SPL_NEW_STRING_DUP(text) : spl_get(0);
}

/**
 * Quote a string to be used in STFL code.
 *
 * This function is designed to be used with the encoding/quoting operator (::).
 */
// builtin encode_stfl(text)

/**
 * Parse an STFL description text and return the form handler
 */
// builtin stfl_create(text)
static struct spl_node *handler_stfl_create(struct spl_task *task, void *data)
{
	struct spl_node *n = SPL_NEW_STRING_DUP("STFL Form");
	n->hnode_name = strdup("stfl_form");
	n->hnode_data = stfl_create(stfl_ipool_towc(ipool, spl_clib_get_string(task)));
	stfl_ipool_flush(ipool);
	return n;
}

/**
 * Display the form and process one input character
 */
// builtin stfl_run(form, timeout)
static struct spl_node *handler_stfl_run(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	struct spl_node *ret = f ? spl_new_nullable_ascii(stfl_ipool_fromwc(ipool, stfl_run(f, spl_clib_get_int(task)))) : 0;
	stfl_ipool_flush(ipool);
	return ret;
}

/**
 * Instruct STFL to completely redraw screen on next run
 */
// builtin stfl_redraw()
static struct spl_node *handler_stfl_redraw(struct spl_task *task, void *data)
{
	stfl_redraw();
	return 0;
}

/**
 * Return to standard text mode
 */
// builtin stfl_reset()
static struct spl_node *handler_stfl_reset(struct spl_task *task, void *data)
{
	stfl_reset();
	return 0;
}

/**
 * Get the value of an STFL variable
 */
// builtin stfl_get(form, name)
static struct spl_node *handler_stfl_get(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	struct spl_node *ret = f ? spl_new_nullable_ascii(stfl_ipool_fromwc(ipool, stfl_get(f, stfl_ipool_towc(ipool, spl_clib_get_string(task))))) : 0;
	stfl_ipool_flush(ipool);
	return ret;
}

/**
 * Set an STFL variable
 */
// builtin stfl_set(form, name, value)
static struct spl_node *handler_stfl_set(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	char *name = spl_clib_get_string(task);
	char *value = spl_clib_get_string(task);
	stfl_set(f, stfl_ipool_towc(ipool, name), stfl_ipool_towc(ipool, value));
	stfl_ipool_flush(ipool);
	return 0;
}

/**
 * Get the name of the widget currently having the focus
 */
// builtin stfl_get_focus(form)
static struct spl_node *handler_stfl_get_focus(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	struct spl_node *ret = f ? spl_new_nullable_ascii(stfl_ipool_fromwc(ipool, stfl_get_focus(f))) : 0;
	stfl_ipool_flush(ipool);
	return ret;
}

/**
 * Set the focus to the specified widget
 */
// builtin stfl_set_focus(form, name)
static struct spl_node *handler_stfl_set_focus(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	stfl_set_focus(f, stfl_ipool_towc(ipool, spl_clib_get_string(task)));
	stfl_ipool_flush(ipool);
	return 0;
}

/**
 * Quote a string to be used in STFL code
 */
// builtin stfl_quote(text)
static struct spl_node *handler_stfl_quote(struct spl_task *task, void *data)
{
	struct spl_node *n = spl_new_nullable_ascii(stfl_ipool_fromwc(ipool, stfl_quote(stfl_ipool_towc(ipool, spl_clib_get_string(task)))));
	stfl_ipool_flush(ipool);
	return n;
}

/**
 * Dump the STFL Code for this form
 */
// builtin stfl_dump(form, name, prefix, focus)
static struct spl_node *handler_stfl_dump(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	char *name = spl_clib_get_string(task);
	char *prefix = spl_clib_get_string(task);
	int focus = spl_clib_get_int(task);
	const char *text = stfl_ipool_fromwc(ipool, stfl_dump(f, stfl_ipool_towc(ipool, name), stfl_ipool_towc(ipool, prefix), focus));
	struct spl_node *n = spl_new_nullable_ascii(text);
	stfl_ipool_flush(ipool);
	return n;
}

/**
 * Dump the text under a widget
 */
// builtin stfl_text(form, name)
static struct spl_node *handler_stfl_text(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	char *name = spl_clib_get_string(task);
	const char *text = stfl_ipool_fromwc(ipool, stfl_text(f, stfl_ipool_towc(ipool, name)));
	struct spl_node *n = spl_new_nullable_ascii(text);
	stfl_ipool_flush(ipool);
	return n;
}

/**
 * Import STFL code to an existing form
 */
// builtin stfl_modify(form, name, mode, text)
static struct spl_node *handler_stfl_modify(struct spl_task *task, void *data)
{
	struct stfl_form *f = clib_get_stfl_form(task);
	char *name = spl_clib_get_string(task);
	char *mode = spl_clib_get_string(task);
	char *text = spl_clib_get_string(task);
	stfl_modify(f, stfl_ipool_towc(ipool, name), stfl_ipool_towc(ipool, mode), stfl_ipool_towc(ipool, text));
	stfl_ipool_flush(ipool);
	return 0;
}

/**
 * Return error message of last stfl call or undef.
 */
// builtin stfl_error()
static struct spl_node *handler_stfl_error(struct spl_task *task, void *data)
{
	struct spl_node *ret = spl_new_nullable_ascii(stfl_ipool_fromwc(ipool, stfl_error()));
	stfl_ipool_flush(ipool);
	return ret;
}

/**
 * Set error handling algorithm.
 */
// builtin stfl_error_action(mode)
static struct spl_node *handler_stfl_error_action(struct spl_task *task, void *data)
{
	char *mode = spl_clib_get_string(task);
	stfl_error_action(stfl_ipool_towc(ipool, mode));
	stfl_ipool_flush(ipool);
	return 0;
}

static void destroy_ipool_atexit()
{
	stfl_ipool_destroy(ipool);
	ipool = 0;
}

void SPL_ABI(spl_mod_stfl_init)(struct spl_vm *vm, struct spl_module *mod, int restore)
{
	if (!ipool) {
		ipool = stfl_ipool_create("UTF8");
		atexit(destroy_ipool_atexit);
		setlocale(LC_ALL,"");
	}

	spl_hnode_reg(vm, "stfl_form", handler_stfl_form_node, 0);

	spl_clib_reg(vm, "stfl_create", handler_stfl_create, 0);

	spl_clib_reg(vm, "stfl_run", handler_stfl_run, 0);
	spl_clib_reg(vm, "stfl_redraw", handler_stfl_redraw, 0);
	spl_clib_reg(vm, "stfl_reset", handler_stfl_reset, 0);

	spl_clib_reg(vm, "stfl_get", handler_stfl_get, 0);
	spl_clib_reg(vm, "stfl_set", handler_stfl_set, 0);

	spl_clib_reg(vm, "stfl_get_focus", handler_stfl_get_focus, 0);
	spl_clib_reg(vm, "stfl_set_focus", handler_stfl_set_focus, 0);

	spl_clib_reg(vm, "encode_stfl", handler_stfl_quote, 0);
	spl_clib_reg(vm, "stfl_quote", handler_stfl_quote, 0);

	spl_clib_reg(vm, "stfl_dump", handler_stfl_dump, 0);
	spl_clib_reg(vm, "stfl_text", handler_stfl_text, 0);
	spl_clib_reg(vm, "stfl_modify", handler_stfl_modify, 0);

	spl_clib_reg(vm, "stfl_error", handler_stfl_error, 0);
	spl_clib_reg(vm, "stfl_error_action", handler_stfl_error_action, 0);
}

void SPL_ABI(spl_mod_stfl_done)(struct spl_vm *vm, struct spl_module *mod)
{
	stfl_reset();
}

