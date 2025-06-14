#!/usr/bin/env python

import sys
import mmap

filename = sys.argv[1]

#global
data = None
dataptr = 0
pfile_version = 0

def bug(s):
	global data
	print >> sys.stderr, s
	print >> sys.stderr, data[dataptr-50:dataptr+50]
	exit()

def advance_whitespace():
	global data, dataptr
	while data[dataptr] == ' ' or data[dataptr] == '\n':
		dataptr += 1

def datasplit(sep=' \n'):
	global data, dataptr
	advance_whitespace()

	p = dataptr
	while data[p] not in sep:
		p += 1

	chunk = data[dataptr:p]
	dataptr = p+1
	return chunk

def read_letter():
	global data, dataptr
	advance_whitespace()
	letter = data[0]
	dataptr += 1
	return letter

def read_word():
# overly complicated to be quote aware, and preserve leading space on the rest of the data
	word = datasplit()

	if word[0] == "'" or word[0] == '"':
		if word[-1] != word[0]:
			word = ' '.join([word[1:], datasplit(word[0])])
		else:
			word = word[1:-1]

	return word

def read_number():
	return int(read_word())

def read_flags():
	return read_word() # don't parse to int, hard to read

def read_string():
	return datasplit('~')

def read_string_eol():
	return datasplit('\n')

def is_key(key, pattern, read_func, d):
	if key == pattern:
		if read_func == True or read_func == False:
			d[key] = read_func
		else:
			d[key] = read_func()
		return True

	return False

def read_char_section():
	global pfile_version
	ch = {}
	plr = {}

	while True:
		word = read_word()
