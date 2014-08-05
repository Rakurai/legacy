/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include "merc.h"


#if !defined(OLD_RAND)
void srandom(unsigned int);
int getpid();
time_t time(time_t *tloc);
#endif


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


/* Stick a little fuzz on a number. */
int number_fuzzy(int number)
{
	switch (number_bits(2)) {
	case 0: number -= 1;    break;

	case 3: number += 1;    break;
	}

	return UMAX(1, number);
}


/* Generate a random number. */
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



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you,
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2 + 55];
#endif

void init_mm()
{
#if defined (OLD_RAND)
	int *piState;
	int iState;
	piState     = &rgiState[2];
	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;
	piState[0]  = ((int) current_time) & ((1 << 30) - 1);
	piState[1]  = 1;

	for (iState = 2; iState < 55; iState++) {
		piState[iState] = (piState[iState - 1] + piState[iState - 2])
		                  & ((1 << 30) - 1);
	}

#else
	srandom(time(NULL)^getpid());
#endif
	return;
}


long number_mm(void)
{
#if defined (OLD_RAND)
	int *piState;
	int iState1;
	int iState2;
	int iRand;
	piState             = &rgiState[2];
	iState1             = piState[-2];
	iState2             = piState[-1];
	iRand               = (piState[iState1] + piState[iState2])
	                      & ((1 << 30) - 1);
	piState[iState1]    = iRand;

	if (++iState1 == 55)
		iState1 = 0;

	if (++iState2 == 55)
		iState2 = 0;

	piState[-2]         = iState1;
	piState[-1]         = iState2;
	return iRand >> 6;
#else
	return random() >> 6;
#endif
}
