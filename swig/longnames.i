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
 *  longnames.i: Use long function names in SWIG bindings
 */

%rename(stfl_create) stfl_create_wrapper;
%rename(stfl_run) stfl_run_wrapper;

%rename(stfl_get) stfl_get_wrapper;
%rename(stfl_set) stfl_set_wrapper;

%rename(stfl_get_focus) stfl_get_focus_wrapper;
%rename(stfl_set_focus) stfl_set_focus_wrapper;

%rename(stfl_quote) stfl_quote_wrapper;
%rename(stfl_dump) stfl_dump_wrapper;
%rename(stfl_modify) stfl_modify_wrapper;
%rename(stfl_lookup) stfl_lookup_wrapper;

%rename(stfl_error) stfl_error_wrapper;
%rename(stfl_error_action) stfl_error_action_wrapper;

