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

#include <endian.h>
#include <stdlib.h>
#include <string.h>

#include "note.h"
#include "rawnote.h"

static inline int is_penup(struct m210_rawnote_body const *const bodyp)
{
	return memcmp(bodyp, &M210_RAWNOTE_BODY_PENUP,
		      sizeof(struct m210_rawnote_body)) == 0;
}

static inline uint32_t le24toh32(uint8_t const *three_bytes)
{
	uint32_t result = 0;
	memcpy(&result, three_bytes, 3);
	return le32toh(result);
}

enum m210_err m210_note_read_head(struct m210_note_head *headp, FILE *file)
{
	enum m210_err err;
	struct m210_rawnote_head rawhead;
	long cur_pos;

	if (fread(&rawhead, sizeof(struct m210_rawnote_head), 1, file) != 1) {
		if (ferror(file)) {
			err = M210_ERR_BAD_RAWNOTE_HEAD;
			goto out;
		}
		/* EOF should never happen at this point, otherwise
		 * the stream is flawed somehow. */
		err = M210_ERR_UNEXPECTED_EOF;
		goto out;
	}

	if (!memcmp(&rawhead, &M210_RAWNOTE_HEAD_LAST,
		    sizeof(struct m210_rawnote_head))) {
		/* Last note is always empty, and therefore there is
		 * no need to walk through the data section. (There
		 * might be few zero-bytes of data just for padding
		 * purposes: notes are always downloaded in 62 byte
		 * long packets.) */
		headp->number = 0;
		headp->bodyc = 0;
		err = M210_ERR_OK;
		goto out;
	}

	cur_pos = ftell(file);
	if (cur_pos == -1) {
		err = M210_ERR_SYS;
		goto out;
	}

	/* The data section of a note consists of exactly N bodies. */
	headp->bodyc = ((le24toh32(rawhead.next_pos) - cur_pos)
			/ sizeof(struct m210_rawnote_body));
	headp->number = rawhead.number;

	err = M210_ERR_OK;
out:
	return err;
}

enum m210_err m210_note_read_body(struct m210_note_body *bodyp, FILE *file)
{
	enum m210_err err;
	struct m210_rawnote_body rawbody;

	if (fread(&rawbody, sizeof(struct m210_rawnote_body), 1, file) != 1) {
		/* We tried to read one item but failed. fread() does
		 * not distinguish between eof and error, let's find
		 * out which one happened. */
		if (ferror(file)) {
			err = M210_ERR_BAD_RAWNOTE_BODY;
			goto out;
		}
		/* EOF should never happen at this point, otherwise
		 * the stream is flawed somehow. */
		err = M210_ERR_UNEXPECTED_EOF;
		goto out;
	}

	/* Map byte arrays to coordinate values. */
	memcpy(&(bodyp->x), rawbody.x, 2);
	memcpy(&(bodyp->y), rawbody.y, 2);

	/* Mind the byte order. */
	bodyp->x = le16toh(bodyp->x);
	bodyp->y = le16toh(bodyp->y);

	if (is_penup(&rawbody)) {
		bodyp->pressure = 0;
	} else {
		bodyp->pressure = 1;
	}

	err = M210_ERR_OK;
out:
	return err;
}
