#!/usr/bin/env python
import math
import random

def PfromC(C):
	if not isinstance(C, float):
		C = float(C)

	pProcOnN = 0.0
	pProcByN = 0.0
	sumNpProcOnN = 0.0

	maxFails = int(math.ceil(1.0 / C))

	for N in range(1, maxFails + 1):
		pProcOnN = min(1.0, N * C) * (1.0 - pProcByN)
		pProcByN += pProcOnN
		sumNpProcOnN += N * pProcOnN

	return (1.0 / sumNpProcOnN)

def CfromP(p):
	if not isinstance(p, float):
		p = float(p) # double precision

	Cupper = p
	Clower = 0.0
	Cmid = 0.0
	p2 = 1.0

	while(True):
		Cmid = (Cupper + Clower) / 2.0
		p1 = PfromC(Cmid)

		if math.fabs(p1 - p2) <= 0:
			break

		if p1 > p:
			Cupper = Cmid
		else:
			Clower = Cmid

		p2 = p1

	return Cmid

def testPfromC(C, iterations):
	if not isinstance(C, float):
		C = float(C)

	nsum = 0

	for i in range(iterations):
		n = 1

		while (C * n) < 1.0 and random.random() > (C * n):
			n += 1

		nsum += n

	av_iters = float(nsum) / iterations
	return 100.0 / av_iters

if __name__ == "__main__":
	iterations = 100000

	for i in range(1, 100):
		p = i / 100.0
		C = CfromP(p)
		max_n = math.ceil(1/C)
		avg_n = 1.0/p
		print "\t %0.6f,   // %2d %5d  %-3.2f" % (C, i, max_n, avg_n)
#		print "%.2f\t%f\t%f" % (p, C, testPfromC(C, iterations))
