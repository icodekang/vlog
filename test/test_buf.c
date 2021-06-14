#include <stdio.h>

#include "vlg_defs.h"
#include "vlg_buf.h"

int main(int argc, char** argv)
{
	vlog_buf_t *a_buf;
	char *aa;

	a_buf = vlog_buf_new(10, 20, "ABC");
	if (!a_buf) {
		vlg_error("vlog_buf_new fail");
		return -1;
	}

	aa = "123456789";
	vlog_buf_append(a_buf, aa, strlen(aa));
	vlg_error("a_buf->start[%s]", a_buf->start);
	fwrite(a_buf->start, vlog_buf_len(a_buf), 1, stdout);
	vlg_error("------------");

	aa = "0";
	vlog_buf_append(a_buf, aa, strlen(aa));
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	aa = "12345";
	vlog_buf_append(a_buf, aa, strlen(aa));
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	aa = "6789";
	vlog_buf_append(a_buf, aa, strlen(aa));
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	aa = "0";
	vlog_buf_append(a_buf, aa, strlen(aa));
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	aa = "22345";
	vlog_buf_append(a_buf, aa, strlen(aa));
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");


	aa = "abc";
	int i,j;
	for (i = 0; i <= 5; i++) {
		for (j = 0; j <= 5; j++) {
			vlog_buf_restart(a_buf);
			vlg_error("left[1],max[%d],min[%d]", i, j);
			vlog_buf_adjust_append(a_buf, aa, strlen(aa), 1, 0, i, j);
			vlg_error("a_buf->start[%s]", a_buf->start);

			vlg_error("-----");

			vlog_buf_restart(a_buf);
			vlg_error("left[0],max[%d],min[%d]", i, j);
			vlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, i, j);
			vlg_error("a_buf->start[%s]", a_buf->start);
			vlg_error("------------");
		}
	}

	aa = "1234567890";
	vlg_error("left[0],max[%d],min[%d]", 15, 5);
	vlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, 15, 5);
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	aa = "1234567890";
	vlog_buf_restart(a_buf);
	vlg_error("left[0],max[%d],min[%d]", 25, 5);
	vlog_buf_adjust_append(a_buf, aa, strlen(aa), 1, 0, 25, 5);
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	vlog_buf_restart(a_buf);
	vlg_error("left[0],max[%d],min[%d]", 19, 5);
	vlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, 19, 5);
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	vlog_buf_restart(a_buf);
	vlg_error("left[0],max[%d],min[%d]", 20, 5);
	vlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, 20, 5);
	vlg_error("a_buf->start[%s]", a_buf->start);
	vlg_error("------------");

	vlog_buf_del(a_buf);

	return 0;
}
