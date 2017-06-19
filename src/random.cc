/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include <cstdlib>
#include <time.h>

#include "macros.hh"

void init_mm()
{
	srand(time(nullptr));
	return;
}

long number_mm(void)
{
	return rand() >> 6;
}

/* Generate a random number. */
// not inclusive! [from, to)
int number_range(int from, int to)
{
	int power, number;

	if (from == 0 && to == 0)
		return 0;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2; power < to; power <<= 1);

	while ((number = number_mm() & (power - 1)) >= to);

	return from + number;
}

/* Roll some dice. */
int dice(int number, int size)
{
	int idice, sum;

	switch (size) {
	case 0: return 0;

	case 1: return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++)
		sum += number_range(1, size);

	return sum;
}

/* Generate a percentile roll. */
int number_percent(void)
{
	int percent;

	while ((percent = number_mm() & (128 - 1)) > 99);

	return 1 + percent;
}

/* Generate a random door. */
int number_door(void)
{
	int door;

	while ((door = number_mm() & (8 - 1)) > 5);

	return door;
}

int number_bits(int width)
{
	return number_mm() & ((1 << width) - 1);
}

/* Stick a little fuzz on a number. */
int number_fuzzy(int number)
{
	switch (number_bits(2)) {
	case 0: number -= 1;    break;

	case 3: number += 1;    break;
	}

	return UMAX(1, number);
}


/* Pseudo-random distribution for percentage chance rolls obvious to the
   player, based on the DOTA2 system at:

   http://dota2.gamepedia.com/Pseudo-random_distribution

   The idea here is that players notice streaks of bad luck more than they
   notice streaks of good luck (confirmation bias).  For example, the chance
   of failing a 75% skill 3 times in a row is 1/4*1/4*1/4 = 1/64.  But,
   players notice that failure but don't notice a streak of successes, because
   success is expected with 75%.

   The goal with this work is to generate random numbers from a distribution
   based on past success or failure.  The probability of success for a skill
   with rate r is C(r)*n, where n is the number of attempts.  For example, a
   player's first attempt at a 75% skill (C = ~0.667) is C*1, or 66.7%.  If
   the player fails, the next attempt is 133.7%, or automatic success.  The 
   player will never fail that skill more than once in a row.  If r = 50% and
   C = 0.302, the first attempt is 30%, second is 60%, third is 90%, and success
   is guaranteed on the 4th attempt.  The overall effect is to limit the failures
   and reduce the variance on the number of attempts for a success.

   The table below lists values of C generated for a particular whole number
   percentile value.  The code used is in calcC.py.  To use it, we store a
   variable with the player, their failure attempts, and use that to roll a
   percentage.  Keep in mind this should only be used with things that are
   obvious to the player, such as spell/skill procs.  If players complain about
   'first time' shots being low chance, we can just increase
                                                      -- Montrey 2016
*/

struct prd_table_entry {
	int p;
	float c;
};

