#!/usr/bin/perl -w
#
#  STFL - The Structured Terminal Forms Language/Library
#  Copyright (C) 2006  Clifford Wolf <clifford@clifford.at>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  example.pl: A little STFL SPL example
#

use strict;
use stfl;

my @files;

{
	my $dirlist = join " ", @ARGV;
	open(F, "find $dirlist -iname '*.mp3' -o -iname '*.ogg' |");
	while (<F>) { chomp; push @files, $_; }
	close F;
}

if ($#files == -1) {
	print STDERR "Usage: $0 <directories-with-mp3s-and-or-oggs>\n";
	exit 1
}

my $f = stfl::create( <<EOT );
table
  label
    .colspan:2 .expand:0 .tie:c
    text:"My little jukebox"
  tablebr
  label
    .border:ltb .expand:0
    text:"Search: "
  input[search]
    .border:rtb .expand:h
    style_normal:attr=underline
    style_focus:fg=red,attr=underline
    text[searchterm]:
  tablebr
  !list[filelist]
    .colspan:2 .border:rltb
    style_focus:fg=red
    pos_name[listposname]:
    pos[listpos]:0
  tablebr
  label
    .colspan:2 .expand:0 .tie:r
    text[helpmsg]:
EOT

sub newlist()
{
	my @templist;
	my $searchterm = $f->get("searchterm");

	for (my $i = 0; $i <= $#files; $i++) {
		push @templist, $i if $files[$i] =~ /$searchterm/i;
	}

	my $code = "{list";
	for (1..30) {
		last if $#templist < 0;
		my $tempid = int(rand($#templist + 1));
		my $id = $templist[$tempid];
		splice @templist, $tempid, 1;
		my $filename = $files[$id];
		$filename =~ s/.*\/(.*?\/.*?)$/$1/;
		$filename =~ s/.{4,}(.{70})/...$1/;
		$filename = stfl::quote($filename);
		$code .= "{listitem[file_$id] text:$filename}";
	}
	$code .= "}";

	$f->modify("filelist", "replace_inner", $code);
	$f->set("listpos", "0");
}

sub play
{
	$f->get("listposname") =~ /(\d+)/;
	my $filename = $files[$1];
	$filename =~ s/'/'\\''/g;
	system("xmms -e '$filename' &");
}

sub helpmsg
{
	my $focus = $f->get_focus();

	$f->set("helpmsg", "[ F1 or ENTER = regenerate list | ESC = exit ]")
		if defined $focus and $focus eq "search";

	$f->set("helpmsg", "[ F1 = regenerate list | ENTER = send to xmms | ESC = exit ]")
		if defined $focus and $focus eq "filelist";
}

newlist;
helpmsg;

while (1)
{
	my $event = $f->run(0);
	my $focus = $f->get_focus();
	helpmsg;

	next unless defined $event;
	newlist if $event eq "F1";
	newlist if $event eq "ENTER" and defined $focus and $focus eq "search";
	play if $event eq "ENTER" and defined $focus and $focus eq "filelist";
	last if $event eq "ESC";
}

