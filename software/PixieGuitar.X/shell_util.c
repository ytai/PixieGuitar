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

#include "shell_util.h"

/******************************************************************************/
int8_t parse_hex_nibble(char byte) {
  if (byte >= '0' && byte <= '9') return (byte - '0');
  if (byte >= 'a' && byte <= 'f') return (byte - 'a' + 10);
  if (byte >= 'A' && byte <= 'F') return (byte - 'A' + 10);

  return -1;
}

/******************************************************************************/
uint32_t parse_hex(char const * string) {
  uint32_t result = 0;

  uint8_t i;
  for (i = 0; string[i] != '\0'; ++i) {
    uint8_t byte = string[i];
    int8_t nibble = parse_hex_nibble(byte);

    if (byte == ' ')
      continue;

    if (nibble >= 0) {
      result <<= 4;
      result |= (uint8_t) nibble;
      continue;
    }

    break;
  }

  return result;
}

/******************************************************************************/
uint32_t parse_dec(char const * string) {
  uint32_t result = 0;

  uint8_t i;
  for (i = 0; string[i] != '\0'; ++i) {
    uint8_t byte = string[i];

    if (byte == ' ')
      continue;

    if (byte >= '0' && byte <= '9') {
      result *= 10;
      result += byte - '0';
      continue;
    }

    break;
  }

  return result;
}

/******************************************************************************/
uint32_t parse_oct(char const * string) {
  uint32_t result = 0;

  uint8_t i;
  for (i = 0; string[i] != '\0'; ++i) {
    uint8_t byte = string[i];

    if (byte == ' ')
      continue;

    if (byte >= '0' && byte <= '7') {
      result <<= 3;
      result |= byte - '0';
      continue;
    }

    break;
  }

  return result;
}

/******************************************************************************/
uint32_t parse_number(char const * string) {
  if (string[0] == '0' && string[1] == 'x')
    return parse_hex(string + 2);
  else if (string[0] == '0')
    return parse_oct(string + 1);
  else
    return parse_dec(string);
}
/******************************************************************************/
