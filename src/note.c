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
                struct m210_note_path *const paths = note_ptr->paths;
                if (paths) {
                        /* If a note has paths, then the first
                         * path always stores all coordinates. */
                        free(paths->coords);
                        for (size_t i = 0; i < note_ptr->path_count; ++i) {
                                (paths + i)->coords = NULL;
                                (paths + i)->coord_count = 0;
                        }
                }
                free(note_ptr->paths);
                note_ptr->paths = NULL;
                note_ptr->path_count = 0;
        }
        free(note_ptr);
        *note_ptr_ptr = NULL;
}

static enum m210_note_err
m210_note_create_paths(struct m210_note *note_ptr, FILE *stream_ptr,
                       uint32_t note_end_pos)
{
        enum m210_note_err err = M210_NOTE_ERR_OK;
        struct m210_note_coord *coords = NULL;

        struct m210_note_path *paths = NULL;
        size_t path_count = 0;
        size_t path_len = 0;

        struct m210_rawnote_body *bodies = NULL;
        size_t body_count = 0;

        long cur_pos;

        cur_pos = ftell(stream_ptr);
        if (cur_pos == -1) {
                err = M210_NOTE_ERR_SYS;
                goto exit;
        }
        
        /* The data section of the note consists of N bodies. */
        body_count = ((note_end_pos - cur_pos)
                      / sizeof(struct m210_rawnote_body));
        bodies = malloc(body_count * sizeof(struct m210_rawnote_body));
        if (!bodies) {
                err = M210_NOTE_ERR_SYS;
                goto exit;
        }
        coords = (struct m210_note_coord *) bodies;

        for (size_t i = 0; i < body_count; ++i) {

                if (fread(bodies + i, sizeof(struct m210_rawnote_body), 1,
                          stream_ptr) != 1) {
                        if (ferror(stream_ptr)) {
                                err = M210_NOTE_ERR_BAD_BODY;
                                goto exit;
                        }
                        /* EOF should never happen at this point,
                         * otherwise the stream is flawed somehow. */
                        err = M210_NOTE_ERR_EOF;
                        goto exit;
                }

                /* Path ends when the pen is raised. */
                if (m210_rawnote_body_is_penup(bodies + i)) {
                        struct m210_note_path *new_paths;

                        /* Let's extend the array of paths by one. */
                        new_paths = realloc(paths,
                                            ((path_count + 1)
                                             * sizeof(struct m210_note_path)));
                        if (!new_paths) {
                                err = M210_NOTE_ERR_SYS;
                                goto exit;
                        }
                        paths = new_paths;
                        (paths + path_count)->coord_count = path_len;
                        (paths + path_count)->coords = coords + i - path_len;
                        path_len = 0;
                        path_count += 1;

                        continue;
                } else { /* Body represents a coordinate change. */
                        (coords + i)->x = le16toh((coords + i)->x);
                        (coords + i)->y = le16toh((coords + i)->y);
                        path_len += 1;
                }

        }
exit:
        if (err) {
                free(paths);
                paths = NULL;
                path_count = 0;
                return err;
        }
        note_ptr->paths = paths;
        note_ptr->path_count = path_count;
        return M210_NOTE_ERR_OK;                
}

enum m210_note_err
m210_note_create_next(struct m210_note **note_ptr_ptr, FILE *stream_ptr)
{
        enum m210_note_err err = M210_NOTE_ERR_OK;
        struct m210_note *note_ptr = NULL;
        struct m210_rawnote_head head;

        if (fread(&head,
                  sizeof(struct m210_rawnote_head), 1, stream_ptr) != 1) {
                if (ferror(stream_ptr)) {
                        err = M210_NOTE_ERR_BAD_HEAD;
                        goto exit;
                }
                /* EOF should never happen at this point, otherwise
                 * the stream is flawed somehow. */
                err = M210_NOTE_ERR_EOF;
                goto exit;
        }

        if (!memcmp(&head, &M210_RAWNOTE_HEAD_LAST,
                    sizeof(struct m210_rawnote_head))) {
                /* Last note is always empty, and therefore there is
                 * no need to walk through the data section. (There
                 * might be few zero-bytes of data just for padding
                 * purposes: notes are always downloaded in 62 byte
                 * long packets.) */
                goto exit;
        }

        if (head.state != M210_RAWNOTE_STATE_EMPTY
            && head.state != M210_RAWNOTE_STATE_UNFINISHED
            && head.state != M210_RAWNOTE_STATE_FINISHED_BY_SOFTWARE
            && head.state != M210_RAWNOTE_STATE_FINISHED_BY_USER) {
                err = M210_NOTE_ERR_BAD_HEAD;
                goto exit;
        }

        note_ptr = calloc(1, sizeof(struct m210_note));
        if (!note_ptr) {
                err = M210_NOTE_ERR_SYS;
                goto exit;
        }
        note_ptr->number = head.number;

        err = m210_note_create_paths(note_ptr, stream_ptr,
                                     le24toh32(head.next_pos));
exit:
        if (err) {
                free(note_ptr);
                note_ptr = NULL;
                return err;
        }
        *note_ptr_ptr = note_ptr;
        return M210_NOTE_ERR_OK;
}
