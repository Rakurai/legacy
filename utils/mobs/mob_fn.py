class AverageStats:
	stats = {}

	def __init__(self, filename):
		with open(filename) as f:
			for line in f.readlines():
				parts = line.strip().split(',')
				if parts[0] == '':
					fields = parts[0:]
					continue

				var = parts[0]
				self.stats[var] = {}

				for index in range(1, 6):
					self.stats[var][fields[index]] = float(parts[index])
		print self.stats
	def _average(self, level, place):
		coeff = self.stats['coeff'][place]
		exp = self.stats['exp'][place]
		vshift = self.stats['vshift'][place]
		hshift = self.stats['hshift'][place]
		return exp * (level + hshift) ** exp + vshift
	def hitroll(self, level):
		return self._average(level, 'hitroll')
	def damroll(self, level):
		return self._average(level, 'damroll')
	def hp(self, level):
		return self._average(level, 'hp')
	def mana(self, level):
		return self._average(level, 'mana')
	def ac(self, level):
		return self._average(level, 'ac')

class Mob:
	def __init__(self, area, level, keywords, act, off, imm, res, vuln, hitroll, damroll, hp, mana, ac, size):
		self.area = area
		self.level = level
		self.keywords = keywords
		self.act = act
		self.off = off
		self.imm = imm
		self.res = res
		self.vuln = vuln
		self.hitroll = hitroll
		self.damroll = damroll
		self.hp = hp
		self.mana = mana
		self.ac = ac
		self.size = size

	danger_weights = {
		'act_aggr':		1.0,
		'act_cleric':	-0.5, # thac0_32 = 2
		'act_mage':		-1.0, # thac0_32 = 6
		'act_thief':	0.0, # thac0_32 = -4
		'act_warrior':	1.0, # thac0_32 = -10
		'off_area':		3.0, # hits everything in the room once every round
		'off_backstab':	1.0,
		'off_bash':		3.0,
		'off_berserk':	2.0,
		'off_fast':		3.0, # check
		'off_kick':		1.0,
		'off_dirt':		1.0,
		'off_trip':		2.0,
		'off_crush':	1.0,
	}

	toughness_weights = {
		'act_undead':	0.5, # just prevents calm, plague, sleep, power word
		'off_disarm':	2.0,
		'off_dodge':	3.0,
		'off_parry':	3.0,
		'off_fast':		2.0, # check
		'imm_weapon':	2.0, # immune per weapon type slash/bash/pierce (multiplied)
		'imm_magic':	3.0, # immune to all magic
	}

	def danger(self, weights, avg_stats): # -1 to 1, danger compared to average mob
		danger_sum = 0
		# act mage, thief, 


		# act flags
		if 'F' in self.act: danger_sum += weights['act_aggr']
		elif 'M' in self.act: danger_sum += weights['act_aggr']/2 #aggress_align
		if 'Q' in self.act: danger_sum += weights['act_cleric']
		if 'R' in self.act: danger_sum += weights['act_mage']
		if 'S' in self.act: danger_sum += weights['act_thief']
		if 'T' in self.act: danger_sum += weights['act_warrior']

		# off flags
		if 'F' in self.act: danger_sum += weights['act_aggr']
	def toughness(self, avg_stats): # -1 to 1, toughness compared to average mob
		pass

def mob_average(mob_list):
	# sum
	sums = [{'count':0,'hitroll':0.0,'damroll':0.0,'hp':0.0,'mana':0.0,'ac':0.0} for x in range(131)]

	for mob in mob_list:
		sums[mob.level]['count'] += 1
		sums[mob.level]['hitroll'] += mob.hitroll
		sums[mob.level]['damroll'] += mob.damroll
		sums[mob.level]['hp'] += mob.hp
		sums[mob.level]['mana'] += mob.mana
		sums[mob.level]['ac'] += mob.ac

	# average
	avgs = []

	for level in range(0, 131):
		count = sums[level]['count']
		avgs.append({'level':level, 'count':count})

		for field in ('hitroll', 'damroll', 'hp', 'mana', 'ac'):
			avgs[level][field] = (sums[level][field]/count if count > 0 else 0.0)

	# interpolate
	for level in range(1, 131):
		if avgs[level]['count'] == 0:
			f = level-1
			t = level+1

			while avgs[t]['count'] == 0:
				t += 1

			for field in ('hitroll', 'damroll', 'hp', 'mana', 'ac'):
				avgs[level][field] = avgs[f][field] + (avgs[t][field]-avgs[f][field])/(t-f)

	return avgs
