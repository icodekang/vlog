#ifndef __vg_util_h
#define __vg_util_h

size_t vg_parse_byte_size(char *astring);
int vg_str_replace_env(char *str, size_t str_size);

#define vg_max(a,b) ((a) > (b) ? (a) : (b))
#define vg_min(a,b) ((a) < (b) ? (a) : (b))

#endif
