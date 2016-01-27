#ifndef I2C_H
#define	I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "error.h"

void I2cInit();

Error I2cWriteRead(uint8_t addr,
                   void const * write, size_t write_count,
                   void * read, size_t read_count);

#endif	/* I2C_H */

