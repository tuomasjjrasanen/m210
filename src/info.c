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

#include "config.h"

extern char *program_invocation_name;

void help_and_exit(void)
{
    fprintf(stderr, "Try `%s --help' for more information.\n",
            program_invocation_name);
    exit(EXIT_FAILURE);
}

#define OUTPUT_MODE     0x01
#define OUTPUT_FIRMWARE 0x02
#define OUTPUT_ANALOG   0x04
#define OUTPUT_PAD      0x08
#define OUTPUT_SIZE     0x10

int outputs = 0;

void parse_args(int argc, char **argv)
{
    const struct option options[] = {
        {"pad", no_argument, NULL, 'p'},
        {"analog", no_argument, NULL, 'a'},
        {"firmware", no_argument, NULL, 'f'},
        {"size", no_argument, NULL, 's'},
        {"mode", no_argument, NULL, 'm'},
        {"version", no_argument, NULL, 'V'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    while (1) {
        int option;

        option = getopt_long(argc, argv, "pafsmVh", options, NULL);

        if (option == -1)
            break;

        switch (option) {
        case 'p':
            outputs |= OUTPUT_PAD;
            break;
        case 'a':
            outputs |= OUTPUT_ANALOG;
            break;
        case 'f':
            outputs |= OUTPUT_FIRMWARE;
            break;
        case 's':
            outputs |= OUTPUT_SIZE;
            break;
        case 'm':
            outputs |= OUTPUT_MODE;
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
                   "Output device information\n"
                   "\n"
                   "Options:\n"
                   " -p, --pad                  output pad version\n"
                   " -a, --analog               output analog version\n"
                   " -f, --firmware             output firmware version\n"
                   " -m, --mode                 output current operating mode\n"
                   " -s, --size                 output size of stored notes\n"
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

    if (!outputs)
        outputs = (OUTPUT_PAD | OUTPUT_ANALOG | OUTPUT_FIRMWARE | OUTPUT_SIZE | OUTPUT_MODE);

    if (optind != argc) {
        fprintf(stderr, "%s: wrong number of arguments\n",
                program_invocation_name);
        help_and_exit();
    }
}

int main(int argc, char **argv)
{
    struct m210 m210;
    struct m210_info info;
    enum m210_err err;
    int exitval = EXIT_FAILURE;

    parse_args(argc, argv);

    err = m210_open(&m210, NULL);
    if (err) {
        m210_err_printf(err, "m210_open");
        goto err;
    }

    err = m210_get_info(&m210, &info);
    if (err) {
        m210_err_printf(err, "m210_get_info");
        goto err;
    }

    if (outputs & OUTPUT_SIZE) {
        ssize_t data_size;
        err = m210_get_notes_size(&m210, &data_size);
        if (err) {
            m210_err_printf(err, "m210_get_notes_size");
            goto err;
        }
        printf("Size of stored notes: %ld\n", data_size);
    }

    if (outputs & OUTPUT_FIRMWARE)
        printf("Firmware version: %d\n", info.firmware_version);
    if (outputs & OUTPUT_ANALOG)
        printf("Analog version: %d\n", info.analog_version);
    if (outputs & OUTPUT_PAD)
        printf("Pad version: %d\n", info.pad_version);
    if (outputs & OUTPUT_MODE)
        printf("Operating mode: %d\n", info.mode);

    exitval = EXIT_SUCCESS;

  err:
    m210_close(&m210);
    return exitval;
}
