#pragma once

class Descriptor;
class Character;
class String;

void    close_socket(Descriptor *dclose);
void	cwtb(Descriptor *d, const String& txt);
void    write_to_buffer(Descriptor *d, const String& txt);
void    echo_off(Descriptor *d);
void    echo_on(Descriptor *d);

void goto_line(Character *ch, int row, int column);
void clear_window(Character *ch);
void set_window(Character *ch, int top, int bottom);
void reset_terminal(Character *ch);
