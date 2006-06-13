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
 *  shortnames.i: Use short function names in SWIG bindings
 */

%rename(create) stfl_create;
// %rename(free) stfl_free;

%rename(run) stfl_run;
%rename(reset) stfl_reset;

%rename(get) stfl_get;
%rename(set) stfl_set;

%rename(get_focus) stfl_get_focus;
%rename(set_focus) stfl_set_focus;

%rename(quote) stfl_quote;
%rename(dump) stfl_dump;
%rename(modify) stfl_modify;
%rename(lookup) stfl_lookup;

%rename(error) stfl_error;
%rename(error_action) stfl_error_action;

