#pragma once

/* random.c */
int	number_fuzzy		args((int number));
int	number_range		args((int from, int to));
int	number_percent		args((void));
int	number_door		args((void));
int	number_bits		args((int width));
long	number_mm		args((void));
int	dice			args((int number, int size));
bool prd_chance      args(( int *prev_fails, int percent ));
