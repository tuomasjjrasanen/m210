/*
  m210 - Delete and download notes stored in Pegasus Mobile NoteTaker Tablet.
  Copyright © 2010 Tuomas Räsänen (tuos) <tuos@codegrove.org>

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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <m210.h>
#include <string.h>

#include "config.h"

extern char *program_invocation_name;

void help_and_exit(void)
{
    fprintf(stderr, "Try `%s --help' for more information.\n",
            program_invocation_name);
    exit(EXIT_FAILURE);
}

enum m210_orientation orientation = M210_ORIENTATION_TOP;
enum m210_area_size area_size = M210_AREA_SIZE_MAX;
char *outfile = NULL;

void parse_args(int argc, char **argv)
{
    const struct option options[] = {
        {"area-size", required_argument, NULL, 'a'},
        {"orientation", required_argument, NULL, 'o'},
        {"stream", optional_argument, NULL, 's'},
        {"version", no_argument, NULL, 'V'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    while (1) {
        int option;

        option = getopt_long(argc, argv, "Vh", options, NULL);

        if (option == -1)
            break;

        switch (option) {
        case 'a':
            break;
        case 'o':
            if (strncmp(optarg, "top", 4) == 0) {
                orientation = M210_ORIENTATION_TOP;
            } else if (strncmp(optarg, "left", 5) == 0) {
                orientation = M210_ORIENTATION_LEFT;
            } else if (strncmp(optarg, "right", 6) == 0) {
                orientation = M210_ORIENTATION_RIGHT;
            } else {
                fprintf(stderr, "%s: illegal orientation argument\n",
                        program_invocation_name);
                help_and_exit();
            }
            break;
        case 's':
            outfile = optarg ? optarg : "-";
            break;
        case 'V':
            printf("%s %s\n"
                   "Copyright © 2010 %s\n"
                   "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
                   "This is free software: you are free to change and redistribute it.\n"
                   "There is NO WARRANTY, to the extent permitted by law.\n",
                   PACKAGE_NAME, VERSION, PACKAGE_AUTHOR);
            exit(EXIT_SUCCESS);
        case 'h':
            printf("Usage: %s [OPTION]...\n"
                   "Set M210 device to operate in tablet mode\n"
                   "\n"
                   "Options:\n"
                   " --area-size=SIZE           set area size, 0-9, default: 9\n"
                   " --orientation=ORIENTATION  set orientation, {top, left, right}, default: top\n"
                   " --stream=FILE              stream tablet data to FILE, default: -\n"
                   " -h, --help                 display this help and exit\n"
                   " -V, --version              output version infromation and exit\n"
                   "\n"
                   "Report %s bugs to <%s>\n"
                   "Home page: <%s>\n",
                   program_invocation_name,
                   PACKAGE_NAME,
                   PACKAGE_BUGREPORT,
                   PACKAGE_URL);
            exit(EXIT_SUCCESS);
        case '?':
            help_and_exit();
        default:
            errx(EXIT_FAILURE, "argument parsing failed");
        }
    }

    if (optind != argc) {
        fprintf(stderr, "%s: wrong number of arguments\n",
                program_invocation_name);
        help_and_exit();
    }
}

int main(int argc, char **argv)
{
    struct m210 m210;
    enum m210_err err;
    int exitval = EXIT_FAILURE;
    FILE *stream;
    int stream_opened = 0;

    parse_args(argc, argv);

    err = m210_open(&m210, NULL);
    if (err) {
        m210_err_printf(err, "m210_open");
        goto err;
    }

    err = m210_set_mode(&m210, M210_MODE_INDICATOR_TABLET, M210_MODE_TABLET);
    if (err) {
        m210_err_printf(err, "m210_set_mode");
        goto err;
    }

    err = m210_config_tablet_mode(&m210, area_size, orientation);
    if (err) {
        m210_err_printf(err, "m210_config_tablet_mode");
        goto err;
    }

    if (outfile != NULL) {
        if (strncmp(outfile, "-", 2) == 0) {
            stream = stdout;
        } else {
            stream = fopen(outfile, "w");
            if (stream == NULL) {
                perror("fopen");
                goto err;
            }
            stream_opened = 1;
        }
        err = m210_fwrite_tablet_data(&m210, stream);
        if (err) {
            m210_err_printf(err, "m210_fwrite_tablet_data");
            goto err;
        }
    }

    exitval = EXIT_SUCCESS;

  err:
    if (stream_opened)
        fclose(stream);
    m210_close(&m210);
    return exitval;
}
