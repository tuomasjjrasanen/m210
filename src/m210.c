/* m210 - control Pegasus Tablet Mobile NoteTaker
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

#define _GNU_SOURCE

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libm210/dev.h"
#include "libm210/note.h"

extern char *program_invocation_name;

static const int svg_stroke_width = 20;
static const char *const svg_stroke_color = "black";

static void print_help_hint(void)
{
	fprintf(stderr, "Try `%s --help' for more information.\n",
		program_invocation_name);
}

static void print_version(void)
{
	printf("%s %s\n"
	       "Copyright (C) 2011 Tuomas Jorma Juhani Räsänen\n"
	       "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
	       "This is free software: you are free to change and redistribute it.\n"
	       "There is NO WARRANTY, to the extent permitted by law.\n"
	       "\n"
	       "Written by Tuomas Jorma Juhani Räsänen.\n",
	       PACKAGE_NAME, VERSION);
}

static void print_help(void)
{
	printf("Usage: %s --help\n"
	       "  or:  %s --version\n"
	       "  or:  %s info\n"
	       "  or:  %s dump [--output-file=FILE]\n"
	       "  or:  %s convert [--input-file=FILE] [--output-dir=DIR] [--overwrite]\n"
	       "  or:  %s delete\n"
	       "\n"
	       "Download notes from Pegasus Tablet Mobile NoteTaker (M210) and\n"
	       "convert them to SVG files.\n"
	       "\n"
	       "Options:\n"
	       " --help                 display this help and exit\n"
	       " --version              output version information and exit\n"
	       "\n"
	       "Dump options:\n"
	       "    --output-file=FILE  defaults to standard output\n"
	       "\n"
	       "Convert options:\n"
	       "    --input-file=FILE   defaults to standard input\n"
	       "    --output-dir=DIR    directory for SVG files,\n"
	       "                        defaults to current directory\n"
	       "    --overwrite         overwrite existing SVG files\n"
	       "\n"
	       "Examples:\n"
	       "Download notes to a file:\n"
	       "  m210 dump > notes\n"
	       "\n"
	       "Convert downloaded notes to SVG files:\n"
	       "  m210 convert < notes\n"
	       "\n"
	       "Erase notes from the device's memory:\n"
	       "  m210 delete\n"
	       "\n"
	       "Display device information:\n"
	       "  m210 info\n"
	       "\n"
	       "Report bugs to <%s>\n"
	       "Homepage: <%s>\n"
	       "\n",
	       program_invocation_name, program_invocation_name,
	       program_invocation_name, program_invocation_name,
	       program_invocation_name, program_invocation_name,
	       PACKAGE_BUGREPORT, PACKAGE_URL);
}

static FILE* open_svg_file(int note_number, char *output_mode)
{
	FILE *file = NULL;
	char *filename = NULL;

	if (asprintf(&filename, "m210_note_%d.svg", note_number) == -1) {
		/* On error, asprintf() leaves the contents of
		 * filename undefined. It needs to be NULLed to safely
		 * call free(). */
		filename = NULL;
		goto out;
	}

	file = fopen(filename, output_mode);
out:
	free(filename);
	return file;
}

