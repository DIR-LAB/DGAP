# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation

#
# src/test/pmemset_event/Makefile -- build pmemset_event test
#
TOP = ../../..

vpath %.c $(TOP)/src/test/unittest
vpath %.c $(TOP)/src/libpmemset

INCS += -I$(TOP)/src/libpmemset
TARGET = pmemset_event
OBJS = pmemset_event.o \
	ut_pmemset_utils.o

LIBPMEMSET=internal-debug

include ../Makefile.inc
