#!/usr/bin/env python

# Example STFL Program
#
# Authors:
#   Davor Ocelic, docelic-stfl@spinlocksolutions.com
#   http://www.spinlocksolutions.com/
#
# License:
#   GPL

import stfl

layout = '''
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
'''

stfl_obj = stfl.create(layout)

while True:
  event = stfl_obj.run(0)
  focus = stfl_obj.get_focus
  pos = stfl_obj.get('listpos')
  pos_name = stfl_obj.get('listposname')
  text = stfl_obj.get(pos_name + 'text')

  stfl_obj.set('labeltext', "List is at position %s, name %s, text '%s'" % (pos, pos_name, text))

  if not event: next

  if event == '^L': stfl.redraw()
  if event == 'ESC': break
