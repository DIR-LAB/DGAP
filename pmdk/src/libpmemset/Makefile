
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation

#
# src/libpmemset/Makefile -- Makefile for libpmemset
#

include ../common.inc

LIBRARY_NAME = pmemset
LIBRARY_SO_VERSION = 1
LIBRARY_VERSION = 0.0
SOURCE =\
	badblock.c\
	config.c\
	errormsg.c\
	file.c\
	file_posix.c\
	libpmemset.c\
	map_config.c\
	part.c\
	pmemset.c\
	pmemset_utils.c\
	sds.c\
	source.c

include ../core/pmemcore.inc
include ../Makefile.inc

CFLAGS += -I.
LIBS += -pthread -lpmem2