#		print 'parsing word', word
		if word[0] == '*':
			read_string_eol()

		elif word[0] == 'A':
			if is_key(word, 'AfBy', read_flags, ch) \
			or is_key(word, 'Act', read_flags, ch) \
			or is_key(word, 'Alig', read_number, ch) \
			or is_key(word, 'Afk', read_string, plr) \
			or is_key(word, 'Akills', read_number, plr) \
			or is_key(word, 'Akilled', read_number, plr) \
			or is_key(word, 'Aura', read_string, plr):
				pass

			elif word == 'Alias':
				if word not in plr.keys():
					plr[word] = []
				plr[word].append((
					read_word(),
					read_string()
				))
			elif word == 'Affc':
				if word not in ch.keys():
					ch[word] = []

				ch[word].append({
					'name':read_word(), 
					'where':read_number(),
					'level':read_number(),
					'dur':read_number(),
					'mod':read_number(),
					'loc':read_number(),
					'bitv':read_number(),
					'evo':read_number() if pfile_version > 7 else 1
				})
			elif word == 'Atrib':
				ch[word] = {
					'str':read_number(),
					'int':read_number(),
					'wis':read_number(),
					'dex':read_number(),
					'con':read_number(),
					'chr':read_number()
				}
			elif word == 'AtMod':
				ch[word] = {
					'str':read_number(),
					'int':read_number(),
					'wis':read_number(),
					'dex':read_number(),
					'con':read_number(),
					'chr':read_number()
				}
			else:
				bug('weird word %s' % (word))

		elif word[0] == 'B':
			if is_key(word, 'Back', read_number, plr) \
			or is_key(word, 'Bin', read_string, plr) \
			or is_key(word, 'Bout', read_string, plr):
				pass
			elif word == 'Block_Remote':
				read_number() #ignore
			else:
				bug('weird word %s' % (word))

		elif word[0] == 'C':
			if is_key(word, 'Cgrp', read_flags, plr) \
			or is_key(word, 'Cla', read_number, ch) \
			or is_key(word, 'Clan', read_string, ch) \
			or is_key(word, 'Comm', read_flags, ch) \
			or is_key(word, 'Cnsr', read_flags, ch):
				pass
			elif word == 'Cnd':
				plr[word] = {
					'drunk':read_number(),
					'full':read_number(),
					'thirst':read_number(),
					'hunger':read_number()
				}
			elif word == 'Colr':
				if word not in plr.keys():
					plr[word] = []
				plr[word].append({
					'slot':read_number(),
					'color':read_number(),
					'bold':read_number()
				})
			else:
				bug('weird word %s' % (word))

		elif word[0] == 'D':
			if is_key(word, 'Dam', read_number, ch) \
			or is_key(word, 'Desc', read_string, ch) \
			or is_key(word, 'Deit', read_string, plr):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'E':
			if is_key(word, 'Email', read_string, plr) \
			or is_key(word, 'Exp', read_number, ch):
				pass
			elif word == 'ExSk':
				plr[word] = []
				for i in range(0, plr['RmCt']/20+1):
					plr[word].append(read_number())
			elif word == 'End':
				break
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'F':
			if is_key(word, 'Familiar', read_number, plr) \
			or is_key(word, 'Finf', read_string, plr) \
			or is_key(word, 'FImm', read_flags, ch) \
			or is_key(word, 'FRes', read_flags, ch) \
			or is_key(word, 'FVul', read_flags, ch)  \
			or is_key(word, 'FlagThief', read_number, plr) \
			or is_key(word, 'FlagKiller', read_number, plr):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'G':
			if is_key(word, 'GameIn', read_string, plr) \
			or is_key(word, 'GameOut', read_string, plr) \
			or is_key(word, 'Gold_in_bank', read_number, ch) \
			or is_key(word, 'Gold', read_number, ch) \
			or is_key(word, 'GlDonated', read_number, ch):
				pass
			elif word == 'Gr':
				if word not in plr.keys():
					plr[word] = []

				plr[word].append(read_word())
			elif word == 'Grant':
				if word not in plr.keys():
					plr[word] = []

				plr[word].append(read_word())
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'H':
			if word == 'HMS':
				if pfile_version < 14:
					ch['HMS'] = {}
					plr['HMSP'] = {}
					ch['HMS']['hit'] = read_number();
					plr['HMSP']['hit'] = read_number();
					ch['HMS']['mana'] = read_number();
					plr['HMSP']['mana'] = read_number();
					ch['HMS']['stam'] = read_number();
					plr['HMSP']['stam'] = read_number();
				else:
					ch[word] = {
						'hit':read_number(),
						'mana':read_number(),
						'stam':read_number()
					}
			elif word == 'HMSP':
				plr[word] = {
					'hit':read_number(),
					'mana':read_number(),
					'stam':read_number()
				}
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'I':
			if is_key(word, 'Id', read_number, ch) \
			or is_key(word, 'Invi', read_number, ch) \
			or is_key(word, 'Immn', read_string, plr):
				pass
			elif word == 'Ignore':
				if word not in plr.keys():
					plr[word] = []

				plr[word].append(read_string())
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'L':
			if is_key(word, 'Lay', read_number, plr) \
			or is_key(word, 'Lay_Next', read_number, plr) \
			or is_key(word, 'LLev', read_number, plr) \
			or is_key(word, 'Levl', read_number, ch) \
			or is_key(word, 'LogO', read_number, plr) \
			or is_key(word, 'LnD', read_string, ch) \
			or is_key(word, 'Lsit', read_string, plr) \
			or is_key(word, 'Lurk', read_number, ch) \
			or is_key(word, 'Ltim', read_string, plr) \
			or is_key(word, 'LSav', read_string, plr):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'M':
			if is_key(word, 'Mark', read_number, plr) \
			or is_key(word, 'Mexp', read_number, plr):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'N':
			if is_key(word, 'Name', read_string, ch):
				pass
			elif word == 'Note':
				plr[word] = {
					'note':read_number(),
					'idea':read_number(),
					'role':read_number(),
					'quest':read_number(),
					'change':read_number(),
					'pers':read_number(),
					'trade':read_number() if pfile_version > 12 else 0
				}
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'P':
			if is_key(word, 'Pass', read_string, plr) \
			or is_key(word, 'PCkills', read_number, plr) \
			or is_key(word, 'PCkilled', read_number, plr) \
			or is_key(word, 'PKRank', read_number, plr) \
			or is_key(word, 'Plyd', read_number, plr) \
			or is_key(word, 'Plr', read_flags, plr) \
			or is_key(word, 'Pnts', read_number, plr) \
			or is_key(word, 'Pos', read_number, ch) \
			or is_key(word, 'Prac', read_number, ch) \
			or is_key(word, 'Prom', read_string, ch):
				pass
			elif word == 'Ptnr':
				read_string() # ignore
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'Q':
			if is_key(word, 'QuestPnts', read_number, ch) \
			or is_key(word, 'QpDonated', read_number, ch) \
			or is_key(word, 'QuestNext', read_number, ch):
				pass
			elif word == 'Query':
				if word not in plr.keys():
					plr[word] = []

				plr[word].append(read_string())
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'R':
			if is_key(word, 'Race', read_string, ch) \
			or is_key(word, 'Rank', read_string, plr) \
			or is_key(word, 'Revk', read_flags, ch) \
			or is_key(word, 'RmCt', read_number, plr) \
			or is_key(word, 'RolePnts', read_number, plr) \
			or is_key(word, 'Room', read_number, ch):
				pass
			elif word == 'Raff':
				plr[word] = []
				for i in range(0, plr['RmCt']/10+1):
					plr[word].append(read_number())
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'S':
			if is_key(word, 'Scro', read_number, ch) \
			or is_key(word, 'Save', read_number, ch) \
			or is_key(word, 'ShD', read_string, ch) \
			or is_key(word, 'Silver_in_bank', read_number, ch) \
			or is_key(word, 'Silv', read_number, ch) \
			or is_key(word, 'Secu', read_number, ch) \
			or is_key(word, 'SkillPnts', read_number, plr) \
			or is_key(word, 'Stus', read_string, plr) \
			or is_key(word, 'Spou', read_string, plr) \
			or is_key(word, 'SQuestNext', read_number, plr):
				pass
			elif word == 'Sex':
				plr['TSex'] = read_number()
			elif word == 'Sk':
				if word not in plr.keys():
					plr[word] = []
				plr[word].append({
					'prac':read_number(),
					'evol':read_number(),
					'name':read_word()
				})
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'T':
			if is_key(word, 'TSex', read_number, plr) \
			or is_key(word, 'Trai', read_number, ch) \
			or is_key(word, 'Titl', read_string, plr):
				pass
			elif word == 'Tru':
				read_number() # ignore
			elif word == 'THMS' or word == 'THVP':
				plr['THMS'] = {
					'hit':read_number(),
					'mana':read_number(),
					'stam':read_number()
				}
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'V':
			if is_key(word, 'Video', read_flags, plr):
				pass
			elif word == 'Vers':
				pfile_version = read_number()
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'W':
			if is_key(word, 'Wimp', read_number, ch) \
			or is_key(word, 'Wizn', read_flags, ch) \
			or is_key(word, 'Wspr', read_string, plr):
				pass
			else:
				bug('weird word %s' % (word))
		else:
			bug('weird letter %s' % (word))

	return ch, plr
