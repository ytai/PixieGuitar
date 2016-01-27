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

#ifndef __shell_h__
#define __shell_h__

#include <stdint.h>
#include <stdio.h>

typedef int (*ShellCommandFunction)(int argc, const char **argv);

typedef struct {
  const char name[10];
  ShellCommandFunction function;
  char const * description;
  char const * usage;
} ShellCommand;

typedef struct {
  int argv_length;
  char const **argv;
} ShellConfig;

#define SHELL_COMMAND(name, func, desc, usage)    \
    static char const _##name##_desc[] = desc;    \
    static char const _##name##_usage[] = usage;  \
    ShellCommand _##name##_command          \
        __attribute__((section(".commands"))) = { \
      #name,                                      \
      func,                                       \
      _##name##_desc,                             \
      _##name##_usage                             \
    };

#define SHELL_CONFIG(ARGV)                 \
    static char const * shell_argv[ARGV];	 \
                                           \
    static ShellConfig shell_config = {    \
        ARGV,                              \
        shell_argv,                        \
    };

/*
 * The shell_run_command function parses a command line and calls the
 * command that is parsed as the first token in the command line.  Failure to
 * find the specified command or failure to successfully run the specified
 * command will result in usage information being printed on the output stream.
 *
 * Returns:    0 Command executed successfully
 *          -127 Failure to parse command line
 *          -128 Failure to find a command
 *          -xxx Status code from failed command execution
 */
int shell_run_command(char * command_line,
    ShellConfig const * config);


#endif //__shell_h__
