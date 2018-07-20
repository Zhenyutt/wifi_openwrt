#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "lib_aes_pbkdf2.h"

/********************************
 * Global variable
 ********************************/
static int _debug_level = 0;


/********************************
 * wifi debug level function
 ********************************/
void set_debug_level(int level)
{
	_debug_level = level;
}


int get_debug_level()
{
	return _debug_level;
}



/********************************
 * common helper function
 ********************************/
int do_system(char *format, ...)
{
	char command[1024] = {0};

	va_list args;
	va_start(args, format);
	vsnprintf(command, sizeof(command), format, args);
	va_end(args);

	int ret = system(command);
	int debug_level = get_debug_level();
	if(debug_level > 0) {
		fprintf(stderr, "%d == system(%s)\n", ret, command);
	} else if(debug_level > 1) {
		FILE *kfp = fopen("/dev/kmsg", "w");
		if(kfp) {
			fprintf(kfp, "wifi: system(%s) == %d\n", command, ret);
			fclose(kfp);
		}
	}
	return ret;
}


char *string_replace(char *string, char *old, char *new, unsigned int max_replace_times)
{
	if(string == NULL) {
		return NULL;
	}

	unsigned int string_len = strlen(string);
	unsigned int newstring_size = string_len + 1;
	unsigned int len = 0; // for newstring

	//create/malloc a new string
	char *newstring = calloc(1, newstring_size);
	if(newstring == NULL) {
		return NULL;
	}

	// check old and new
	unsigned int old_len = 0;
	if(old) {
		old_len = strlen(old);
	}
	unsigned int new_len = 0;
	if(new) {
		new_len = strlen(new);
	}

	if(old == NULL || old_len == 0 || new == NULL) {
		//nothing to replace
		strncpy(newstring, string, newstring_size - 1);
		return newstring;
	}

	//replace times
	unsigned int replace_times = 0;
	if(max_replace_times == 0) {
		//unlimit mode
		max_replace_times = string_len;
	}

	char *p = string;
	char *match = NULL;
	do {
		//check replace_times
		if(replace_times >= max_replace_times) {
			//reach the max replace times
			len += snprintf(newstring + len, newstring_size - len, "%s", p);
			break;
		}

		match = strstr(p, old);
		if(match) {
			//need to replace
			if(len + (match - p) + new_len >= newstring_size) {
				//need to larger the size of newstring
				newstring_size = newstring_size + (match - p) +  new_len;
				char * temp = realloc(newstring, newstring_size);
				if(temp) {
					newstring = temp;
				} else {
					fprintf(stderr, "%s():%d: error: realloc newstring fail\n", __FUNCTION__, __LINE__);
					free(newstring);
					return NULL;
				}
			}
			len += snprintf(newstring + len, newstring_size - len, "%.*s%s", (int)(match - p), p, new);
			p = match + old_len;
			replace_times++;
		} else {
			//nothing to replace, just copy left data
			len += snprintf(newstring + len, newstring_size - len, "%s", p);
			break;
		}
	} while(1);

	return newstring;
}


int string_replace_to_buffer(char *string, char *old, char *new, unsigned int max_replace_times, char *buffer, unsigned int buffer_size)
{
	char *p = string_replace(string, old, new, max_replace_times);
	if(p == NULL) {
		//replace fail
		return -1;
	}

	int ret = 0;
	if(strlen(p) < buffer_size) {
		strncpy(buffer, p, buffer_size - 1);
		ret = 0;
	} else {
		ret = -1;
	}

	free(p);
	return ret;
}


char *gmtk_decrypt_helper(char *enc_text, char *plain_text, unsigned int plain_size)
{
	char *output = NULL;
	if(gemtek_aes_decrypt(enc_text, &output) == 0) {
		snprintf(plain_text, plain_size, "%s", output);
		free(output);
	} else {
		snprintf(plain_text, plain_size, "%s", enc_text);
	}

	return plain_text;
}


char *strtoupper(char *str, size_t str_len)
{
	int i = 0;

	for(i = 0; i < str_len; i++) {
		str[i] = toupper(str[i]);
	}

	return str;
}


/*************************************************
 * copy content from lib/converters.h and lib/utf8.h (libiconv)
 *************************************************/
/* Return code if invalid input after a shift sequence of n bytes was read.
   (xxx_mbtowc) */
#define RET_SHIFT_ILSEQ(n)  (-1-2*(n))
/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ           RET_SHIFT_ILSEQ(0)
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)       (-2-2*(n))

/* Our own notion of wide character, as UCS-4, according to ISO-10646-1. */
typedef unsigned int ucs4_t;

