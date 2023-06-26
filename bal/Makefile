# See LICENSE.txt for license details.

CXX_FLAGS += -std=c++11 -O3 -Wall
LDLIBS = -lpmemobj -lpmem
PAR_FLAG = -fopenmp

ifneq (,$(findstring icpc,$(CXX)))
	PAR_FLAG = -openmp
endif

ifneq (,$(findstring sunCC,$(CXX)))
	CXX_FLAGS = -std=c++11 -xO3 -m64 -xtarget=native
	PAR_FLAG = -xopenmp
endif

ifneq ($(SERIAL), 1)
	CXX_FLAGS += $(PAR_FLAG)
endif

KERNELS = bfs pr cc_sv bc
SUITE = $(KERNELS)

.PHONY: all
all: $(SUITE)

% : src/%.cc src/*.h
	$(CXX) $< -o $@ $(CXX_FLAGS) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(SUITE) test/out/*
