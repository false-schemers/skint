-include  prefix.mk
PREFIX    ?= /usr/local
LIBROOT   = $(PREFIX)/share/skint
CFLAGS    = -O3 -DNDEBUG
LDFLAGS   = 
MKPATH    = install -d
INSTALL   = install -m 755 -s
CPR       = cp -a
UNINSTALL = rm -f 
RM        = rm -f
RMR       = rm -rf
ARCH      = unknown
VERSION   = $(shell sed -n 's/(define \*skint-version\* "\([0-9.]*\)")/\1/p' pre/t.scm)
DISTNAME  = skint-$(VERSION)
DISTDIR   = dist/$(DISTNAME)

$(info version is $(VERSION))
$(info prefix is set to $(PREFIX))

ifneq ($(wildcard lib/.),)
  CFLAGS += -DLIBDIR=$(LIBROOT)/lib
endif

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
    endif
else 
    ifeq ($(shell uname),Darwin)
        UNAME := $(shell uname -m)
        ifeq ($(UNAME),x86_64)
            ARCH = AMD64
        endif
        ifeq ($(UNAME),arm64)
            ARCH = ARM64
        endif
    else
        UNAME := $(shell uname -p)
        ifeq ($(UNAME),unknown)
            UNAME := $(shell uname -m)
        endif
        ifeq ($(UNAME),x86_64)
            ARCH = AMD64
        endif
        ifeq ($(UNAME),ppc64le)
            ARCH = POWER64
        endif
        ifeq ($(UNAME),riscv64)
            ARCH = RV64
        endif
    endif
endif

ifeq ($(ARCH),AMD64)
  $(info X86_64 architecture is detected)
  CFLAGS += -D NAN_BOXING
endif
ifeq ($(ARCH),ARM64)
  $(info ARM_64 architecture is detected)
  CFLAGS += -D NAN_BOXING
endif
ifeq ($(ARCH),POWER64)
  $(info POWER_64 architecture is detected)
  CFLAGS += -D NAN_BOXING
endif
ifeq ($(ARCH),RV64)
  $(info RISCV_64 architecture is detected)
  CFLAGS += -D NAN_BOXING
endif

.PHONY: all clean realclean dist distclean test libtest install libinstall uninstall libuninstall

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
	$(exe) test/test.scm

libtest:
	$(exe) test/srfi/all.scm

clean:
	$(RM) $(objects)
	$(RM) tmp1

realclean:
	$(RM) $(objects) $(exe)
	$(RM) tmp1 prefix.mk
	$(RMR) dist

distclean: realclean

install:
	$(MKPATH) $(PREFIX)/bin
	$(INSTALL) $(exe) $(PREFIX)/bin

libinstall:
	$(MKPATH) $(LIBROOT)/lib
	$(CPR) lib/. $(LIBROOT)/lib/
	find $(LIBROOT) -type f -exec chmod 644 {} +
	find $(LIBROOT) -type d -exec chmod 755 {} +

uninstall:
	$(UNINSTALL) $(exe) $(PREFIX)/bin

libuninstall:
	$(RMR) $(LIBROOT)

$(exe): $(objects)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(objects) -lm

$(objects): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(objects): $(includes)

dist:   dist/$(DISTNAME).tar.gz
dist/$(DISTNAME).tar.gz: $(DISTDIR)
	tar czf $@ -C dist $(DISTNAME)

$(DISTDIR): Makefile
	mkdir -p $@
	cp -r pre misc lib test *.c *.h LICENSE configure Makefile $@

