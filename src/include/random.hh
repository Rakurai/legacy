#pragma once

/* random.c */
int	number_fuzzy		(int number);
int	number_range		(int from, int to);
int	number_percent		(void);
int	number_door		(void);
int	number_bits		(int width);
long	number_mm		(void);
int	dice			(int number, int size);
bool roll_chance          (int percent);
bool prd_chance      ( int *prev_fails, int percent );
