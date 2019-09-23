.PHONY: test connect

ARCH := $(shell uname -p)
ifeq ($(ARCH),unknown)
# Running on Raspberry Pi
DEVELOPMENT := 0
else
DEVELOPMENT := 1
endif

CFLAGS := -O3 -std=c11 -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500
CFLAGS += -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=format -Wshadow -Wswitch
CFLAGS += `pkg-config --cflags cairo`
LDFLAGS := `pkg-config --libs cairo` -lfontconfig

ifeq ($(DEVELOPMENT),1)
CFLAGS += -ggdb3 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer -D_FORTITY_SOURCE=2
TEST_FLAGS += 
endif

SPECIFIC_OBJS := test_display.o
OBJS := \
	cairo.o \
	clock.o \
	framebuffer.o

BINARIES := test_display

all: $(BINARIES)

test_display: test_display.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(SPECIFIC_OBJS)
	rm -f $(BINARIES)
	rm -f out.png output.png

test: all
	./test_display /dev/fb1

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
