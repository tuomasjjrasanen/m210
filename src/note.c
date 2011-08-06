/*
  libm210 - C API for Pegasus Tablet Mobile NoteTaker (M210)
  Copyright © 2011 Tuomas Jorma Juhani Räsänen <tuomas.rasanen@tjjr.fi>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <endian.h>
#include <stdlib.h>
#include <string.h>

#include "note.h"
#include "rawnote.h"

static inline int
m210_rawnote_body_is_penup(struct m210_rawnote_body const *const body_ptr)
{
        return memcmp(body_ptr, &M210_RAWNOTE_BODY_PENUP,
                      sizeof(struct m210_rawnote_body)) == 0;
}

static inline uint32_t
le24toh32(uint8_t const *three_bytes)
{
        uint32_t result = 0;
        memcpy(&result, three_bytes, 3);
        return le32toh(result);
}

void
m210_note_destroy(struct m210_note **note_ptr_ptr)
{
        struct m210_note *const note_ptr = *note_ptr_ptr;
        if (note_ptr) {
                if (note_ptr->paths) {
                        free(note_ptr->paths[0].coords);
                        free(note_ptr->paths);
                        note_ptr->paths = NULL;
                }
                free(note_ptr);
                *note_ptr_ptr = NULL;
        }
}

enum m210_note_err
m210_note_create_next(struct m210_note **note_ptr_ptr, FILE *stream_ptr)
{
        enum m210_note_err err = M210_NOTE_ERR_OK;
        struct m210_note *note_ptr = NULL;
        struct m210_rawnote_head head;
        uint32_t note_end_pos;
        size_t path_len = 0;
        long cur_pos;

        struct m210_note_coord *coords = NULL;
        struct m210_rawnote_body *bodies = NULL;
        size_t body_count = 0;

        if (fread(&head,
                  sizeof(struct m210_rawnote_head), 1, stream_ptr) != 1) {
                if (ferror(stream_ptr)) {
                        err = M210_NOTE_ERR_BAD_HEAD;
                        goto err;
                }
                /* EOF should never happen at this point, otherwise
                 * the stream is flawed somehow. */
                err = M210_NOTE_ERR_EOF;
                goto err;                
        }

        if (!memcmp(&head, &M210_RAWNOTE_HEAD_LAST,
                    sizeof(struct m210_rawnote_head))) {
                /* Last note is always empty, and therefore there is
                 * no need to walk through the data section. (There
                 * might be few zero-bytes of data just for padding
                 * purposes: notes are always downloaded in 62 byte
                 * long packets.) */
                goto out;
        }

        note_ptr = calloc(1, sizeof(struct m210_note));
        if (!note_ptr) {
                err = M210_NOTE_ERR_SYS;
                goto err;
        }
        note_ptr->number = head.number;

        cur_pos = ftell(stream_ptr);
        if (cur_pos == -1) {
                err = M210_NOTE_ERR_SYS;
                goto err;
        }
        
        /* Note ends at the beginning of next node head. */
        note_end_pos = le24toh32(head.next_pos);

        /* The data section of the note consists of N bodies. */
        body_count = ((note_end_pos - cur_pos) / sizeof(struct m210_rawnote_body));
        bodies = malloc(body_count * sizeof(struct m210_rawnote_body));
        if (!bodies) {
                err = M210_NOTE_ERR_SYS;
                goto err;
        }
        coords = (struct m210_note_coord *) bodies;

        /* So the rest of our journey is all about collecting bodies
         * from the stream. */
        for (size_t i = 0; i < body_count; ++i) {
                struct m210_rawnote_body *const body_ptr = bodies + i;

                if (fread(body_ptr, sizeof(struct m210_rawnote_body), 1,
                          stream_ptr) != 1) {
                        if (ferror(stream_ptr)) {
                                err = M210_NOTE_ERR_BAD_BODY;
                                goto err;
                        }
                        /* EOF should never happen at this point,
                         * otherwise the stream is flawed somehow. */
                        err = M210_NOTE_ERR_EOF;
                        goto err;
                }

                /* Path ends when the pen is raised. */
                if (m210_rawnote_body_is_penup(body_ptr)) {
                        struct m210_note_path *cur_path;
                        struct m210_note_path *new_paths;

                        /* Let's extend the array of paths by one. */
                        new_paths = realloc(note_ptr->paths,
                                            ((note_ptr->path_count + 1)
                                             * sizeof(struct m210_note_path)));
                        if (!new_paths) {
                                err = M210_NOTE_ERR_SYS;
                                goto err;
                        }
                        cur_path = new_paths + note_ptr->path_count;
                        cur_path->coord_count = path_len;
                        cur_path->coords = coords + i - path_len;
                        path_len = 0;
                        note_ptr->paths = new_paths;
                        note_ptr->path_count += 1;

                        continue;
                } 
                path_len += 1;

        }
        goto out;
err:
        free(bodies);
        m210_note_destroy(&note_ptr);
out:
        *note_ptr_ptr = note_ptr;
        return err;
}
