/*
 * Copyright © 2019, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <glib.h>

#include "slipcat.h"
#include "libslipcat.h"

#define is(_a, _b) (strcmp((_a), (_b)) == 0)

static char *input, *output, *expected_output;

void line_parse(char *line)
{
	GVariant *v_in, *v_ex, *v = g_variant_parse(G_VARIANT_TYPE("a{ss}"),
							line, NULL, NULL, NULL);
	P("%s", g_variant_print(v, TRUE));

	if (v_in = g_variant_lookup_value(v, "input", G_VARIANT_TYPE("s"))) {

		P("input: %s", g_variant_print(v_in, TRUE));

		g_variant_get(v_in, "s", &input);
	}

	if (v_ex = g_variant_lookup_value(v, "expected_output",
					G_VARIANT_TYPE("s"))) {

		P("expected_output: %s", g_variant_print(v_ex, TRUE));

		g_variant_get(v_ex, "s", &expected_output);
	}
}

void test_load(FILE *f)
{
	char *line = NULL;
	size_t line_len = 0;
	ssize_t chars_read;

	while ((chars_read = getline(&line, &line_len, f)) > 0) {

		P("line: %s", line);

		if (line[0] != '#') {
			line_parse(line);
		}

		free(line);
		line = NULL;
		line_len = 0;
	}

	if (chars_read < 0 && errno) {
		_E("getline");
	}
}

int main(int argc, char *argv[])
{
	int exit_status = EXIT_FAILURE;
	FILE *fp = (argc > 1) ? fopen(argv[1], "r") : stdin;

	test_load(fp);

	P("input: '%s', expected_output: '%s'", input, expected_output);

	{
		struct sockaddr_in *s_in = s_in_new(input);

		output = g_strdup_printf("%s:%hu", inet_ntoa(s_in->sin_addr),
						ntohs(s_in->sin_port));
		if (is(output, expected_output)) {
			exit_status = EXIT_SUCCESS;
		}
	}

end_test:
	P("TEST: %s", exit_status == EXIT_SUCCESS ? "PASSED" : "FAILED");

	return exit_status;
}
