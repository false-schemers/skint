CFLAGS    = -O3 -DNDEBUG
LDFLAGS   = 
PREFIX    = /usr/local
INSTALL   = install -m 755 -s
UNINSTALL = rm -f 
RM        = rm -f
ARCH      = unknown

ifneq ($(shell which clang),)
  $(info clang is detected) 
  CC = clang
else
  CC = gcc
endif

ifeq ($(OS),Windows_NT)
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        ARCH = AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            ARCH = AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            ARCH = IA32
        endif
    endif
else
    UNAME := $(shell uname -p)
    ifeq ($(UNAME),unknown)
        UNAME := $(shell uname -m)
    endif
    ifeq ($(UNAME),x86_64)
        ARCH = AMD64
    endif
    ifneq ($(filter %86,$(UNAME)),)
        ARCH = IA32
    endif
    ifneq ($(filter arm%,$(UNAME)),)
        ARCH = ARM
    endif
    ifeq ($(UNAME),riscv64)
        ARCH = RV64
    endif
endif

ifeq ($(ARCH),AMD64)
  $(info x86_64 architecture is detected)
  CFLAGS += -D NAN_BOXING
endif
ifeq ($(ARCH),RV64)
  $(info RISCV_64 architecture is detected)
  CFLAGS += -D NAN_BOXING
endif

.PHONY: all clean realclean test install uninstall

exe     = ./skint

sources = s.c \
          k.c \
          i.c \
          n.c \
          t.c

includes = i.h \
           n.h \
           s.h

objects = $(sources:%.c=%.o)

all: $(exe)

test:
	$(exe) misc/test.scm

clean:
	$(RM) $(objects)

realclean:
	$(RM) $(objects) $(exe)

install:
	$(INSTALL) $(exe) $(PREFIX)/bin

uninstall:
	$(UNINSTALL) $(exe) $(PREFIX)/bin

$(exe): $(objects)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(objects) -lm

$(objects): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(objects): $(includes)
