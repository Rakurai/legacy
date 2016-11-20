#!/usr/bin/env python

import sys
import shlex
import sqlite3
import re

conn = sqlite3.connect('areas.db')
c = conn.cursor()
c.execute("PRAGMA case_sensitive_like=ON") # for flag fields

fields = 'area, keywords, vnum, glimit, act, level, hitroll, damroll, hp, mana, acp, acb, acs, acm, off, imm, res, vuln, size'
conditions = " area !='vegita.are'"
conditions += " AND area!='eilyndrae.are'"
conditions += " AND area!='immort.are'"
conditions += " AND area!='player.are'"
conditions += " AND area!='limbo.are'"
conditions += " AND area!='torayna.are'"
conditions += " AND area!='arena.are'"
conditions += " AND area!='flaming.are'"
conditions += " AND glimit > 0"
conditions += " AND not act like '%I%'" # act_pet
conditions += " AND not act like '%J%'" # act_train
conditions += " AND not act like '%K%'" # act_practice
conditions += " AND not act like '%V%'" # act_nopurge
conditions += " AND not act like '%a%'" # act_healer
conditions += " AND not act like '%b%'" # act_gain
conditions += " AND not act like '%d%'" # act_changer
conditions += " AND not imm like '%C%D%'" # imm to weapon and magic
conditions += " AND not imm like '%D%C%'" # imm to magic and weapon

import mob
mobs = []


#condition_args = ('creselon.are','vegita.are')
condition_args = ()
c.execute("SELECT " +fields+ " FROM mobs WHERE "+conditions+" ORDER BY level", condition_args)

import mob
mobs = []

for row in c:
  glimit = row[3]
  for i in range(glimit):
    mobs.append(mob.Mob(
      row[5], # level
      row[1], # keywords
      row[4], # act
      row[14], # off
      row[15], # imm
      row[16], # res
      row[17], # vuln
      row[6], # hitroll
      row[7], # damroll
      row[8], # hp
      row[9], # mana
      (row[10]+row[11]+row[12]+row[13])/4, # ac
      row[18] # size
    ));


conn.close()


for level in range(0, 131):
  count = avgs[level]['count']
  print '%d\t%d\t%f\t%f\t%f\t%f\t%f' % (
    level,
    count,
    avgs[level]['hitroll'],
    avgs[level]['damroll'],
    avgs[level]['hp'],
    avgs[level]['mana'],
    avgs[level]['ac']
  )

