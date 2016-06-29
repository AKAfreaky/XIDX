#!/usr/bin/make
# Makefile to build the XIDX program from source 
# on POSIX systems. 
#
# XIDX sources are modified from the original created by `Vercingetorix'.
# (Modified) sources and this makefile are made available under the GPL v. 2
# or later. See the ./licence file for a copy of the terms and conditions.
# 
# --Tellos Athenaios

SRC = src/main.cpp src/idx.cpp src/shared.cpp
OUT = bin/xidx
MINGW_TARGET ?= yes
mingw_i586 = $(shell which i586-mingw32msvc-g++)
mingw_i686 = $(shell which i686-w64-mingw32-g++)
mingw_amd64 = $(shell which x86_64-w64-mingw32-g++)
gpp = $(shell which g++)
CFLAGS = -Wall -Os -s
STATICS= -static -static-libgcc -static-libstdc++

build : bin $(SRC)
ifneq "$(gpp)" ""
	$(gpp) $(CFLAGS) $(SRC) -o $(OUT)
else
	@echo "Skipping compiler: G++ (Native)"
endif
ifeq "$(MINGW_TARGET)" "yes"
ifneq "$(mingw_amd64)" ""
	$(mingw_amd64) $(CFLAGS) $(STATICS) $(SRC) -o $(OUT)_amd64.exe
else
	@echo "Skipping compiler: (MinGW 64bit AMD64)"
endif

ifneq "$(mingw_i686)" ""
ifeq "$(mingw_i586)" ""
	$(mingw_i686) $(CFLAGS) $(STATICS) $(SRC) -o $(OUT)_x86.exe
else
	@echo "Skipping compiler: (MinGW 32bit i686). Using i586 instead."
endif
else
	@echo "Skipping compiler: (MinGW 32bit i686)"
endif

ifneq "$(mingw_i586)" ""
	$(mingw_i586) $(CFLAGS) $(STATICS) $(SRC) -o $(OUT)_x86.exe
else
	@echo "Skipping compiler: (MinGW 32bit i586)"
endif
else
	@echo "Skip cross compiler (MinGW) targets"
endif
bin :
	mkdir bin

clean :
	rm -rf bin