static int note_to_svg(FILE *input_file, char *output_mode) {
	int result = -1;
	FILE *output_file = NULL;
	struct m210_note_head head;
	enum m210_err err;
	int bodyi;
	int has_path = 0;

	err = m210_note_read_head(&head, input_file);
	if (err) {
		m210_err_perror(err, "error: failed to read note head");
		goto out;
	}

	if (head.number == 0) {
		/* End of note stream. */
		result = 0;
		goto out;
	}

	output_file = open_svg_file(head.number, output_mode);
	if (output_file == NULL) {
		perror("error: failed to create SVG file");
		goto out;
	}

	fprintf(output_file, "%s\n", "<?xml version=\"1.0\"?>");
	fprintf(output_file, "%s\n", "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">");
	fprintf(output_file, "%s\n", "<svg width=\"210mm\" height=\"297mm\" viewBox=\"-7000 0 14000 20000\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">");

	for (bodyi = 0; bodyi < head.bodyc; ++bodyi) {
		struct m210_note_body body;
		err = m210_note_read_body(&body, input_file);
		if (err) {
			m210_err_perror(err, "error: failed to read note body");
			goto out;
		}
		if (body.pressure) {
			if (!has_path) {
				fprintf(output_file,
					"<polyline stroke-width=\"%d\" "
					"stroke=\"%s\" fill=\"none\" points=\"",
					svg_stroke_width, svg_stroke_color);
				has_path = 1;
			}
			fprintf(output_file, "%d,%d ", body.x, body.y);
		} else {
			fprintf(output_file, "%s\n", "\" />");
			has_path = 0;
		}
	}

	if (fprintf(output_file, "%s", "</svg>\n") < 0) {
		perror("error: failed to write to output file");
		goto out;
	}

	result = 1;
out:
	if (output_file && output_file != stdout && fclose(output_file)) {
		perror("error: failed to close output file");
		result = -1;
	}
	return result;
}

static int convert_cmd(int argc, char **argv)
{
	int result = -1;
	FILE *input_file = NULL;
	char *output_mode = "wx";
	const struct option opts[] = {
		{"input-file", required_argument, NULL, 'i'},
		{"output-dir", required_argument, NULL, 'd'},
		{"overwrite", no_argument, NULL, 'f'},
		{0, 0, 0, 0}
	};

	input_file = stdin;

	while (1) {
		int option = getopt_long(argc, argv, "+", opts, NULL);

		if (option == -1) {
			break;
		}

		switch (option) {
		case 'i':
			input_file = fopen(optarg, "rb");
			if (input_file == NULL) {
				perror("error: failed to open input file");
				goto out;
			}
			break;
		case 'd':
			if (chdir(optarg)) {
				perror("error: failed to change the current "
				       "working directory");
				goto out;
			}
			break;
		case 'f':
			output_mode = "w";
			break;
		default:
			print_help_hint();
			goto out;
		}
	}

	if (optind != argc) {
		fprintf(stderr, "error: unexpected convert arguments\n");
		print_help_hint();
		goto out;
	}

	while (1) {
		result = note_to_svg(input_file, output_mode);
		if (result == -1) {
			goto out;
		} else if (result == 0) {
			break;
		}
	}

out:
	if (input_file && input_file != stdin && fclose(input_file)) {
		perror("failed to close input file");
		result = -1;
	}
	return result;
}

static int delete_cmd(int argc, char **argv)
{
	int result = -1;
	m210_dev dev;
	enum m210_err err;
	const struct option opts[] = {
		{0, 0, 0, 0}
	};

	while (1) {
		int option = getopt_long(argc, argv, "+", opts, NULL);

		if (option == -1) {
			break;
		}

		switch (option) {
		default:
			print_help_hint();
			goto out;
		}
	}

	if (optind != argc) {
		fprintf(stderr, "error: unexpected delete arguments\n");
		print_help_hint();
		goto out;
	}

	err = m210_dev_connect(&dev);
	if (err) {
		m210_err_perror(err, "failed to open device");
		goto out;
	}

	err = m210_dev_delete_notes(dev);
	if (err) {
		m210_err_perror(err, "failed to delete notes");
		goto out;
	}
	result = 0;
out:
	if (dev) {
		err = m210_dev_disconnect(&dev);
		if (err) {
			m210_err_perror(err, "error: failed to disconnect");
			result = -1;
		}
	}
	return result;
}

static int dump_cmd(int argc, char **argv)
{
	int result = -1;
	m210_dev dev;
	FILE *output_file = NULL;
	enum m210_err err;
	const struct option opts[] = {
		{"output-file", required_argument, NULL, 'o'},
		{0, 0, 0, 0}
	};

	output_file = stdout;

	while (1) {
		int option = getopt_long(argc, argv, "+", opts, NULL);

		if (option == -1) {
			break;
		}

		switch (option) {
		case 'o':
			output_file = fopen(optarg, "wb");
			if (output_file == NULL) {
				perror("error: failed to open output file");
				goto out;
			}
			break;
		default:
			print_help_hint();
			goto out;
		}
	}

	if (optind != argc) {
		fprintf(stderr, "error: unexpected dump arguments\n");
		print_help_hint();
		goto out;
	}

	err = m210_dev_connect(&dev);
	if (err) {
		m210_err_perror(err, "failed to open device");
		goto out;
	}

	err = m210_dev_download_notes(dev, output_file);
	if (err) {
		m210_err_perror(err, "failed to download notes");
		goto out;
	}

	result = 0;
out:
	if (dev) {
		err = m210_dev_disconnect(&dev);
		if (err) {
			m210_err_perror(err, "error: failed to disconnect");
			result = -1;
		}
	}

	if (output_file && output_file != stdout && fclose(output_file)) {
		perror("failed to close output file");
		result = -1;
	}
	return result;
}

static int info_cmd(int argc, char **argv)
{
	int result = -1;
	m210_dev dev;
	enum m210_err err;
	struct m210_dev_info info;
	const char *device_mode;
	const struct option opts[] = {
		{0, 0, 0, 0}
	};

	while (1) {
		int option = getopt_long(argc, argv, "+", opts, NULL);

		if (option == -1) {
			break;
		}

		switch (option) {
		default:
			print_help_hint();
			goto out;
		}
	}

	if (optind != argc) {
		fprintf(stderr, "error: unexpected info arguments\n");
		print_help_hint();
		goto out;
	}

	err = m210_dev_connect(&dev);
	if (err) {
		m210_err_perror(err, "failed to open device");
		goto out;
	}

	err = m210_dev_get_info(dev, &info);
	if (err) {
		m210_err_perror(err, "failed to get information");
		goto out;
	}

	switch (info.mode) {
	case M210_DEV_MODE_MOUSE:
		device_mode = "MOUSE";
		break;
	case M210_DEV_MODE_TABLET:
		device_mode = "TABLET";
		break;
	default:
		device_mode = "UNKNOWN";
		break;
	}

	printf("Mode:		 %s\n", device_mode);
	printf("Used memory:	 %d bytes\n", info.used_memory);
	printf("Pad version:	 %d\n", info.pad_version);
	printf("Analog version:	 %d\n", info.analog_version);
	printf("Firmare version: %d\n", info.firmware_version);

	result = 0;
out:
	if (dev) {
		err = m210_dev_disconnect(&dev);
		if (err) {
			m210_err_perror(err, "error: failed to disconnect");
			result = -1;
		}
	}
	return result;
}

int main(int argc, char **argv)
{
	int cmd_argc;
	char **cmd_argv;
	int exitval = EXIT_FAILURE;
	const char *cmd;
	int (*cmdfn)(int, char**);
	const struct option opts[] = {
		{"help",    no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'v'},
		{0, 0, 0, 0}
	};

	while (1) {
		int option = getopt_long(argc, argv, "+", opts, NULL);
		if (option == -1) {
			break;
		}
		switch (option) {
		case 'h':
			print_help();
			exitval = EXIT_SUCCESS;
			goto out;
		case 'v':
			print_version();
			exitval = EXIT_SUCCESS;
			goto out;
		default:
			print_help_hint();
			goto out;
		}
	}

	if (optind == argc) {
		fprintf(stderr, "error: command is missing\n");
		print_help_hint();
		goto out;
	}

	cmd = argv[optind];

	cmd_argc = argc - optind;
	cmd_argv = argv + optind;
	optind = 0;

	if (strcmp(cmd, "info") == 0) {
		cmdfn = &info_cmd;
	} else if (strcmp(cmd, "dump") == 0) {
		cmdfn = &dump_cmd;
	} else if (strcmp(cmd, "convert") == 0) {
		cmdfn = &convert_cmd;
	} else if (strcmp(cmd, "delete") == 0) {
		cmdfn = &delete_cmd;
	} else {
		fprintf(stderr, "error: unknown command '%s'\n", cmd);
		print_help_hint();
		goto out;
	}

	if (cmdfn(cmd_argc, cmd_argv) == -1) {
		fprintf(stderr, "error: %s failed\n", cmd);
		goto out;
	}

	exitval = EXIT_SUCCESS;
out:
	return exitval;
}
