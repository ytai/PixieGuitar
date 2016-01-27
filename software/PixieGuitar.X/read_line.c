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

#include "read_line.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/******************************************************************************/
typedef enum {
  read_line_normal,
  read_line_escape,
  read_line_control
} ReadLineMode;

static int _read_line(char * line, int length,
    char * history, int history_length) {
  int history_index = 0;
  int index = 0;
  int max = 0;
  ReadLineMode mode = read_line_normal;

  while (1) {
    int c = getchar();

    if (c == EOF) return -1;

    switch (mode) {
      case read_line_control:
      {
        switch (c) {
          case 'A':
            if (history_index < history_length) {
              int i;
              for (i = 0; i < index; ++i)
                printf("%c", '\b');

              for (i = 0; i < max; ++i)
                printf("%c", ' ');

              for (i = 0; i < max; ++i)
                printf("%c", '\b');

              for (index = 0; index < length; ++index) {
                line[index] = history[length *
                    history_index +
                    index];

                if (line[index] == '\0')
                  break;

                printf("%c", line[index]);
              }

              max = index;
              history_index++;
            }
            break;

          case 'B':
            if (history_index > 0) {
              int i;
              history_index--;

              for (i = 0; i < index; ++i)
                printf("%c", '\b');

              for (i = 0; i < max; ++i)
                printf("%c", ' ');

              for (i = 0; i < max; ++i)
                printf("%c", '\b');

              if (history_index == 0) {
                index = 0;
                line[0] = '\0';
              } else {
                for (index = 0; index < length; ++index) {
                  line[index] = history[length *
                      (history_index - 1) +
                      index];

                  if (line[index] == '\0')
                    break;

                  printf("%c", line[index]);
                }
              }

              max = index;
            }
            break;

          case 'C':
            if (index < max) {
              printf("%c", line[index]);
              ++index;
            }
            break;

          case 'D':
            if (index > 0) {
              printf("%c", '\b');
              --index;
            }
            break;
        }

        mode = read_line_normal;
        break;
      }

      case read_line_escape:
      {
        if (c == '[') mode = read_line_control;
        else mode = read_line_normal;

        break;
      }

      case read_line_normal:
      {
        if ((c == '\r') || (c == '\n')) {
          line[max] = '\0';
          puts("");
          return 0;
        }

        if ((c == 0x7f) || (c == '\b')) {
          if (index > 0) {
            /*
             * Move all characters from index to the end of the
             * line back one space.
             */
            int i;
            for (i = 0; i < (max - index); ++i)
              line[index - 1 + i] = line[index + i];

            --index;
            --max;

            /*
             * Fix up the displayed text, making sure to add an
             * extra space at the end to cover up what was there.
             */
            printf("%c", '\b');

            for (i = 0; i < (max - index); ++i)
              printf("%c", line[index + i]);

            printf("%c", ' ');

            /*
             * And move the cursor back into position.
             */
            for (i = 0; i < (max - index + 1); ++i)
              printf("%c", '\b');
          }
        } else if (c == 0x1b) {
          mode = read_line_escape;
          continue;
        } else if (c < 0x20) {
          continue;
        } else if (max < (length - 1)) {
          /*
           * Move all characters from index to the end of the line
           * forward one space.
           */
          int i;
          for (i = max - index - 1; i >= 0; --i)
            line[index + 1 + i] = line[index + i];

          line[index] = c;

          ++max;

          for (i = 0; i < (max - index); ++i)
            printf("%c", line[index + i]);

          ++index;

          for (i = 0; i < (max - index); ++i)
            printf("%c", '\b');
        } else {
          printf("%c", '\a');
        }
        break;
      }
    }
  }
}

/******************************************************************************/
void read_line_init(ReadLineConfig * config) {
  config->line[0] = '\0';

  int i;
  for (i = 0; i < config->history_length; ++i)
    config->history[i * config->line_length] = '\0';
}

/******************************************************************************/
char * read_line(ReadLineConfig * config, char const * prompt) {
  int line_length = config->line_length;
  char * line = config->line;
  int history_length = config->history_length;
  char * history = config->history;

  if (prompt) {
    fputs(prompt, stdout);
    fflush(stdout);
  }

  if (0 != _read_line(line, line_length, history, history_length)) {
    clearerr(stdin);
    return NULL;
  }

  /*
   * Move all command lines down one place in history and copy the current
   * command line into the head of the history buffer.
   */
  memmove(history + line_length, history, line_length * (history_length - 1));
  memmove(history, line, line_length);

  return line;
}
/******************************************************************************/
