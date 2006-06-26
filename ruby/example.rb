#!/usr/bin/env ruby
#
#  STFL - The Structured Terminal Forms Language/Library
#  Copyright (C) 2006  Andreas Krennmair <ak@synflood.at>
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
#  example.rb: A little STFL Ruby example

require 'stfl'

layout = <<EOT
vbox 
  @style_normal:bg=blue
  table
    label
      .colspan:2 .expand:0 .tie:c
      text:"vCard Creator"
    tablebr
    label
      .border:ltb .expand:0
      text:"First Name: "
    input[firstname]
      .border:rtb .expand:h
      style_normal:attr=underline
      style_focus:fg=red,attr=underline
      text[firstnametext]:
    tablebr
    label
      .border:ltb .expand:0
      text:"Last Name: "
    input[lastname]
      .border:rtb .expand:h
      style_normal:attr=underline
      style_focus:fg=red,attr=underline
      text[lastnametext]:
    tablebr
    label
      .border:ltb .expand:0
      text:"Email Address: "
    input[email]
      .border:rtb .expand:h
      style_normal:attr=underline
      style_focus:fg=red,attr=underline
      text[emailtext]:
    tablebr
    label
      .border:ltb .expand:0
      text:"Save to File: "
    input[file]
      .border:rtb .expand:h
      style_normal:attr=underline
      style_focus:fg=red,attr=underline
      text[filetext]:
    tablebr
    label
      .colspan:2 .expand:0 .tie:r
      text[msg]:
EOT

$stfl = Stfl.create(layout)

def save_vcard
  filename = $stfl.get("filetext")
  if filename then
    firstname = $stfl.get("firstnametext")
    lastname = $stfl.get("lastnametext")
    email = $stfl.get("emailtext")
    vcard = <<EOVCARD
BEGIN:VCARD
VERSION:2.1
FN:#{firstname} #{lastname}
N:#{lastname};#{firstname}
EMAIL;INTERNET:#{email}
END:VCARD
EOVCARD
  end
  File.open(filename,"w") do |f|
    f.write(vcard)
  end

  $stfl.set("msg","Stored vCard to #{filename} ")
end

loop do
  event = $stfl.run(0)
  focus = $stfl.get_focus

  break if event == "ESC"
  
  if event == "ENTER" and focus and focus == "file" then
    save_vcard
  end
end

