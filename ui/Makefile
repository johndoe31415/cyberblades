.PHONY: test testfb

ARCH := $(shell uname -p)
ifeq ($(ARCH),unknown)
# Running on Raspberry Pi
DEVELOPMENT := 0
else
DEVELOPMENT := 1
endif
BUILD_WITH_SDL := 1

CFLAGS := -O3 -std=c11 -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500
CFLAGS += -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=format -Wshadow -Wswitch -pthread
CFLAGS += `pkg-config --cflags cairo`
LDFLAGS := `pkg-config --libs cairo` -lfontconfig
CFLAGS += `pkg-config --cflags yajl`
LDFLAGS += `pkg-config --libs yajl`

ifeq ($(BUILD_WITH_SDL),1)
CFLAGS += -DBUILD_WITH_SDL `pkg-config --cflags sdl2`
LDFLAGS += `pkg-config --libs sdl2`
#CFLAGS += -DCAIRO_DEBUG
endif

ifeq ($(DEVELOPMENT),1)
#CFLAGS += -ggdb3 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer -D_FORTITY_SOURCE=2
CFLAGS += -DDEVELOPMENT
TEST_FLAGS +=
endif

SPECIFIC_OBJS := cyberblades-ui.o cairo-fonttest.o
OBJS := \
	cairo.o \
	display.o \
	display_fb.o \
	cairoglue.o \
	historian.o \
	jsondom.o \
	tools.o \
	isleep.o \
	signals.o \
	renderer_fullhd.o \
	llist.o

ifeq ($(BUILD_WITH_SDL),1)
OBJS +=	display_sdl.o
endif

BINARIES := cyberblades-ui cairo-fonttest

all: cyberblades-ui 

cyberblades-ui: cyberblades-ui.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cairo-fonttest: cairo-fonttest.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(SPECIFIC_OBJS)
	rm -f $(BINARIES)
	rm -f out.png output.png

test: all
	./cyberblades-ui

testfb:
	./cyberblades-ui /dev/fb0

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
