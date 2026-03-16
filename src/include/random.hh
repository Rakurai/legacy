#pragma once

/* random.c */
int	number_fuzzy		(int number);
int	number_range		(int from, int to);
int	number_percent		();
int	number_door		();
int	number_bits		(int width);
long	number_mm		();
int	dice			(int number, int size);
bool roll_chance          (int percent);
bool prd_chance      ( int *prev_fails, int percent );
