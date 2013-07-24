/* libm210
 * Copyright (C) 2011, 2013 Tuomas Räsänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "err.h"

char const *m210_err_strerror(enum m210_err const err)
{
	static char const *const err_strs[] = {
		"",
		"system call failed",
		"unknown device",
		"device not found",
		"unexpected response",
		"response waiting timeouted",
		"raw note has malformed head",
		"raw note has malformed body",
		"unexpected end-of-file"
	};
	return err_strs[err];
}

extern char *program_invocation_name;

enum m210_err m210_err_perror(enum m210_err const err, char const *const msg)
{
	enum m210_err result;
	int const original_errno = errno;
	char const *const m210_errstr = m210_err_strerror(err);
	/* +2 == a colon and a blank */
	size_t const progname_len = strlen(program_invocation_name) + 2;
	/* +2 == a colon and a blank */
	size_t const msg_len = msg == NULL ? 0 : strlen(msg) + 2;
	size_t const m210_errstr_len = strlen(m210_errstr);

	/* +1 == a terminating null byte. */
	size_t const total_len = (progname_len + msg_len
				  + m210_errstr_len + 1);
	char *errstr;

	errstr = (char *)calloc(total_len, sizeof(char));
	if (errstr == NULL) {
		result = M210_ERR_SYS;
		goto out;
	}

	if (msg == NULL) {
		snprintf(errstr, total_len, "%s: %s",
			 program_invocation_name, m210_errstr);
	} else {
		snprintf(errstr, total_len, "%s: %s: %s",
			 program_invocation_name, msg, m210_errstr);
	}

	if (err == M210_ERR_SYS) {
		errno = original_errno;
		perror(errstr);
	} else {
		fprintf(stderr, "%s\n", errstr);
	}

	result = M210_ERR_OK;
out:
	free(errstr);
	return result;
}
