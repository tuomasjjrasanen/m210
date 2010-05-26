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

enum format {
    format_txt
};

enum format format = format_txt;

void parse_args(int argc, char **argv)
{
    const struct option options[] = {
        {"format", optional_argument, NULL, 'f'},
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
        case 'f':
            if (strncmp(optarg, "txt", 4) == 0) {
                format = format_txt;
            } else {
                fprintf(stderr, "%s: illegal format argument\n",
                        program_invocation_name);
                help_and_exit();
            }
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
                   "Read M210 mouse data from standard input, format it and\n"
                   "write it to standard output.\n"
                   "\n"
                   "Options:\n"
                   " --format=FORMAT  set orientation, {txt}, default: txt\n"
                   " -h, --help       display this help and exit\n"
                   " -V, --version    output version infromation and exit\n"
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
    int exitval = EXIT_FAILURE;

    parse_args(argc, argv);

    while (1) {
        struct m210_mouse_data data;
        uint16_t x;
        uint16_t y;

        if (fread(&data, sizeof(struct m210_mouse_data), 1, stdin) != 1) {
            if (ferror(stdin))
                goto err;
            break;
        }

        memcpy(&x, data.x, 2);
        x = le16toh(x);
        memcpy(&y, data.y, 2);
        y = le16toh(y);

        printf("battery: ");
        switch (data.battery) {
        case battery_unknown:
            printf("unknown\n");
            break;
        case battery_low:
            printf("low\n");
            break;
        case battery_high:
            printf("high\n");
            break;
        default:
            printf("\n");
            break;
        }

        printf("pen: ");
        switch (data.pen) {
        case pen_out_of_range:
            printf("out of range\n");
            break;
        case pen_hovering:
            printf("hovering\n");
            break;
        case pen_tip_pressed:
            printf("tip pressed\n");
            break;
        case pen_switch_pressed:
            printf("switch pressed\n");
            break;
        case pen_both_pressed:
            printf("tip and switch pressed\n");
            break;
        default:
            printf("\n");
            break;
        }

        printf("x: %d\n", x);
        printf("y: %d\n", y);
    }

    exitval = EXIT_SUCCESS;

  err:
    return exitval;
}
