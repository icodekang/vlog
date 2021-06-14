#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>

#include "vlg_defs.h"
#include "vlg_buf.h"

void vlog_buf_profile(vlog_buf_t * a_buf, int flag)
{
	//vlg_assert(a_buf,);
	vlg_profile(flag, "---buf[%p][%ld-%ld][%ld][%s][%p:%ld]---",
			a_buf,
			a_buf->size_min, a_buf->size_max,
			a_buf->size_real,
			a_buf->truncate_str,
			a_buf->start, a_buf->tail - a_buf->start);
	return;
}

void vlog_buf_del(vlog_buf_t * a_buf)
{
	//vlg_assert(a_buf,);
	if (a_buf->start) free(a_buf->start);
	vlg_debug("vlog_buf_del[%p]", a_buf);
    free(a_buf);
	return;
}

vlog_buf_t *vlog_buf_new(size_t buf_size_min, size_t buf_size_max, const char *truncate_str)
{
	vlog_buf_t *a_buf;

	if (buf_size_min == 0) {
		vlg_error("buf_size_min == 0, not allowed");
		return NULL;
	}

	if (buf_size_max != 0 && buf_size_max < buf_size_min) {
		vlg_error("buf_size_max[%lu] < buf_size_min[%lu] && buf_size_max != 0",
			 (unsigned long)buf_size_max, (unsigned long)buf_size_min);
		return NULL;
	}

	a_buf = calloc(1, sizeof(*a_buf));
	if (!a_buf) {
		vlg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	if (truncate_str) {
		if (strlen(truncate_str) > sizeof(a_buf->truncate_str) - 1) {
			vlg_error("truncate_str[%s] overflow", truncate_str);
			goto err;
		} else {
			strcpy(a_buf->truncate_str, truncate_str);
		}
		a_buf->truncate_str_len = strlen(truncate_str);
	}

	a_buf->size_min = buf_size_min;
	a_buf->size_max = buf_size_max;
	a_buf->size_real = a_buf->size_min;

	a_buf->start = calloc(1, a_buf->size_real);
	if (!a_buf->start) {
		vlg_error("calloc fail, errno[%d]", errno);
		goto err;
	}

	a_buf->tail = a_buf->start;
	a_buf->end_plus_1 = a_buf->start + a_buf->size_real;
	a_buf->end = a_buf->end_plus_1 - 1;

	//vlog_buf_profile(a_buf, vlg_DEBUG);
	return a_buf;

err:
	vlog_buf_del(a_buf);
	return NULL;
}

static void vlog_buf_truncate(vlog_buf_t * a_buf)
{
	char *p;
	size_t len;

	if ((a_buf->truncate_str)[0] == '\0') return;
	p = (a_buf->tail - a_buf->truncate_str_len);
	if (p < a_buf->start) p = a_buf->start;
	len = a_buf->tail - p;
	memcpy(p, a_buf->truncate_str, len);
	return;
}

/* return 0:	success
 * return <0:	fail, set size_real to -1;
 * return >0:	by conf limit, can't extend size
 * increment must > 0
 */
static int vlog_buf_resize(vlog_buf_t * a_buf, size_t increment)
{
	int rc = 0;
	size_t new_size = 0;
	size_t len = 0;
	char *p = NULL;

	if (a_buf->size_max != 0 && a_buf->size_real >= a_buf->size_max) {
		vlg_error("a_buf->size_real[%ld] >= a_buf->size_max[%ld]",
			 a_buf->size_real, a_buf->size_max);
		return 1;
	}

	if (a_buf->size_max == 0) {
		/* unlimit */
		new_size = a_buf->size_real + 1.5 * increment;
	} else {
		/* limited  */
		if (a_buf->size_real + increment <= a_buf->size_max) {
			new_size = a_buf->size_real + increment;
		} else {
			new_size = a_buf->size_max;
			rc = 1;
		}
	}

	len = a_buf->tail - a_buf->start;
	p = realloc(a_buf->start, new_size);
	if (!p) {
		vlg_error("realloc fail, errno[%d]", errno);
		free(a_buf->start);
		a_buf->start = NULL;
		a_buf->tail = NULL;
		a_buf->end = NULL;
		a_buf->end_plus_1 = NULL;
		return -1;
	} else {
		a_buf->start = p;
		a_buf->tail = p + len;
		a_buf->size_real = new_size;
		a_buf->end_plus_1 = a_buf->start + new_size;
		a_buf->end = a_buf->end_plus_1 - 1;
	}

	return rc;
}

int vlog_buf_vprintf(vlog_buf_t * a_buf, const char *format, va_list args)
{
	va_list ap;
	size_t size_left;
	int nwrite;

	if (!a_buf->start) {
		vlg_error("pre-use of vlog_buf_resize fail, so can't convert");
		return -1;
	}

	va_copy(ap, args);
	size_left = a_buf->end_plus_1 - a_buf->tail;
	nwrite = vsnprintf(a_buf->tail, size_left, format, ap);
	if (nwrite >= 0 && nwrite < size_left) {
		a_buf->tail += nwrite;
		//*(a_buf->tail) = '\0';
		return 0;
	} else if (nwrite < 0) {
		vlg_error("vsnprintf fail, errno[%d]", errno);
		vlg_error("nwrite[%d], size_left[%ld], format[%s]", nwrite, size_left, format);
		return -1;
	} else if (nwrite >= size_left) {
		int rc;
		//vlg_debug("nwrite[%d]>=size_left[%ld],format[%s],resize", nwrite, size_left, format);
		rc = vlog_buf_resize(a_buf, nwrite - size_left + 1);
		if (rc > 0) {
			vlg_error("conf limit to %ld, can't extend, so truncate", a_buf->size_max);
			va_copy(ap, args);
			size_left = a_buf->end_plus_1 - a_buf->tail;
			vsnprintf(a_buf->tail, size_left, format, ap);
			a_buf->tail += size_left - 1;
			//*(a_buf->tail) = '\0';
			vlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			vlg_error("vlog_buf_resize fail");
			return -1;
		} else {
			//vlg_debug("vlog_buf_resize succ, to[%ld]", a_buf->size_real);

			va_copy(ap, args);
			size_left = a_buf->end_plus_1 - a_buf->tail;
			nwrite = vsnprintf(a_buf->tail, size_left, format, ap);
			if (nwrite < 0) {
				vlg_error("vsnprintf fail, errno[%d]", errno);
				vlg_error("nwrite[%d], size_left[%ld], format[%s]", nwrite, size_left, format);
				return -1;
			} else {
				a_buf->tail += nwrite;
				//*(a_buf->tail) = '\0';
				return 0;
			}
		}
	}

	return 0;
}

/* if width > num_len, 0 padding, else output num */
int vlog_buf_printf_dec32(vlog_buf_t * a_buf, uint32_t ui32, int width)
{
	unsigned char *p;
	char *q;
	unsigned char tmp[vlog_INT32_LEN + 1];
	size_t num_len, zero_len, out_len;

	if (!a_buf->start) {
		vlg_error("pre-use of vlog_buf_resize fail, so can't convert");
		return -1;
	}

	p = tmp + vlog_INT32_LEN;
	do {
		*--p = (unsigned char) (ui32 % 10 + '0');
	} while (ui32 /= 10);

	/* zero or space padding */
	num_len = (tmp + vlog_INT32_LEN) - p;

	if (width > num_len) {
		zero_len = width - num_len;
		out_len = width;
	} else {
		zero_len = 0;
		out_len = num_len;
	}

	if ((q = a_buf->tail + out_len) > a_buf->end) {
		int rc;
		//vlg_debug("size_left not enough, resize");
		rc = vlog_buf_resize(a_buf, out_len - (a_buf->end - a_buf->tail));
		if (rc > 0) {
			size_t len_left;
			vlg_error("conf limit to %ld, can't extend, so output", a_buf->size_max);
			len_left = a_buf->end - a_buf->tail;
			if (len_left <= zero_len) {
				zero_len = len_left;
				num_len = 0;
			} else if (len_left > zero_len) {
				/* zero_len not changed */
				num_len = len_left - zero_len;
			}
			if (zero_len) memset(a_buf->tail, '0', zero_len);
			memcpy(a_buf->tail + zero_len, p, num_len);
			a_buf->tail += len_left;
			//*(a_buf->tail) = '\0';
			vlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			vlg_error("vlog_buf_resize fail");
			return -1;
		} else {
			//vlg_debug("vlog_buf_resize succ, to[%ld]", a_buf->size_real);
			q = a_buf->tail + out_len; /* re-calculate p*/
		}
	}

	if (zero_len) memset(a_buf->tail, '0', zero_len);
	memcpy(a_buf->tail + zero_len, p, num_len);
	a_buf->tail = q;
	//*(a_buf->tail) = '\0';
	return 0;
}

int vlog_buf_printf_dec64(vlog_buf_t * a_buf, uint64_t ui64, int width)
{
	unsigned char *p;
	char *q;
	unsigned char tmp[vlog_INT64_LEN + 1];
	size_t num_len, zero_len, out_len;
	uint32_t ui32;

	if (!a_buf->start) {
		vlg_error("pre-use of vlog_buf_resize fail, so can't convert");
		return -1;
	}

	p = tmp + vlog_INT64_LEN;
	if (ui64 <= vlog_MAX_UINT32_VALUE) {
		/*
		* To divide 64-bit numbers and to find remainders
		* on the x86 platform gcc and icc call the libc functions
		* [u]divdi3() and [u]moddi3(), they call another function
		* in its turn.  On FreeBSD it is the qdivrem() function,
		* its source code is about 170 lines of the code.
		* The glibc counterpart is about 150 lines of the code.
		*
		* For 32-bit numbers and some divisors gcc and icc use
		* a inlined multiplication and shifts.  For example,
		* unsigned "i32 / 10" is compiled to
		*
		*     (i32 * 0xCCCCCCCD) >> 35
		*/

		ui32 = (uint32_t) ui64;

		do {
			*--p = (unsigned char) (ui32 % 10 + '0');
		} while (ui32 /= 10);

	} else {
		do {
			*--p = (unsigned char) (ui64 % 10 + '0');
		} while (ui64 /= 10);
	}


	/* zero or space padding */
	num_len = (tmp + vlog_INT64_LEN) - p;

	if (width > num_len) {
		zero_len = width - num_len;
		out_len = width;
	} else {
		zero_len = 0;
		out_len = num_len;
	}

	if ((q = a_buf->tail + out_len) > a_buf->end) {
		int rc;
		//vlg_debug("size_left not enough, resize");
		rc = vlog_buf_resize(a_buf, out_len - (a_buf->end - a_buf->tail));
		if (rc > 0) {
			size_t len_left;
			vlg_error("conf limit to %ld, can't extend, so output", a_buf->size_max);
			len_left = a_buf->end - a_buf->tail;
			if (len_left <= zero_len) {
				zero_len = len_left;
				num_len = 0;
			} else if (len_left > zero_len) {
				/* zero_len not changed */
				num_len = len_left - zero_len;
			}
			if (zero_len) memset(a_buf->tail, '0', zero_len);
			memcpy(a_buf->tail + zero_len, p, num_len);
			a_buf->tail += len_left;
			//*(a_buf->tail) = '\0';
			vlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			vlg_error("vlog_buf_resize fail");
			return -1;
		} else {
			//vlg_debug("vlog_buf_resize succ, to[%ld]", a_buf->size_real);
			q = a_buf->tail + out_len; /* re-calculate p*/
		}
	}

	if (zero_len) memset(a_buf->tail, '0', zero_len);
	memcpy(a_buf->tail + zero_len, p, num_len);
	a_buf->tail = q;
	//*(a_buf->tail) = '\0';
	return 0;
}

int vlog_buf_printf_hex(vlog_buf_t * a_buf, uint32_t ui32, int width)
{
	unsigned char *p;
	char *q;
	unsigned char tmp[vlog_INT32_LEN + 1];
	size_t num_len, zero_len, out_len;
	static unsigned char   hex[] = "0123456789abcdef";
	//static unsigned char   HEX[] = "0123456789ABCDEF";

	if (!a_buf->start) {
		vlg_error("pre-use of vlog_buf_resize fail, so can't convert");
		return -1;
	}


	p = tmp + vlog_INT32_LEN;
	do {
		/* the "(uint32_t)" cast disables the BCC's warning */
		*--p = hex[(uint32_t) (ui32 & 0xf)];
	} while (ui32 >>= 4);

#if 0
	} else { /* is_hex == 2 */

		do {
			/* the "(uint32_t)" cast disables the BCC's warning */
			*--p = HEX[(uint32_t) (ui64 & 0xf)];

		} while (ui64 >>= 4);
	}
#endif

	/* zero or space padding */
	num_len = (tmp + vlog_INT32_LEN) - p;

	if (width > num_len) {
		zero_len = width - num_len;
		out_len = width;
	} else {
		zero_len = 0;
		out_len = num_len;
	}

	if ((q = a_buf->tail + out_len) > a_buf->end) {
		int rc;
		//vlg_debug("size_left not enough, resize");
		rc = vlog_buf_resize(a_buf, out_len - (a_buf->end - a_buf->tail));
		if (rc > 0) {
			size_t len_left;
			vlg_error("conf limit to %ld, can't extend, so output", a_buf->size_max);
			len_left = a_buf->end - a_buf->tail;
			if (len_left <= zero_len) {
				zero_len = len_left;
				num_len = 0;
			} else if (len_left > zero_len) {
				/* zero_len not changed */
				num_len = len_left - zero_len;
			}
			if (zero_len) memset(a_buf->tail, '0', zero_len);
			memcpy(a_buf->tail + zero_len, p, num_len);
			a_buf->tail += len_left;
			//*(a_buf->tail) = '\0';
			vlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			vlg_error("vlog_buf_resize fail");
			return -1;
		} else {
			//vlg_debug("vlog_buf_resize succ, to[%ld]", a_buf->size_real);
			q = a_buf->tail + out_len; /* re-calculate p*/
		}
	}

	if (zero_len) memset(a_buf->tail, '0', zero_len);
	memcpy(a_buf->tail + zero_len, p, num_len);
	a_buf->tail = q;
	//*(a_buf->tail) = '\0';
	return 0;
}

