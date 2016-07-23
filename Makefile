include commands.mk

OPTS    := -O2
CFLAGS  := -std=c99 $(OPTS) -fPIC -Wall
LDFLAGS :=

SRC  = $(wildcard *.c)
OBJ  = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP  = $(SRC:.c=.d)

TARGETS     = wsn-sniffer-cli wsn-injector-cli wsn-ping-cli pcap-selector

SNIFFER_OBJ  = version.o iobuf.o dump.o help.o mac-display.o mac-decode.o pcap-write.o input.o uart.o wsn-sniffer-cli.o \
               signal-utils.o 802154-parse.o protocol-mqueue.o protocol.o xatoi.o
INJECTOR_OBJ = version.o uart.o getflg.o atoi-gen.o help.o dump.o mac-encode.o mac-decode.o mac-display.o mac-parse.o \
               wsn-injector-cli.o signal-utils.o input.o 802154-parse.o protocol-mqueue.o protocol.o string-utils.o xatoi.o
PING_OBJ     = version.o uart.o help.o protocol.o input.o signal-utils.o wsn-ping-cli.o string-utils.o dump.o crc32.o xatoi.o
SELECTOR_OBJ = version.o help.o pcap-write.o pcap-read.o pcap-list.o iobuf.o dump.o selector.o text-ui.o mac-decode.o \
               string-utils.o mac-display.o xatoi.o

PREFIX  ?= /usr/local
BIN     ?= /bin

CFLAGS += -DVERSION="\"$(shell cat VERSION)\""

commit = $(shell ./hash.sh)
ifneq ($(commit), UNKNOWN)
	CFLAGS += -DCOMMIT="\"$(commit)\""
	CFLAGS += -DPARTIAL_COMMIT="\"$(shell echo $(commit) | cut -c1-8)\""
endif

ifndef DISABLE_DEBUG
CFLAGS += -ggdb
else
CFLAGS += -DNDEBUG=1
endif

CFLAGS += -DUSE_CRC32_C=1

# In FreeBSD systems, sometimes the correct cputype is not picked up.
# We check the log and enable it when it is available.
SSE42_SUPPORT=$(shell $(CC) -march=native -dM -E - < /dev/null | grep SSE4_2)
ifeq ($(SSE42_SUPPORT),)
  SSE42_SUPPORT=$(shell if [ -f /var/run/dmesg.boot ] ; then grep SSE4\.2 /var/run/dmesg.boot ; fi)
endif
ifneq ($(SSE42_SUPPORT),)
	CFLAGS += -msse4.2
endif

.PHONY: all clean

all: $(TARGETS)

wsn-sniffer-cli: $(SNIFFER_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

wsn-injector-cli: $(INJECTOR_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

wsn-ping-cli: $(PING_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) -lm

pcap-selector: $(SELECTOR_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -Wp,-MMD,$*.d -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(DEP)
	$(RM) $(OBJ)
	$(RM) $(CATALOGS)
	$(RM) $(TARGETS)

install:
	$(MKDIR) -p $(DESTDIR)/$(PREFIX)/$(BIN)
	$(INSTALL_PROGRAM) wsn-sniffer-cli $(DESTDIR)/$(PREFIX)/$(BIN)
	$(INSTALL_PROGRAM) wsn-injector-cli $(DESTDIR)/$(PREFIX)/$(BIN)
	$(INSTALL_PROGRAM) wsn-ping-cli $(DESTDIR)/$(PREFIX)/$(BIN)
	$(INSTALL_PROGRAM) pcap-selector $(DESTDIR)/$(PREFIX)/$(BIN)

uninstall:
	$(RM) $(DESTDIR)/$(PREFIX)/wsn-sniffer-cli

-include $(DEP)

