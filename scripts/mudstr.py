def parse_flags(str):
  bits = 0
  bitv = 1
  for i in range(26):
    if chr(ord('A')+i) in str:
      bits |= bitv
    bitv *= 2
  for i in range(6):
    if chr(ord('a')+i) in str:
      bits |= bitv
    bitv *= 2
  return bits

if __name__ == "__main__":
  import sys
  if sys.argv[1] == 'parse_flags':
    print parse_flags(sys.argv[2])
  else:
    print 'usage: %s parse_flags <str>' % sys.argv[0]
