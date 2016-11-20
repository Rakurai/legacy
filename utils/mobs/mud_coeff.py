#!/usr/bin/env python

# print mud-wide and area constants for a generating stats based on level,
# in the form of coefficients to a 2nd degree polynomial equation

import sys
import sqlite3
import numpy as np
import mob_fn

def calc_coeffs(mobs):
  level_array = [mob.level for mob in mobs]

  return {
    'hitroll':[x for x in np.polyfit(level_array, [mob.hitroll for mob in mobs], 2)],
    'damroll':[x for x in np.polyfit(level_array, [mob.damroll for mob in mobs], 2)],
    'hp':[x for x in np.polyfit(level_array, [mob.hp for mob in mobs], 2)],
    'mana':[x for x in np.polyfit(level_array, [mob.mana for mob in mobs], 2)],
    'ac':[x for x in np.polyfit(level_array, [mob.ac for mob in mobs], 2)]
  }

conn = sqlite3.connect('areas.db')
c = conn.cursor()
c.execute("PRAGMA case_sensitive_like=ON") # for flag fields

c.execute("SELECT DISTINCT area FROM mobs")
areas = []
for row in c:
  areas.append(row[0])

all_mobs = []
coeffs = {}

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

#condition_args = ('creselon.are','vegita.are')
condition_args = ()
c.execute("SELECT " +fields+ " FROM mobs WHERE "+conditions+" ORDER BY level", condition_args)

for row in c:
  glimit = row[3]
  for i in range(glimit):
    all_mobs.append(mob_fn.Mob(
      row[0], # area
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
coeffs['mud'] = {'inside': calc_coeffs(all_mobs)}

import warnings
warnings.simplefilter('ignore', np.RankWarning)

for area in areas:
  inside_mobs = []
  outside_mobs = []

  for mob in all_mobs:
    if mob.area == area:
      inside_mobs.append(mob)
    else:
      outside_mobs.append(mob)

  if len(inside_mobs) == 0:
    continue

  coeffs[area] = {
    'inside': calc_coeffs(inside_mobs),
    'outside': calc_coeffs(outside_mobs)
  }

import json
print json.dumps(coeffs, sort_keys=True, indent=1, separators=(',', ': '))
