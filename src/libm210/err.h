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

#ifndef ERR_H
#define ERR_H

enum m210_err {
	M210_ERR_OK,
	M210_ERR_SYS,
	M210_ERR_BAD_DEV,
	M210_ERR_NO_DEV,
	M210_ERR_BAD_DEV_MSG,
	M210_ERR_DEV_TIMEOUT,
	M210_ERR_BAD_RAWNOTE_HEAD,
	M210_ERR_BAD_RAWNOTE_BODY,
	M210_ERR_UNEXPECTED_EOF
};

char const *m210_err_strerror(enum m210_err err);
enum m210_err m210_err_perror(enum m210_err err, char const *msg);

#endif /* ERR_H */
