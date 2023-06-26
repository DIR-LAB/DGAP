// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <cinttypes>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "timer.h"


/*
Miscellaneous helpers that don't fit into classes
*/

/**
 * Utility functions to flush cache line to PMEM: Code copied from "PMDK" repository
 * */

#define FLUSH_ALIGN ((uintptr_t)64)

/*
 * The x86 memory instructions are new enough that the compiler
 * intrinsic functions are not always available.  The intrinsic
 * functions are defined here in terms of asm statements for now.
 */
static inline void
pmem_clflushopt(const void *addr)
{
  asm volatile(".byte 0x66; clflush %0" : "+m" \
		(*(volatile char *)(addr)));
}

static inline void
pmem_clwb(const void *addr)
{
  asm volatile(".byte 0x66; xsaveopt %0" : "+m" \
		(*(volatile char *)(addr)));
}

/*
 * flush_clwb_nolog -- flush the CPU cache, using clwb
 */
static inline void
flush_clwb_nolog(const void *addr, size_t len)
{
  uintptr_t uptr;

  /*
   * Loop through cache-line-size (typically 64B) aligned chunks
   * covering the given range.
   */
  for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
       uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN) {
    pmem_clwb((char *)uptr);
  }

  _mm_sfence();
}

#define CREATE_MODE_RW (S_IWUSR | S_IRUSR)

/* size of the pmem object pool -- 8 GB */
#define DB_POOL_SIZE (((size_t)1) << 39)
//#define DB_POOL_SIZE (((size_t)1) << 35)

/* name of layout in the pool */
#define LAYOUT_NAME "apma_layout"

/* type of allocations */
enum type {
  VERTEX_TYPE,
  EDGE_TYPE
};

static inline int file_exists(char const *file) {
  return access(file, F_OK);  /* 0 means the file exists */
}

static inline void check_sanity(struct Base *bp) {
  if (bp == NULL){
    fprintf(stderr, "[%s]: FATAL: The Root Object Not Initalized Yet, Exit!\n", __FUNCTION__);
    exit(0);
  }
}

static const int64_t kRandSeed = 27491095;


void PrintLabel(const std::string &label, const std::string &val) {
  printf("%-21s%7s\n", (label + ":").c_str(), val.c_str());
}

void PrintTime(const std::string &s, double seconds) {
  printf("%-21s%3.5lf\n", (s + ":").c_str(), seconds);
}

void PrintStep(const std::string &s, int64_t count) {
  printf("%-14s%14" PRId64 "\n", (s + ":").c_str(), count);
}

void PrintStep(int step, double seconds, int64_t count = -1) {
  if (count != -1)
    printf("%5d%11" PRId64 "  %10.5lf\n", step, count, seconds);
  else
    printf("%5d%23.5lf\n", step, seconds);
}

void PrintStep(const std::string &s, double seconds, int64_t count = -1) {
  if (count != -1)
    printf("%5s%11" PRId64 "  %10.5lf\n", s.c_str(), count, seconds);
  else
    printf("%5s%23.5lf\n", s.c_str(), seconds);
}

// Runs op and prints the time it took to execute labelled by label
#define TIME_PRINT(label, op) {   \
  Timer t_;                       \
  t_.Start();                     \
  (op);                           \
  t_.Stop();                      \
  PrintTime(label, t_.Seconds()); \
}


template <typename T_>
class RangeIter {
  T_ x_;
 public:
  explicit RangeIter(T_ x) : x_(x) {}
  bool operator!=(RangeIter const& other) const { return x_ != other.x_; }
  T_ const& operator*() const { return x_; }
  RangeIter& operator++() {
    ++x_;
    return *this;
  }
};

template <typename T_>
class Range{
  T_ from_;
  T_ to_;
 public:
  explicit Range(T_ to) : from_(0), to_(to) {}
  Range(T_ from, T_ to) : from_(from), to_(to) {}
  RangeIter<T_> begin() const { return RangeIter<T_>(from_); }
  RangeIter<T_> end() const { return RangeIter<T_>(to_); }
};

#endif  // UTIL_H_
