/*
 * This file is part of the vlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __vlog_buf_h
#define __vlog_buf_h

/* buf, is a dynamic expand buffer for one single log,
 * as one single log will interlace if use multiple write() to file.
 * and buf is always keep in a thread, to make each thread has its
 * own buffer to avoid lock.
 */

#include <stdarg.h>
#include <stdint.h>

typedef struct vlog_buf_s {
	char *start;
	char *tail;
	char *end;
	char *end_plus_1;

	size_t size_min;
	size_t size_max;
	size_t size_real;

	char truncate_str[MAXLEN_PATH + 1];
	size_t truncate_str_len;
} vlog_buf_t;


vlog_buf_t *vlog_buf_new(size_t min, size_t max, const char *truncate_str);
void vlog_buf_del(vlog_buf_t * a_buf);
void vlog_buf_profile(vlog_buf_t * a_buf, int flag);

int vlog_buf_vprintf(vlog_buf_t * a_buf, const char *format, va_list args);
int vlog_buf_append(vlog_buf_t * a_buf, const char *str, size_t str_len);
int vlog_buf_adjust_append(vlog_buf_t * a_buf, const char *str, size_t str_len,
			int left_adjust, int zero_pad, size_t in_width, size_t out_width);
int vlog_buf_printf_dec32(vlog_buf_t * a_buf, uint32_t ui32, int width);
int vlog_buf_printf_dec64(vlog_buf_t * a_buf, uint64_t ui64, int width);
int vlog_buf_printf_hex(vlog_buf_t * a_buf, uint32_t ui32, int width);

#define vlog_buf_restart(a_buf) do { \
	a_buf->tail = a_buf->start; \
} while(0)

#define vlog_buf_len(a_buf) (a_buf->tail - a_buf->start)
#define vlog_buf_str(a_buf) (a_buf->start)
#define vlog_buf_seal(a_buf) do {*(a_buf)->tail = '\0';} while (0)

#endif
