#ifndef __vlg_util_h
#define __vlg_util_h

size_t vlg_parse_byte_size(char *astring);
int vlg_str_replace_env(char *str, size_t str_size);

#define vlg_max(a,b) ((a) > (b) ? (a) : (b))
#define vlg_min(a,b) ((a) < (b) ? (a) : (b))

#endif
