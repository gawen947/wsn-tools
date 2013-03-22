include commands.mk

OPTS    := -O2
CFLAGS  := -std=c99 $(OPTS) -fPIC -Wall
LDFLAGS :=

SRC  = $(wildcard *.c)
OBJ  = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP  = $(SRC:.c=.d)

TARGETS     = wsn-sniffer-cli wsn-injector-cli

SNIFFER_OBJ = version.o iobuf.o dump.o help.o mac-display.o mac-decode.o pcap.o uart-input.o uart.o wsn-sniffer-cli.o
INJECTOR_OBJ = version.o uart.o getflg.o atoi-gen.o help.o mac-decode.o mac-display.o wsn-injector-cli.o

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

.PHONY: all clean

all: $(TARGETS)

wsn-sniffer-cli: $(SNIFFER_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

wsn-injector-cli: $(INJECTOR_OBJ)
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

uninstall:
	$(RM) $(DESTDIR)/$(PREFIX)/wsn-sniffer-cli

-include $(DEP)