int vlog_buf_append(vlog_buf_t * a_buf, const char *str, size_t str_len)
{
	char *p;
#if 0
	if (str_len <= 0 || str == NULL) {
		return 0;
	}
	if (!a_buf->start) {
		vlg_error("pre-use of vlog_buf_resize fail, so can't convert");
		return -1;
	}
#endif

	if ((p = a_buf->tail + str_len) > a_buf->end) {
		int rc;
		//vlg_debug("size_left not enough, resize");
		rc = vlog_buf_resize(a_buf, str_len - (a_buf->end - a_buf->tail));
		if (rc > 0) {
			size_t len_left;
			vlg_error("conf limit to %ld, can't extend, so output",
				 a_buf->size_max);
			len_left = a_buf->end - a_buf->tail;
			memcpy(a_buf->tail, str, len_left);
			a_buf->tail += len_left;
			//*(a_buf->tail) = '\0';
			vlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			vlg_error("vlog_buf_resize fail");
			return -1;
		} else {
			//vlg_debug("vlog_buf_resize succ, to[%ld]", a_buf->size_real);
			p = a_buf->tail + str_len; /* re-calculate p*/
		}
	}

	memcpy(a_buf->tail, str, str_len);
	a_buf->tail = p;
	// *(a_buf->tail) = '\0';
	return 0;
}

