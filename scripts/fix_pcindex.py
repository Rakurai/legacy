import mudstr

pmap = {}
f = open('pcindex/cgroup')
for line in f:
  name, flags = line.split()
  pmap[name] = {}
  pmap[name]['cgroup'] = flags
f.close()

f = open('pcindex/act')
for line in f:
  name, flags = line.split()
  if name not in pmap:
    pmap[name] = {}
    pmap[name]['cgroup'] = 'A'
  pmap[name]['act'] = flags
f.close()

for name in pmap:
  player = pmap[name]
#  print player
  cgroup = mudstr.parse_flags(player['cgroup'])
  act = int(player['act'])
  N = mudstr.parse_flags('N')
  O = mudstr.parse_flags('O')
  e = mudstr.parse_flags('e')
#  print act
  if act&N > 0:
#    print 'found deputy'
    cgroup |= N
  if act & e > 0:
#    print 'found leader'
    cgroup |= O

  print "update newlegacy.pc_index set act=%d where name='%s';" % (cgroup, name)
