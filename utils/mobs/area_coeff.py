#!/usr/bin/env python

import sys
import shlex
import sqlite3
import re
import mob

conn = sqlite3.connect('areas.db')
c = conn.cursor()
c.execute("PRAGMA case_sensitive_like=ON") # for flag fields

c.execute("SELECT DISTINCT area FROM mobs")
areas = []
for row in c:
  areas.append(row[0])

conn.close()

avgstats = mob.AverageStats('avg_stats.csv')
print avgstats.hitroll(1)
print avgstats.damroll(1)
print avgstats.hp(1)
print avgstats.mana(1)
print avgstats.ac(1)
