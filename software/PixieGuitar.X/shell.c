/*
 * Copyright (c) 2012, Anton Staaf
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the project nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "shell.h"
#include "shell_util.h"

/******************************************************************************/
extern ShellCommand const _commands_start;
extern ShellCommand const _commands_end;

static ShellCommand const * const shell_command_table = &_commands_start;
#define shell_command_count (&_commands_end - &_commands_start)

/******************************************************************************/
static void shell_usage() {
  puts("Valid commands are:");

  int i;
  for (i = 0; i < shell_command_count; ++i) {
    char const * description = shell_command_table[i].description;

    printf("\t%s\t%s\n",
        shell_command_table[i].name,
        description ? description : "");
  }
}

/******************************************************************************/
static void parse_line(char * line,
    int * argc,
    char const ** argv,
    int argv_length) {
  int write_index = 0;
  int index = 0;
  bool quoted = false;
  bool escaped = false;
  bool scanning = true;

  int read_index;
  for (read_index = 0; line[read_index] != '\0'; ++read_index) {
    if (scanning) {
      /*
       * Record this argument into the argv array.
       */
      if (index < argv_length) {
        argv[index] = line + write_index;
        ++index;
      }

      scanning = false;

      /*
       * Scan to the beginning of the next argument or the end of the
       * string.
       */
      while (isspace((int) line[read_index]) &&
          line[read_index] != '\0')
        ++read_index;

      if (line[read_index] == '\0')
        break;
    }

    if (quoted) {
      if (escaped) {
        switch (line[read_index]) {
          case 'r': line[write_index++] = '\r';
            break;
          case 'n': line[write_index++] = '\n';
            break;
          case 't': line[write_index++] = '\t';
            break;
          default: line[write_index++] = line[read_index];
            break;
        }

        escaped = false;
      } else {
        switch (line[read_index]) {
          case '"':
            quoted = false;
            break;

          case '\\':
            escaped = true;
            break;

          default:
            line[write_index++] = line[read_index];
            break;
        }
      }
    } else {
      switch (line[read_index]) {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
          line[write_index++] = '\0';
          scanning = true;
          break;

        case '"':
          quoted = true;
          break;

        default:
          line[write_index++] = line[read_index];
          break;
      }
    }
  }

  line[write_index] = '\0';

  *argc = index;
}

/******************************************************************************/
static int lookup_command(const char * command) {
  int i;
  for (i = 0; i < shell_command_count; ++i) {
    if (strcmp(command, shell_command_table[i].name) == 0) {
      return i;
    }
  }

  return -1;
}

/******************************************************************************/
int shell_run_command(char * command_line,
    ShellConfig const * config) {
  int argv_length = config->argv_length;
  char const * * argv = config->argv;
  int argc = 0;
  int status = -1;
  int index = -1;

  parse_line(command_line, &argc, argv, argv_length);

  if (argc <= 0)
    return -127;

  index = lookup_command(argv[0]);

  if (index < 0) {
    printf("Command not found: %s\n", argv[0]);

    shell_usage();

    return -128;
  }

  status = shell_command_table[index].function(argc, argv);

  if (status < 0) {
    char const * usage = shell_command_table[index].usage;

    printf("Command failed (%d).\n\n", status);

    if (usage) {
      printf("Command usage:\n%s\n",
          shell_command_table[index].usage);
    }
  }

  return status;
}
/******************************************************************************/

static int help(int argc, const char **argv) {
  int i;
  switch (argc) {
    case 1:
      shell_usage();
      break;

    case 2:
      i = lookup_command(argv[1]);
      if (i < 0) {
        printf("Command not found: %s\n", argv[1]);
        shell_usage();
      } else {
        char const * description = shell_command_table[i].description;
        char const * usage = shell_command_table[i].usage;
        if (description) {
          printf("Description:\n%s\n", description);
        }
        if (usage) {
          printf("Usage:\n%s\n", usage);
        }
      }
      break;

    default:
      return -1;
  }
  return 0;
}

SHELL_COMMAND(help, help, "Print this help information.", "help [command]")
