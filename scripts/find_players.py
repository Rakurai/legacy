#!/usr/bin/env python

import os

new_path='player'
old_path='../legacy_backup/player'

current_players = [x for x in os.listdir(new_path) if os.path.isfile(new_path+'/'+x)]
old_players = [x for x in os.listdir(old_path) if os.path.isfile(old_path+'/'+x)]

for x in old_players:
	if x not in current_players:
		print x

#for player in current_players:
#	print player