const float prd_table[] = {
//          C,      p, max_n, avg_n
	 0.000000,   //  0   inf  inf
	 0.000156,   //  1  6409  100.00
	 0.000620,   //  2  1613  50.00
	 0.001386,   //  3   722  33.33
	 0.002449,   //  4   409  25.00
	 0.003802,   //  5   264  20.00
	 0.005440,   //  6   184  16.67
	 0.007359,   //  7   136  14.29
	 0.009552,   //  8   105  12.50
	 0.012016,   //  9    84  11.11
	 0.014746,   // 10    68  10.00
	 0.017736,   // 11    57  9.09
	 0.020983,   // 12    48  8.33
	 0.024482,   // 13    41  7.69
	 0.028230,   // 14    36  7.14
	 0.032221,   // 15    32  6.67
	 0.036452,   // 16    28  6.25
	 0.040920,   // 17    25  5.88
	 0.045620,   // 18    22  5.56
	 0.050549,   // 19    20  5.26
	 0.055704,   // 20    18  5.00
	 0.061081,   // 21    17  4.76
	 0.066676,   // 22    15  4.55
	 0.072488,   // 23    14  4.35
	 0.078511,   // 24    13  4.17
	 0.084744,   // 25    12  4.00
	 0.091183,   // 26    11  3.85
	 0.097826,   // 27    11  3.70
	 0.104670,   // 28    10  3.57
	 0.111712,   // 29     9  3.45
	 0.118949,   // 30     9  3.33
	 0.126379,   // 31     8  3.23
	 0.134001,   // 32     8  3.12
	 0.141805,   // 33     8  3.03
	 0.149810,   // 34     7  2.94
	 0.157983,   // 35     7  2.86
	 0.166329,   // 36     7  2.78
	 0.174909,   // 37     6  2.70
	 0.183625,   // 38     6  2.63
	 0.192486,   // 39     6  2.56
	 0.201547,   // 40     5  2.50
	 0.210920,   // 41     5  2.44
	 0.220365,   // 42     5  2.38
	 0.229899,   // 43     5  2.33
	 0.239540,   // 44     5  2.27
	 0.249307,   // 45     5  2.22
	 0.259872,   // 46     4  2.17
	 0.270453,   // 47     4  2.13
	 0.281008,   // 48     4  2.08
	 0.291552,   // 49     4  2.04
	 0.302103,   // 50     4  2.00
	 0.312677,   // 51     4  1.96
	 0.323291,   // 52     4  1.92
	 0.334120,   // 53     3  1.89
	 0.347370,   // 54     3  1.85
	 0.360398,   // 55     3  1.82
	 0.373217,   // 56     3  1.79
	 0.385840,   // 57     3  1.75
	 0.398278,   // 58     3  1.72
	 0.410545,   // 59     3  1.69
	 0.422650,   // 60     3  1.67
	 0.434604,   // 61     3  1.64
	 0.446419,   // 62     3  1.61
	 0.458104,   // 63     3  1.59
	 0.469670,   // 64     3  1.56
	 0.481125,   // 65     3  1.54
	 0.492481,   // 66     3  1.52
	 0.507463,   // 67     2  1.49
	 0.529412,   // 68     2  1.47
	 0.550725,   // 69     2  1.45
	 0.571429,   // 70     2  1.43
	 0.591549,   // 71     2  1.41
	 0.611111,   // 72     2  1.39
	 0.630137,   // 73     2  1.37
	 0.648649,   // 74     2  1.35
	 0.666667,   // 75     2  1.33
	 0.684211,   // 76     2  1.32
	 0.701299,   // 77     2  1.30
	 0.717949,   // 78     2  1.28
	 0.734177,   // 79     2  1.27
	 0.750000,   // 80     2  1.25
	 0.765432,   // 81     2  1.23
	 0.780488,   // 82     2  1.22
	 0.795181,   // 83     2  1.20
	 0.809524,   // 84     2  1.19
	 0.823529,   // 85     2  1.18
	 0.837209,   // 86     2  1.16
	 0.850575,   // 87     2  1.15
	 0.863636,   // 88     2  1.14
	 0.876404,   // 89     2  1.12
	 0.888889,   // 90     2  1.11
	 0.901099,   // 91     2  1.10
	 0.913043,   // 92     2  1.09
	 0.924731,   // 93     2  1.08
	 0.936170,   // 94     2  1.06
	 0.947368,   // 95     2  1.05
	 0.958333,   // 96     2  1.04
	 0.969072,   // 97     2  1.03
	 0.979592,   // 98     2  1.02
	 0.989899,   // 99     2  1.01
	 1.000000    //100     1  1.00
};

bool prd_chance(int *prev_fails, int p) {
	p = URANGE(0, p, 100);

	// number_percent is [1,100], calculation is [0,100]
	float cp = prd_table[p] * ((*prev_fails) + 1);
	bool success = (number_percent()-1) < (cp * 100);

	if (success)
		*prev_fails = 0;
	else
		(*prev_fails)++;

	return success;
}
