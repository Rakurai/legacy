#pragma once

int         translate_telopts        ( DESCRIPTOR_DATA *d, const unsigned char *src, int srclen, unsigned char *out, int outlen );
void        announce_support         ( DESCRIPTOR_DATA *d );
void        unannounce_support       ( DESCRIPTOR_DATA *d );
void        write_mccp2              ( DESCRIPTOR_DATA *d, const char *txt, int length );
void        send_echo_on             ( DESCRIPTOR_DATA *d );
void        send_echo_off            ( DESCRIPTOR_DATA *d );
