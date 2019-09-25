.PHONY: test connect

ARCH := $(shell uname -p)
ifeq ($(ARCH),unknown)
# Running on Raspberry Pi
DEVELOPMENT := 0
else
DEVELOPMENT := 1
endif

CFLAGS := -O3 -std=c11 -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500
CFLAGS += -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=format -Wshadow -Wswitch -pthread
CFLAGS += `pkg-config --cflags cairo`
LDFLAGS := `pkg-config --libs cairo` -lfontconfig
CFLAGS += `pkg-config --cflags yajl`
LDFLAGS += `pkg-config --libs yajl`

ifeq ($(DEVELOPMENT),1)
CFLAGS += -DBUILD_WITH_SDL `pkg-config --cflags sdl2`
LDFLAGS += `pkg-config --libs sdl2`
endif

ifeq ($(DEVELOPMENT),1)
#CFLAGS += -ggdb3 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer -D_FORTITY_SOURCE=2
TEST_FLAGS +=
endif

SPECIFIC_OBJS := cyberblades-ui.o
OBJS := \
	cairo.o \
	display.o \
	display_fb.o \
	cairoglue.o \
	historian.o \
	jsondom.o \
	tools.o \
	isleep.o

ifeq ($(DEVELOPMENT),1)
OBJS +=	display_sdl.o
endif

BINARIES := cyberblades-ui

all: $(BINARIES)

cyberblades-ui: cyberblades-ui.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(SPECIFIC_OBJS)
	rm -f $(BINARIES)
	rm -f out.png output.png

test: all
ifeq ($(DEVELOPMENT),1)
	./cyberblades-ui
else
	./cyberblades-ui /dev/fb0
endif

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