#	print 'done with char section'

def read_pet_section():
	print 'nyi'

def read_obj():
	global pfile_version
	nest_level = 0
	s = {}
	enchanted_obj = False

	while True:
		word = read_word()
#		print 'parsing word', word
		if word[0] == '*':
			read_string_eol()

		elif word[0] == 'A':
			if word == 'Affc':
				aff = {
					'name':read_word(), 
					'where':read_number(),
					'level':read_number(),
					'dur':read_number(),
					'mod':read_number(),
					'loc':read_number(),
					'bitv':read_number(),
					'evo':read_number() if pfile_version > 7 else 1
				}

				if enchanted_obj == True:
					if word not in s.keys():
						s[word] = []

					s[word].append(aff)
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'C':
			if is_key(word, 'Cond', read_number, s) \
			or is_key(word, 'Cost', read_number, s):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'D':
			if is_key(word, 'Desc', read_string, s):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'E':
			if is_key(word, 'ExtF', read_number, s):
				pass
			elif word == 'Enchanted':
				enchanted_obj = True
			elif word == 'ExDe':
				if word not in s.keys():
					s[word] = {}
				key = read_string()
				val = read_string()

				if key not in s[word].keys():
					s[word][key] = val
			elif word == 'End':
				break
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'I':
			if is_key(word, 'Ityp', read_number, s):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'L':
			if is_key(word, 'Lev', read_number, s):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'M':
			if is_key(word, 'Mat', read_string, s):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'N':
			if is_key(word, 'Name', read_string, s):
				pass
			elif word == 'Nest':
				nest_level = read_number()
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'S':
			if is_key(word, 'ShD', read_string, s):
				pass
			elif word == 'Splxtra':
				if word not in s.keys():
					s[word] = []
				read_number() # ignore
				s[word].append({
					'name':read_word(),
					'level':read_number()
				})
			elif word == 'Spell':
				read_number()
				read_word()
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'T':
			if is_key(word, 'Time', read_number, s):
				pass
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'V':
			if is_key(word, 'Vnum', read_number, s):
				pass
			elif word == 'Val':
				s[word] = (
					read_number(),
					read_number(),
					read_number(),
					read_number(),
					read_number()
				)
			else:
				bug('weird word %s' % (word))
		elif word[0] == 'W':
			if is_key(word, 'WeaF', read_number, s) \
			or is_key(word, 'Wt', read_number, s):
				pass
			elif word == 'Wear':
				loc = read_number()
				if loc != -1: # old style, no entry can mean -1
					s[word] = loc
			else:
				bug('weird word %s' % (word))
		else:
			bug('weird letter %s' % (word))

	return s, nest_level
	

