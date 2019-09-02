TARGET=app
OBJS=main.o callback.o

INCDIR=
CFLAGS=-G0 -Wall -O2
CXXFLAGS=$(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS=$(CFLAGS)

LIBDIR=
LDFLAGS=
LIBS=-lm

BUILD_PRX=1

EXTRA_TARGETS=EBOOT.PBP
PSP_EBOOT_TITLE=Kristians spel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
