#ifndef __GAI_GNOME_CONFIG_H
#define __GAI_GNOME_CONFIG_H
/*#include "gaiconf.h"*/

#ifndef GAI_WITH_GNOME
/* the following functions are exported in our gnome lib replacements */

/* in gnome-config.c compiled via gai-gnome-config.c ... */
/*
#define gnome_config_sync           gai_gnome_config_sync
#define gnome_config_sync_file_     gai_gnome_config_sync_file_
#define gnome_config_clean_file_    gai_gnome_config_clean_file_
#define gnome_config_drop_file_     gai_gnome_config_drop_file_
#define gnome_config_init_iterator_ gai_gnome_config_init_iterator_
#define gnome_config_init_iterator_sections_ gai_gnome_config_init_iterator_sections_
#define gnome_config_iterator_next  gai_gnome_config_iterator_next
#define gnome_config_clean_section_ gai_gnome_config_clean_section_
#define gnome_config_clean_key_     gai_gnome_config_clean_key_
#define gnome_config_has_section_   gai_gnome_config_has_section_
#define gnome_config_drop_all       gai_gnome_config_drop_all
#define gnome_config_get_int_with_default_    gai_gnome_config_get_int_with_default_
#define gnome_config_get_float_with_default_  gai_gnome_config_get_float_with_default_
#define gnome_config_get_string_with_default_ gai_gnome_config_get_string_with_default_
#define gnome_config_get_bool_with_default_   gai_gnome_config_get_bool_with_default_
#define gnome_config_make_vector    gai_gnome_config_make_vector
#define gnome_config_get_vector_with_default_ gai_gnome_config_get_vector_with_default_
#define gnome_config_set_string_    gai_gnome_config_set_string_
#define gnome_config_set_int_       gai_gnome_config_set_int_
#define gnome_config_set_float_     gai_gnome_config_set_float_
#define gnome_config_set_bool_      gai_gnome_config_set_bool_
#define gnome_config_assemble_vector gai_gnome_config_assemble_vector
#define gnome_config_set_vector_    gai_gnome_config_set_vector_
#define gnome_config_push_prefix    gai_gnome_config_push_prefix
#define gnome_config_pop_prefix     gai_gnome_config_pop_prefix
#define gnome_config_set_set_handler gai_gnome_config_set_set_handler
#define gnome_config_set_sync_handler gai_gnome_config_set_sync_handler
*/
/* in gnome-util.c compiled via gai-gnome-util.c ... */
/*#define gnome_util_user_shell       gai_gnome_util_user_shell
#define g_extension_pointer         gai_g_extension_pointer
#define gnome_setenv                gai_gnome_setenv
#define gnome_unsetenv              gai_gnome_unsetenv
#define gnome_clearenv              gai_gnome_clearenv
*/
#endif

/* finally, declare the gnome-config.h stuff we want actually... */
#include "gnome-config.h"
#endif
