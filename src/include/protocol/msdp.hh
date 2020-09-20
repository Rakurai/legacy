#pragma once

void        init_msdp_table               ( void );
void        process_msdp_varval           ( DESCRIPTOR_DATA *d, const char *var, const char *val );
void        msdp_send_update              ( DESCRIPTOR_DATA *d );
void        msdp_update_var               ( DESCRIPTOR_DATA *d, const char *var, const char *fmt, ... );
void        msdp_update_var_instant       ( DESCRIPTOR_DATA *d, const char *var, const char *fmt, ... );

void        msdp_configure_arachnos       ( DESCRIPTOR_DATA *d, int index );
void        msdp_configure_pluginid       ( DESCRIPTOR_DATA *d, int index );

int         json2msdp                     ( const unsigned char *src, int srclen, char *out );
