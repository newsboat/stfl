/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006  Clifford Wolf <clifford@clifford.at>
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  stfl_compat.h: Some compatibility hacks for b0rken architectures
 */

#ifndef STFL__COMPAT_H
#define STFL__COMPAT_H 1

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static inline wchar_t *compat_wcsdup(const wchar_t *src)
{
	size_t n = wcslen(src) * sizeof(wchar_t);
	wchar_t *dest = malloc(n);
	memcpy(dest, src, n);
	return dest;
}

#endif

