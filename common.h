#ifndef _COMMON_H
#define _COMMON_H


#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define array_for_each_entry_size(entry, array, array_size) \
	for (entry = array;entry<array+array_size;entry++)
#define array_for_each_entry(entry, array) array_for_each_entry_size(entry, array, ARRAY_SIZE(array))



void set_debug_level(int level);
unsigned int get_debug_level();

int do_system(char *format, ...);
int string_replace_to_buffer(char *string, char *old, char *new, unsigned int max_replace_times, char *buffer, unsigned int buffer_size);

char *gmtk_decrypt_helper(char *enc_text, char *plain_text, unsigned int plain_size);

char *strtoupper(char *str, size_t str_len);

int utf8str_to_utf8(char *utf8str, char *utf8);
int utf8_to_ucs2hex(char *utf8, char *ucs2hex, int ucs2hex_size);

#endif //_COMMON_H
