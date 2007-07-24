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
 *  shortnames.i: Use short function names in SWIG bindings
 */

%rename(create) stfl_create_wrapper;
%rename(run) stfl_run_wrapper;

%rename(get) stfl_get_wrapper;
%rename(set) stfl_set_wrapper;

%rename(get_focus) stfl_get_focus_wrapper;
%rename(set_focus) stfl_set_focus_wrapper;

%rename(quote) stfl_quote_wrapper;
%rename(dump) stfl_dump_wrapper;
%rename(modify) stfl_modify_wrapper;
%rename(lookup) stfl_lookup_wrapper;

%rename(error) stfl_error_wrapper;
%rename(error_action) stfl_error_action_wrapper;

%rename(reset) stfl_reset;

