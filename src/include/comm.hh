#pragma once

class Descriptor;
class Character;
class String;

void    close_socket    args( ( Descriptor *dclose ) );
void	cwtb		args((Descriptor *d, const String& txt));
void    write_to_buffer args( ( Descriptor *d, const String& txt ) );
void    echo_off args((Descriptor *));
void    echo_on  args((Descriptor *));

void goto_line(Character *ch, int row, int column);
void clear_window(Character *ch);
void set_window(Character *ch, int top, int bottom);
void reset_terminal(Character *ch);