/* Specification: RFC 3629 */
//static int utf8_mbtowc (conv_t conv, ucs4_t *pwc, const unsigned char *s, int n)
inline static int utf8_mbtowc(ucs4_t *pwc, const unsigned char *s, int n)
{
	unsigned char c = s[0];

	if(c < 0x80) {
		*pwc = c;
		return 1;
	} else if(c < 0xc2) {
		return RET_ILSEQ;
	} else if(c < 0xe0) {
		if(n < 2)
			return RET_TOOFEW(0);
		if(!((s[1] ^ 0x80) < 0x40))
			return RET_ILSEQ;
		*pwc = ((ucs4_t)(c & 0x1f) << 6)
		       | (ucs4_t)(s[1] ^ 0x80);
		return 2;
	} else if(c < 0xf0) {
		if(n < 3)
			return RET_TOOFEW(0);
		if(!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
		     && (c >= 0xe1 || s[1] >= 0xa0)))
			return RET_ILSEQ;
		*pwc = ((ucs4_t)(c & 0x0f) << 12)
		       | ((ucs4_t)(s[1] ^ 0x80) << 6)
		       | (ucs4_t)(s[2] ^ 0x80);
		return 3;
	} else if(c < 0xf8 && sizeof(ucs4_t) * 8 >= 32) {
		if(n < 4)
			return RET_TOOFEW(0);
		if(!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
		     && (s[3] ^ 0x80) < 0x40
		     && (c >= 0xf1 || s[1] >= 0x90)))
			return RET_ILSEQ;
		*pwc = ((ucs4_t)(c & 0x07) << 18)
		       | ((ucs4_t)(s[1] ^ 0x80) << 12)
		       | ((ucs4_t)(s[2] ^ 0x80) << 6)
		       | (ucs4_t)(s[3] ^ 0x80);
		return 4;
	} else if(c < 0xfc && sizeof(ucs4_t) * 8 >= 32) {
		if(n < 5)
			return RET_TOOFEW(0);
		if(!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
		     && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
		     && (c >= 0xf9 || s[1] >= 0x88)))
			return RET_ILSEQ;
		*pwc = ((ucs4_t)(c & 0x03) << 24)
		       | ((ucs4_t)(s[1] ^ 0x80) << 18)
		       | ((ucs4_t)(s[2] ^ 0x80) << 12)
		       | ((ucs4_t)(s[3] ^ 0x80) << 6)
		       | (ucs4_t)(s[4] ^ 0x80);
		return 5;
	} else if(c < 0xfe && sizeof(ucs4_t) * 8 >= 32) {
		if(n < 6)
			return RET_TOOFEW(0);
		if(!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
		     && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
		     && (s[5] ^ 0x80) < 0x40
		     && (c >= 0xfd || s[1] >= 0x84)))
			return RET_ILSEQ;
		*pwc = ((ucs4_t)(c & 0x01) << 30)
		       | ((ucs4_t)(s[1] ^ 0x80) << 24)
		       | ((ucs4_t)(s[2] ^ 0x80) << 18)
		       | ((ucs4_t)(s[3] ^ 0x80) << 12)
		       | ((ucs4_t)(s[4] ^ 0x80) << 6)
		       | (ucs4_t)(s[5] ^ 0x80);
		return 6;
	} else
		return RET_ILSEQ;
}


int utf8str_to_utf8(char *utf8str, char *utf8)
{
	int i = 0;

	while(*utf8str != '\0') {
		if(*(utf8str + 0) == '\\' && *(utf8str + 1) == 'x' && isxdigit(*(utf8str + 2)) && isxdigit(*(utf8str + 3))) {
			char hex[3] = {0};
			strncpy(hex, utf8str + 2, 2);
			utf8[i++] = strtoul(hex, NULL, 16);
			utf8str += 4;
		} else {
			return -1;
		}
	}

	return 0;
}


//char *utf8_to_json_string(char *output, unsigned int output_size, const char *utf8_str)
int utf8_to_ucs2hex(char *utf8, char *ucs2hex, int ucs2hex_size)
{
	if(ucs2hex == NULL || ucs2hex_size == 0) {
		return -1;
	}

	unsigned int ucs2hex_len = 0;
	unsigned int utf8_str_len = strlen(utf8);

	while(*utf8 != '\0' && utf8_str_len > 0) {
		unsigned int code_point = 0;
		int utf8_str_byte = utf8_mbtowc(&code_point, (unsigned char *)utf8, utf8_str_len);
		if(utf8_str_byte == 1) {
			//ascii
			ucs2hex_len += snprintf(ucs2hex + ucs2hex_len, ucs2hex_size - ucs2hex_len, "%c", code_point);
		} else if(utf8_str_byte > 1) {
			//utf8 char, trans it's codepoint to "\uXXXX" format
			ucs2hex_len += snprintf(ucs2hex + ucs2hex_len, ucs2hex_size - ucs2hex_len, "\\u%02X", code_point);
		} else {
			//error
			break;
		}

		//next...
		utf8 += utf8_str_byte;
		utf8_str_len -= utf8_str_byte;
	}

	return 0;
}
