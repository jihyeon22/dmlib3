CC=gcc
AR=ar

INIT = build/simulinit
SRCS = md5 main
OBJS = $(SRCS:%=build/init_gcc/%.o)

DLPKG = build/dlpkg
DLPKGSRCS = update update_api main file_check gpio
DLPKGSRCS += util ftp_list pack_list syslog_api at_simulator
DLPKGOBJS = $(DLPKGSRCS:%=build/dlpkg_gcc/%.o)

DM = build/dm
DMSRCS = main
DMOBJ = dm/main.o

INCS = -I../include
INCS += -I../libs/iniparser/src
LIBS = -L../ -ldm
LIBS += -L../libs/iniparser -liniparser
LIBS += -ljansson -lcurl

.PHONY: all all-before all-after clean

all: all-before $(INIT) $(DLPKG) $(DM) all-after

all-before:
	mkdir -p build/init_gcc
	mkdir -p build/dlpkg_gcc
	mkdir -p build/dm_gcc

all-after:
	cp dm/dm.ini build

clean:
	rm -f $(OBJS) $(DLPKGOBJS) $(DMOBJ) $(INIT) $(DLPKG) $(DM)
	rm -f build/dm.ini

remake: clean all

$(INIT): $(OBJS)
	$(CC) -o $@ $^

build/init_gcc/%.o: init/%.c
	$(CC) -c $< -o $@

$(DLPKG): $(DLPKGOBJS)
	$(CC) -o $@ $^ -ldl

build/dlpkg_gcc/%.o: dlpkg/%.c
	$(CC) -c $< -o $@

$(DM): dm/main.o
	$(CC) $< $(LIBS) -o $@

dm/main.o: dm/main.c
	$(CC) $(INCS) -c $< -o $@
