#!/usr/bin/env python

import mmap
import random

random.seed()

def randomize_line(line, color1, color2, bias):
	out = ''
	last_color = None

	for i in range(len(line)):
		if line[i] != ' ':
			if random.randint(0, 100) < bias:
#				if last_color != color1:
					out += '{'
					out += color1
#					last_color = color1
			else:
#				if last_color != color2:
					out += '{'
					out += color2
#					last_color = color2
		out += line[i]

	return out

count = 0

with open('misc.old') as f:
	for line in f.readlines():
		count += 1
		if count == 21 or count == 27 or count == 35 or count == 41:
			line = randomize_line(line, 'C', 'T', 50);
		elif count == 22 or count == 26 or count == 36 or count == 40:
			line = randomize_line(line, 'g', 'C', 60);
		elif count == 23 or count == 25 or count == 37 or count == 39:
			line = randomize_line(line, 'W', 'g', 60);

		print line,

#		if count > 41:
#			break
