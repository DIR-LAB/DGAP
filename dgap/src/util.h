// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <cinttypes>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

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
//#define DB_POOL_SIZE ((((size_t)1) << 39) + (((size_t)1) << 37))
//#define DB_POOL_SIZE (((size_t)1) << 31)

/* name of layout in the pool */
#define LAYOUT_NAME "apma_layout"

/* type of allocations */
enum type{
  VERTEX_TYPE,
  EDGE_TYPE,
  ULOG_PTR_TYPE,
  OPLOG_PTR_TYPE,
  SEG_LOG_PTR_TYPE,
  SEG_LOG_IDX_TYPE,
  SEG_LOG_TYPE,
  PMA_TREE_META_TYPE
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

/*****************************************************************************
 *                                                                           *
 *   APMA Utility functions                                                  *
 *                                                                           *
 *****************************************************************************/

// Returns the 1-based index of the last (most significant) bit set in x.
inline uint64_t last_bit_set(uint64_t x)
{
  return (sizeof(uint64_t) * 8 - __builtin_clzll(x)); // Linux
}

inline uint64_t floor_log2(uint64_t x)
{
  return (last_bit_set(x) - 1);
}

inline uint64_t ceil_log2(uint64_t x)
{
  assert(x > 0 && "x should not be 0 or less");
  return (last_bit_set(x - 1));
  // i.e. ceil_log2(13) = 4, ceil_log2(27) = 5, etc.
}

inline uint64_t floor_div(uint64_t x, uint64_t y)
{
  return (x / y);
}

inline uint64_t ceil_div(uint64_t x, uint64_t y)
{
  if(x == 0) return 0;
  return (1 + ((x - 1) / y));
}

// Returns the largest power of 2 not greater than x ($2^{\lfloor \lg x \rfloor}$).
inline uint64_t hyperfloor(uint64_t x)
{
  return (1ULL << floor_log2(x));
}

// Returns the smallest power of 2 not less than x ($2^{\lceil \lg x \rceil}$).
inline uint64_t hyperceil(uint64_t x)
{
  return (1ULL << ceil_log2(x));
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
