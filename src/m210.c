/* m210 - Pegasus Tablet Mobile NoteTaker (M210) Controller
 * Copyright © 2011 Tuomas Jorma Juhani Räsänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char *program_invocation_name;

void print_error_and_exit(char const *const msg)
{
	if (msg) {
		fprintf(stderr, "%s: %s\n", program_invocation_name, msg);
	}
        fprintf(stderr, "Try `%s --help' for more information.\n",
                program_invocation_name);
        exit(EXIT_FAILURE);
}

void print_version_and_exit(void)
{
	printf("%s %s\n"
	       "Copyright © 2011 Tuomas Jorma Juhani Räsänen\n"
	       "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
	       "This is free software: you are free to change and redistribute it.\n"
	       "There is NO WARRANTY, to the extent permitted by law.\n"
	       "\n"
	       "Written by Tuomas Jorma Juhani Räsänen.\n",
	       PACKAGE_NAME, VERSION);
	exit(EXIT_SUCCESS);
}

void print_help_and_exit(void)
{
	printf("Usage: %s --help\n"
	       "       %s --version\n"
	       "       %s info\n"
	       "       %s dump [--output-file=<file>]\n"
	       "       %s convert [--input-file=<file>] [--output-dir=<dir>] [--overwrite]\n"
	       "       %s delete\n"
	       "\n"
	       "Pull notes from Pegasus Tablet Mobile NoteTaker (M210) and\n"
	       "convert them to SVG files.\n"
	       "\n"
	       "General options:\n"
	       " --help                   display this help and exit\n"
	       " --version                output version information and exit\n"
	       "\n"
	       "Commands:\n"
	       "  info                    show device information\n"
	       "\n"
	       "  dump                    dump notes as a binary stream\n"
	       "    --output-file=<file>  defaults to standard output\n"
	       "\n"
	       "  convert                 convert notes to SVG files\n"
	       "    --input-file=<file>   defaults to standard input\n"
	       "    --output-dir=<dir>    directory where files will be\n"
	       "                          written to, defaults to\n"
	       "    --overwrite           overwrite existing files\n"
	       "\n"
	       "  delete                  delete all notes from the device\n"
	       "\n"
	       "Report bugs to <%s>\n"
	       "Homepage: <%s>\n"
	       "\n",
	       program_invocation_name, program_invocation_name,
	       program_invocation_name, program_invocation_name,
	       program_invocation_name, program_invocation_name,
	       PACKAGE_BUGREPORT, PACKAGE_URL);
	exit(EXIT_SUCCESS);
}

/* struct dump_options { */
/* 	FILE *output_file; */
/* }; */

/* enum format { */
/* 	SVG */
/* }; */

/* struct convert_options { */
/* 	FILE *input_file; */
/* 	enum format output_format; */
/* 	int overwrite; */
/* }; */

/* void parse_convert_options(); */

/* void parse_dump_options(); */

/* void parse_command(int argc, char **argv); */

void parse_general_options(int argc, char **argv)
{
        const struct option options[] = {
                {"help",          no_argument,       NULL, 'h'},
                {"version",       no_argument,       NULL, 'v'},
                {0, 0, 0, 0}
        };

        while (1) {
                int option = getopt_long(argc, argv, "", options, NULL);
		if (option == -1) {
			break;
		}
		switch (option) {
		case 'h':
			print_help_and_exit();
 		case 'v':
			print_version_and_exit();
		default:
			print_error_and_exit(NULL);
		}
        }
}

int main(int argc, char **argv)
{
	parse_general_options(argc, argv);
	return EXIT_SUCCESS;
}
