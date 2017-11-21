##############################################################################
# Path

SRCROOT := $(PWD)
DESTDIR := $(SRCROOT)/out
LIB_PATH   := /usr/lib
INC_PATH   := /usr/include


###############################################################################
CFLAGS  += -DBOARD_$(BOARD)
###############################################################################
# Compile

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib

TARGET	= libdm.a
CFLAGS += -W -Wall
HEADERS = include/dm
SRCS 	= http rest rest_json evlist event
SRCS	+= update update_api util safe safe_iniparser
SRCS	+= dm config package sys version sms
OBJ = $(SRCS:%=build/dm_gcc/%.o)
LINKOBJ = $(OBJ)

SAMPLES = dm
SMPLS = $(SAMPLES:%=build/samples_%)
SMPLOBJ = $(SAMPLES:%=samples/%/main.o)

INCS = -Iinclude
INCS += -Ilibs/iniparser/src
LIBS = -L./ -ldm
LIBS += -Llibs/iniparser -liniparser
ifneq (,$(findstring arm,$(CROSS_COMPILE)))
LIBS += -Llibs/zlib -lz
INCS += -Ilibs/jansson/src
LIBS += -Llibs/jansson/src/.libs -ljansson
INCS += -Ilibs/curl/include
LIBS += -Llibs/curl/lib/.libs -lcurl 
INCS += -Ilibs/openssl/include
LIBS += -Llibs/openssl/usr/lib/ -lssl -lcrypto
else
LIBS += -ljansson -lcurl
endif
LIBDIRS = libs/iniparser

.PHONY: all all-before libs all-after install clean clean-libs

#all: all-before libs $(BIN) samples all-after
all: all-before libs $(TARGET) samples all-after

all-before:
	mkdir -p libs
	@if [ ! -d libs/iniparser ]; then \
		ln -sf ../../../external/iniparser libs/iniparser; fi
	@if [ ! -d libs/zlib ]; then \
		ln -sf ../../../external/zlib-1.2.7 libs/zlib; fi
	@if [ ! -d libs/curl ]; then \
		ln -sf ../../../external/curl-7.30.0 libs/curl; fi
	@if [ ! -d libs/jansson ]; then \
		ln -sf ../../../external/jansson-2.7 libs/jansson; fi
	@if [ ! -d libs/openssl ]; then \
		ln -sf ../../../external/prebuilt-openssl-tl500 libs/openssl; fi
	mkdir -p build/dm_gcc

libs:
	@for dir in $(LIBDIRS) ; do \
		make -C $$dir CC=$(CC) AR=$(AR) || exit $?; \
	done

install:
	fakeroot cp -va $(TARGET) $(DESTDIR)$(LIB_PATH)
	fakeroot cp -va $(HEADERS) $(DESTDIR)$(INC_PATH)

clean: clean-libs
	rm -f $(OBJ) $(TARGET) $(SMPLS) $(SMPLOBJ)

clean-libs:
	@for dir in $(LIBDIRS) ; do \
		if [ -s $$dir/Makefile ]; then \
		make -C $$dir clean; \
		fi; \
	done

remake: clean all

$(TARGET): $(OBJ)
	$(AR) rsc $@ $^
	$(RANLIB) $@

build/dm_gcc/%.o: src/%.c
	$(CC) $(INCS) $(CFLAGS) -c $< -o $@

src/version.c: FORCE
	@git describe --abbrev=4 --dirty --always --tags | awk \
		' BEGIN {print "/* AUTOGENERATE */"}\
		{print "#include <dm/version.h>"}\
		{print "const char* libdm_name = \"libdm\";"}\
		{print "const char* libdm_version = \""$$0"\";"} END{} ' > src/version.c

FORCE:

samples: $(SMPLS)

build/samples_%: samples/%/main.o
	$(CC) $< $(LIBS) -o $@

samples/%/main.o: samples/%/main.c
	$(CC) $(INCS) -c $< -o $@
