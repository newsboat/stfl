#!/usr/bin/env python

__author__ = "chrysn <chrysn@fsfe.org>"
__copyright__ = "Copyright 2007, chrysn"
__license__ = "GPL"

import sys
import csv

import stfl

class CSV(object):
	def __init__(self,file,delimiter):
		self.file=file
		self.delimiter=delimiter

		self.data=[l for l in csv.reader(open(self.file),delimiter=self.delimiter)]
		maxlen=max([len(l) for l in self.data])
		for l in self.data:
			while len(l)<maxlen: l.append("")
	
	@staticmethod
	def cellname(row,col): return "cell_%d_%d"%(row,col)
	@staticmethod
	def valuename(row,col): return "value_%d_%d"%(row,col)
	@staticmethod
	def namecell(name): 
		tag,row,col=name.split('_')
		assert tag=='cell'
		row,col=int(row),int(col)
		return row,col
	
	def stfltable(self):
		return "{table .expand:1 %s}"%"{tablebr}".join([
				" ".join([
					"{input[%s] text[%s]:%s style_focus:bg=blue}"%(self.cellname(row,col),self.valuename(row,col),stfl.quote(x))
					for col,x in zip(range(len(l)),l)
					])
				for row,l in zip(range(len(self.data)),self.data)
			])
	
	def updatefromstfl(self, f):
		for row in range(len(self.data)):
			for col in range(len(self.data[row])):
				self.data[row][col]=f.get(self.valuename(row,col))

	
	def save(self):
		w=csv.writer(open(self.file,'w'),delimiter=self.delimiter)
		w.writerows(self.data)
	

if __name__=="__main__":
	if len(sys.argv) not in [2,3]:
		print "Usage: %s file.csv [delimiter]"%sys.argv[0]
		sys.exit(1)
	
	c=CSV(sys.argv[1], len(sys.argv)==3 and sys.argv[2] or ",")

	table=c.stfltable()

	form="{vbox %s {label} {hbox .expand:0 @style_normal:attr=reverse {label text[statusbar]:}{label text[help]: .tie:r}}"%table
	
	f=stfl.create(form)

	def setstatus(text):
		f.set('statusbar',text)
	def sethelp(text):
		f.set('help',text)
	def currentcell():
		return c.namecell(f.get_focus())
	def setcell(row,col):
		f.set_focus(c.cellname(row,col))
	
	setstatus("editing %s"%c.file)
	sethelp("^W = write, ^C = exit")

	while True:
		try:
			e=f.run(0)
		except KeyboardInterrupt:
			e='^C' # other possible reasons?
		row,col=currentcell()
		setstatus("editing %s, row %d, col %d"%(c.file,row,col))
		if e=="ENTER":
			row=row+1
			setcell(row,col)
		elif e=='^C':
			break;
		elif e=='^W':
			c.updatefromstfl(f)
			c.save()
			setstatus("saved to %s"%c.file)