int vlog_buf_adjust_append(vlog_buf_t * a_buf, const char *str, size_t str_len,
		int left_adjust, int zero_pad, size_t in_width, size_t out_width)
{
	size_t append_len = 0;
	size_t source_len = 0;
	size_t space_len = 0;

#if 0
	if (str_len <= 0 || str == NULL) {
		return 0;
	}
#endif

	if (!a_buf->start) {
		vlg_error("pre-use of vlog_buf_resize fail, so can't convert");
		return -1;
	}

	/* calculate how many character will be got from str */
	if (out_width == 0 || str_len < out_width) {
		source_len = str_len;
	} else {
		source_len = out_width;
	}

	/* calculate how many character will be output */
	if (in_width == 0 || source_len >= in_width ) {
		append_len = source_len;
		space_len = 0;
	} else {
		append_len = in_width;
		space_len = in_width - source_len;
	}

	/*  |-----append_len-----------| */
	/*  |-source_len---|-space_len-|  left_adjust */
	/*  |-space_len---|-source_len-|  right_adjust */
	/*  |-(size_real-1)---|           size not enough */

	if (append_len > a_buf->end - a_buf->tail) {
		int rc = 0;
		//vlg_debug("size_left not enough, resize");
		rc = vlog_buf_resize(a_buf, append_len - (a_buf->end -a_buf->tail));
		if (rc > 0) {
			vlg_error("conf limit to %ld, can't extend, so output", a_buf->size_max);
			append_len = (a_buf->end - a_buf->tail);
			if (left_adjust) {
				if (source_len < append_len) {
					space_len = append_len - source_len;
				} else {
					source_len = append_len;
					space_len = 0;
				}
				if (space_len) memset(a_buf->tail + source_len, ' ', space_len);
				memcpy(a_buf->tail, str, source_len);
			} else {
				if (space_len < append_len) {
					source_len = append_len - space_len;
				} else {
					space_len = append_len;
					source_len = 0;
				}
				if (space_len) {
					if (zero_pad) {
						memset(a_buf->tail, '0', space_len);
					} else {
						memset(a_buf->tail, ' ', space_len);
					}
				}
				memcpy(a_buf->tail + space_len, str, source_len);
			}
			a_buf->tail += append_len;
			//*(a_buf->tail) = '\0';
			vlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			vlg_error("vlog_buf_resize fail");
			return -1;
		} else {
			//vlg_debug("vlog_buf_resize succ, to[%ld]", a_buf->size_real);
		}
	}

	if (left_adjust) {
		if (space_len) memset(a_buf->tail + source_len, ' ', space_len);
		memcpy(a_buf->tail, str, source_len);
	} else {
		if (space_len) {
			if (zero_pad) {
				memset(a_buf->tail, '0', space_len);
			} else {
				memset(a_buf->tail, ' ', space_len);
			}
		}
		memcpy(a_buf->tail + space_len, str, source_len);
	}
	a_buf->tail += append_len;
	//*(a_buf->tail) = '\0';
	return 0;
}