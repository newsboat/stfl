#!/usr/bin/env ruby

# Example STFL Program
#
# Authors:
#   Davor Ocelic, docelic-stfl@spinlocksolutions.com
#   http://www.spinlocksolutions.com/
#
# License:
#   GPL

require 'stfl'

layout = <<EOT
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

$stfl = Stfl.create layout

loop do
  event = $stfl.run 0
  focus = $stfl.get_focus
  pos = $stfl.get 'listpos'
  pos_name = $stfl.get 'listposname'
  text = $stfl.get "#{pos_name}text"

  $stfl.set 'labeltext', "List is at position #{pos}, name #{pos_name}, text '#{text}'"

	next unless event.length > 0

  Stfl.redraw if event == '^L'
  break if event == 'ESC'
end
