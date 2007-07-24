/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2007  Clifford Wolf <clifford@clifford.at>
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
 *  stfl_compat.h: Some compatibility hacks for b0rken architectures
 */

#ifndef STFL__COMPAT_H
#define STFL__COMPAT_H 1

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static inline wchar_t *compat_wcsdup(const wchar_t *src)
{
	size_t n = (wcslen(src) + 1) * sizeof(wchar_t);
	wchar_t *dest = malloc(n);
	memcpy(dest, src, n);
	return dest;
}

#endif

