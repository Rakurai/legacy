#!/usr/bin/env python

import sys
import shlex
import sqlite3
import re

#create table mobs(vnum int, keywords text, race text, act text, aff text, align int, level int, hitroll int, hp int, mana int, damroll int, damtype text, acp int, acb int, acs int, acm int, off text, imm text, res text, vuln text, pos1 text, pos2 text, sex text, silver int, size text, area text, llimit int, glimit int);
def parse_dice(s, vnum):
  parts = filter(None, re.split(r'(\d+)', s))
  dice = sides = bonus = 0

  try:
    dice = int(parts[0])

    if (len(parts) > 1):
      sides = int(parts[2])

    if (len(parts) > 3):
      bonus = int(parts[4])

      if (parts[3] == '-'):
        bonus *= -1
  except:
    print >> sys.stderr, "invalid dice string %s on vnum %d" % (s, vnum)

  return dice * (sides + 1) + bonus

filename = sys.argv[1]

inserting = False
printing = False

if inserting:
  conn = sqlite3.connect('areas.db')
  c = conn.cursor()

with open(filename) as f:
  data = f.read()

data = data.split("#MOBILES", 1)

if len(data) != 2:
  exit() # no objects section

data = data[1].split("#0", 1)

if len(data) != 2:
  print >> sys.stderr, "no end to mobiles section!"

objs = data[0].replace('\n',' ').split("#")[1:]

data = data[1].split("#RESETS")

if len(data) != 2:
  exit() # no resets section

lines = data[1].split('\n')

glimits = {}
llimits = {}

# parse resets first, build dict
for line in lines:
#  print line
  parts = filter(None, line.strip().split())

  if len(parts) == 0:
    continue

  if parts[0] == 'S':
    break;

  if parts[0] == 'M':
    mvnum = int(parts[2])
    glimit = int(parts[3])
    rvnum = int(parts[4])
    llimit = int(parts[5])

    glimits[mvnum] = glimit
    llimits[mvnum] = llimit

#print glimits
#print llimits
#exit()

for obj in objs:
  rstring = obj + ' 0' # pad the end so splits don't fail
  vnum, rstring = rstring.split(None, 1)
  vnum = int(vnum)

  try:
    keywords, shortdesc, longdesc, exdesc, race, rstring = rstring.split('~', 5)
    actflags, affflags, align, _, rstring = rstring.split(None, 4)
    level, hitroll, hitpoints, mana, damroll, damtype, rstring = rstring.split(None, 6)

    acp, acb, acs, acm, rstring = rstring.split(None, 4)
    offflags, immflags, resflags, vulnflags, rstring = rstring.split(None, 4)
    pos1, pos2, sex, silver, rstring = rstring.split(None, 4)
    _, _, size, _, rstring = rstring.split(None, 4)
  except:
    print >> sys.stderr, "weird mob vnum", vnum

  hitpoints = parse_dice(hitpoints, vnum)
  mana = parse_dice(mana, vnum)
  damroll = parse_dice(damroll, vnum)

#  lex = shlex.shlex(rstring, posix=True)
#  lex.whitespace_split = True
#  lex.commenters = ''
#  values = [lex.next(), lex.next(), lex.next(), lex.next(), lex.next()]
#  rstring = lex.instream.read()
  if vnum not in glimits.keys():
    print >> sys.stderr, "vnum %d doesn't have a reset" % (vnum)
    glimit = 0
    llimit = 0
  else:
    glimit = glimits[vnum]
    llimit = llimits[vnum]
 
  istr = "INSERT INTO mobs VALUES ({0},'{1}','{2}','{3}','{4}',{5},{6},{7},{8},{9},{10},'{11}',{12},{13},{14},{15},'{16}','{17}','{18}','{19}','{20}','{21}','{22}',{23},'{24}','{25}',{26},{27})".format(
    vnum,
    keywords.strip().replace("'",""),
    race.strip().replace("'",""),
    actflags, affflags, align,
    level, hitroll, hitpoints, mana, damroll, damtype,
    acp, acb, acs, acm,
    offflags, immflags, resflags, vulnflags,
    pos1, pos2, sex, silver, size,
    filename.split('/')[-1], llimit, glimit)

  if inserting:
#    try:
      c.execute(istr)
#    except:
#      print >> sys.stderr, "problem inserting with vnum", vnum
  if printing:
    print istr

if inserting:
  conn.commit()
  conn.close()
