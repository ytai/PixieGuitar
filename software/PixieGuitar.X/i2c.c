#include "i2c.h"

#include <assert.h>
#include <string.h>
#include <xc.h>

#include <FreeRTOSConfig.h>
#include <portmacro.h>
#include <libpic30.h>

#include "shell.h"
#include "sync.h"
#include "util.h"

typedef enum {
  STATE_IDLE,
  STATE_START,
  STATE_WADDR,
  STATE_WDATA,
  STATE_RESTART,
  STATE_RADDR,
  STATE_RDATA,
  STATE_RACK,
  STATE_STOP
} TransactionState;

static volatile TransactionState state;
static volatile uint8_t addr;
static volatile uint8_t const * write;
static volatile size_t write_count;
static volatile uint8_t * read;
static volatile size_t read_count;
static volatile bool result;
static void * result_queue;

void I2cInit() {
  state = STATE_IDLE;
  result_queue = QueueCreate(1, sizeof(bool));
  _MI2C1IP = configKERNEL_INTERRUPT_PRIORITY;
  _MI2C2IP = configKERNEL_INTERRUPT_PRIORITY + 1;

  I2C2BRG = 165;
  I2C2CONbits.I2CEN = 1;  // Enable.
}

Error I2cWriteRead(uint8_t a,
                   void const * w, size_t wc,
                   void * r, size_t rc) {
  assert(w || !wc);
  assert(r || !rc);
  assert(state == STATE_IDLE);

  addr = a;
  write = (uint8_t const *) w;
  write_count = wc;
  read = (uint8_t *) r;
  read_count = rc;
  result = false;

  _MI2C2IE = 1;
  _MI2C1IE = 1;

  // Assert a start condition to kick off the state machine.
  state = STATE_START;
  I2C2CONbits.SEN = 1;    // Start.

  // Wait for completion.
  bool ret;
  Error e = QueuePopFront(result_queue, &ret);

  _MI2C2IE = 0;  // Disable interrupts.
  _MI2C1IE = 0;
  _MI2C2IF = 0;  // Clear any lingering interrupts.
  _MI2C1IF = 0;
  QueueReset(result_queue);  // Clear any lingering results.
  state = STATE_IDLE;

  if (!I2C2CONbits.I2CEN) {
    // We had to shut-off the module because of a bus collision. Re-enable.
    I2C2CONbits.I2CEN = 1;
  }

  if (e != ERROR_NONE) return e;
  if (!ret) return ERROR_FAILURE;
  return ERROR_NONE;
}

void __attribute__((interrupt, auto_psv)) _MI2C2Interrupt() {
  _MI2C2IF = 0;
  assert(!_MI2C2IF);

  if (I2C2STATbits.BCL) {
    // Bus collision.
    // We're done here. Raise the slave interrupt to notify the thread.
    I2C2CONbits.I2CEN = 0;
    _MI2C1IF = 1;
  } else {
    switch (state) {
      case STATE_IDLE:
        assert(false);
        break;

      case STATE_START:
        assert(!I2C2CONbits.SEN);

        if (read_count && !write_count) {
          // Receive-only transaction.
          I2C2TRN = (addr << 1) | 1;
          state = STATE_RADDR;
        } else {
          // Send-receive transaction.
          I2C2TRN = addr << 1;
          state = STATE_WADDR;
        }
        break;

      case STATE_WADDR:
      case STATE_WDATA:
        assert(!I2C2STATbits.TRSTAT);
        if (I2C2STATbits.ACKSTAT) {
          // NACK.
          I2C2CONbits.PEN = 1;  // Assert a stop condition.
          state = STATE_STOP;
        } else {
          // ACK.
          if (write_count) {
            I2C2TRN = *write++;
            --write_count;
            state = STATE_WDATA;
          } else {
            // Write is done. Should we read?
            if (read_count) {
               I2C2CONbits.RSEN = 1;  // Assert a restart condition.
               state = STATE_RESTART;
            } else {
              // Done.
              result = true;
              I2C2CONbits.PEN = 1;  // Assert a stop condition.
              state = STATE_STOP;
            }
          }
        }
        break;

      case STATE_RESTART:
        assert(!I2C2CONbits.RSEN);
        I2C2TRN = (addr << 1) | 1;
        state = STATE_RADDR;
        break;

      case STATE_RADDR:
        assert(!I2C2STATbits.TRSTAT);
        if (I2C2STATbits.ACKSTAT) {
          // NACK.
          I2C2CONbits.PEN = 1;  // Assert a stop condition.
          state = STATE_STOP;
        } else {
          // ACK.
          I2C2CONbits.RCEN = 1;  // Initiate receive.
          state = STATE_RDATA;
        }
        break;

      case STATE_RDATA:
        assert(!I2C2CONbits.RCEN);
        assert(read_count);
        *read++ = I2C2RCV;
        I2C2CONbits.ACKDT = (read_count == 1);  // ACK all expect the last byte.
        I2C2CONbits.ACKEN = 1;
        --read_count;
        state = STATE_RACK;
        break;

      case STATE_RACK:
        assert(!I2C2CONbits.ACKEN);
        if (read_count) {
          // Have more to read.
          I2C2CONbits.RCEN = 1;  // Initiate receive.
          state = STATE_RDATA;
        } else {
          // Done reading.
          result = true;
          I2C2CONbits.PEN = 1;  // Assert a stop condition.
          state = STATE_STOP;
        }
        break;

      case STATE_STOP:
        assert(!I2C2CONbits.PEN);
        // We're done here. Raise the slave interrupt to notify the thread.
        _MI2C1IF = 1;
        break;
    }
  }
}

void __attribute__((interrupt, no_auto_psv, naked)) _MI2C1Interrupt() {
  portSAVE_CONTEXT();
  _MI2C1IF = 0;
  bool r = result;
  bool pushed = QueuePushBackFromISR(result_queue, &r);
  assert(pushed);
  portRESTORE_CONTEXT();
}

static int I2cScanCommand(int argc, char const ** argv) {
  if (argc != 0) return -1;

  for (uint8_t i = 0; i < 128; ++i) {
    if (I2cWriteRead(i, NULL, 0, NULL, 0) == ERROR_NONE) {
      printf("0x%02x\r\n", i);
    }
  }
  return 0;
}

static int I2cWriteReadCommand(int argc, char const ** argv) {
  if (argc < 2 || argc > 18) return -1;

  uint8_t addr = (uint8_t) parse_hex(argv[0]);
  ++argv;
  --argc;

  size_t readlen = (size_t) parse_number(argv[argc - 1]);
  --argc;

  uint8_t buf[16];
  for (int i = 0; i < argc; ++i) {
    buf[i] = (uint8_t) parse_hex(argv[i]);
  }

  if (ERROR_NONE != I2cWriteRead(addr, buf, argc, buf, readlen)) return -3;

  for (int i = 0; i < readlen; ++i) {
    printf("%02x ", buf[i]);
  }
  printf("\r\n");

  return 0;
}

static int I2cCommand(int argc, char const ** argv) {
  if (argc < 2) return -1;
  char const * subcommand = argv[1];
  argc -= 2;
  argv += 2;
  if (strcmp("scan", subcommand) == 0) {
    return I2cScanCommand(argc, argv);
  }
  if (strcmp("wr", subcommand) == 0) {
    return I2cWriteReadCommand(argc, argv);
  }
  return -2;
}

SHELL_COMMAND(i2c, I2cCommand, "I2C operations.",
              "i2c scan\r\n"
              "i2c wr <addr> [<out>...] <readlen>")
