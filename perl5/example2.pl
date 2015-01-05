#!/usr/bin/env perl

# Example STFL Program
#
# Authors:
#   Davor Ocelic, docelic-stfl@spinlocksolutions.com
#   http://www.spinlocksolutions.com/
#
# License:
#   GPL

use warnings;
use strict;
use stfl;

my $layout = <<EOT;
table
  list[list]
    .expand:h
    .border:lrtb
    pos[listpos]:0
    pos_name[listposname]:li0
    listitem[li0] text[li0text]:"ListItem 0"
    listitem[li1] text[li1text]:"ListItem 1"
    listitem[li2] text[li2text]:"ListItem 2"
  tablebr
  label[label]
    .expand:h
    .border:lrtb
    text[labeltext]:
EOT

my $stfl = stfl::create $layout;

while (1) {
  my $event = $stfl->run(0);
  my $focus = $stfl->get_focus;
  my $pos = $stfl->get('listpos');
  my $pos_name = $stfl->get('listposname');
  my $text = $stfl->get("${pos_name}text");

  $stfl->set('labeltext', "List is at position $pos, name $pos_name, text '$text'");

	next unless $event;

  stfl::redraw if $event eq '^L';
  last if $event eq 'ESC';
}