with open(filename, "r+b") as f:
	root = {}
	inventory = []
	locker = []
	strongbox = []
	pet = {}
#	data = f.read();
	data = mmap.mmap(f.fileno(), 0)
	nest = []
	last_obj = None
	last_nest_level = 0


	while True:
		# read a section
		letter = read_letter()

		if letter == '*':
			read_eol()
			continue

		if letter != '#':
			bug('# not found')

		word = read_word()

		if word == 'PLAYER':
			ch, plr = read_char_section()
			root['character'] = ch
			root['player'] = plr
			root['version'] = pfile_version
		elif word == 'OBJECT' or word == 'O' or word == 'L' or word == 'B':
			obj, nest_level = read_obj()

			# new sections must start on nest 0
			# but an old section could end on nest > 0,
			# so wipe the list just in case
			if nest_level == 0:
				nest = []
				if word == 'OBJECT' or word == 'O':
					inventory.append(obj)
				elif word == 'L':
					locker.append(obj)
				elif word == 'B':
					strongbox.append(obj)
			elif nest_level > last_nest_level:
				nest.append(last_obj)
				last_obj['contains'] = [obj]
			elif nest_level < last_nest_level:
				nest = nest[:-1]
				nest[-1]['contains'].append(obj)
			else: # equal
				nest[-1]['contains'].append(obj)

			last_obj = obj
			last_nest_level = nest_level

		elif word == 'PET':
			break
#			read_pet_section()
		elif word == 'END':
			break
		else:
			bug('bad section word %s-' % (word))

	if len(inventory) > 0:
		root['inventory'] = inventory
	if len(locker) > 0:
		root['locker'] = locker
	if len(strongbox) > 0:
		root['strongbox'] = strongbox

	import json
	print json.dumps(root, sort_keys=True, indent=1, separators=(',', ': '))
