// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2016-2020, Intel Corporation */

/*
 * out_err_mt_win.c -- unit test for error messages
 */

#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>
#include "unittest.h"
#include "valgrind_internal.h"
#include "util.h"

#define NUM_THREADS 16

static void
print_errors(const wchar_t *msg)
{
	UT_OUT("%S", msg);
	UT_OUT("PMEM: %S", pmem_errormsgW());
	UT_OUT("PMEMOBJ: %S", pmemobj_errormsgW());
	UT_OUT("PMEMLOG: %S", pmemlog_errormsgW());
	UT_OUT("PMEMBLK: %S", pmemblk_errormsgW());
	UT_OUT("PMEMPOOL: %S", pmempool_errormsgW());
}

static void
check_errors(int ver)
{
	int ret;
	int err_need;
	int err_found;

	ret = swscanf(pmem_errormsgW(),
		L"libpmem major version mismatch (need %d, found %d)",
		&err_need, &err_found);
	UT_ASSERTeq(ret, 2);
	UT_ASSERTeq(err_need, ver);
	UT_ASSERTeq(err_found, PMEM_MAJOR_VERSION);

	ret = swscanf(pmemobj_errormsgW(),
		L"libpmemobj major version mismatch (need %d, found %d)",
		&err_need, &err_found);
	UT_ASSERTeq(ret, 2);
	UT_ASSERTeq(err_need, ver);
	UT_ASSERTeq(err_found, PMEMOBJ_MAJOR_VERSION);

	ret = swscanf(pmemlog_errormsgW(),
		L"libpmemlog major version mismatch (need %d, found %d)",
		&err_need, &err_found);
	UT_ASSERTeq(ret, 2);
	UT_ASSERTeq(err_need, ver);
	UT_ASSERTeq(err_found, PMEMLOG_MAJOR_VERSION);

	ret = swscanf(pmemblk_errormsgW(),
		L"libpmemblk major version mismatch (need %d, found %d)",
		&err_need, &err_found);
	UT_ASSERTeq(ret, 2);
	UT_ASSERTeq(err_need, ver);
	UT_ASSERTeq(err_found, PMEMBLK_MAJOR_VERSION);

	ret = swscanf(pmempool_errormsgW(),
		L"libpmempool major version mismatch (need %d, found %d)",
		&err_need, &err_found);
	UT_ASSERTeq(ret, 2);
	UT_ASSERTeq(err_need, ver);
	UT_ASSERTeq(err_found, PMEMPOOL_MAJOR_VERSION);
}

static void *
do_test(void *arg)
{
	int ver = *(int *)arg;

	pmem_check_version(ver, 0);
	pmemobj_check_version(ver, 0);
	pmemlog_check_version(ver, 0);
	pmemblk_check_version(ver, 0);
	pmempool_check_version(ver, 0);
	check_errors(ver);

	return NULL;
}

static void
run_mt_test(void *(*worker)(void *))
{
	os_thread_t thread[NUM_THREADS];
	int ver[NUM_THREADS];

	for (int i = 0; i < NUM_THREADS; ++i) {
		ver[i] = 10000 + i;
		THREAD_CREATE(&thread[i], NULL, worker, &ver[i]);
	}
	for (int i = 0; i < NUM_THREADS; ++i) {
		THREAD_JOIN(&thread[i], NULL);
	}
}

int
wmain(int argc, wchar_t *argv[])
{
	STARTW(argc, argv, "out_err_mt_win");

	if (argc != 6)
		UT_FATAL("usage: %S file1 file2 file3 file4 dir",
				argv[0]);

	print_errors(L"start");

	PMEMobjpool *pop = pmemobj_createW(argv[1], L"test",
		PMEMOBJ_MIN_POOL, 0666);
	PMEMlogpool *plp = pmemlog_createW(argv[2],
		PMEMLOG_MIN_POOL, 0666);
	PMEMblkpool *pbp = pmemblk_createW(argv[3],
		128, PMEMBLK_MIN_POOL, 0666);

	util_init();

	pmem_check_version(10000, 0);
	pmemobj_check_version(10001, 0);
	pmemlog_check_version(10002, 0);
	pmemblk_check_version(10003, 0);
	pmempool_check_version(10006, 0);
	print_errors(L"version check");

	void *ptr = NULL;
	/*
	 * We are testing library error reporting and we don't want this test
	 * to fail under memcheck.
	 */
	VALGRIND_DO_DISABLE_ERROR_REPORTING;
	pmem_msync(ptr, 1);
	VALGRIND_DO_ENABLE_ERROR_REPORTING;
	print_errors(L"pmem_msync");

	int ret;
	PMEMoid oid;
	ret = pmemobj_alloc(pop, &oid, 0, 0, NULL, NULL);
	UT_ASSERTeq(ret, -1);
	print_errors(L"pmemobj_alloc");

	pmemlog_append(plp, NULL, PMEMLOG_MIN_POOL);
	print_errors(L"pmemlog_append");

	size_t nblock = pmemblk_nblock(pbp);
	pmemblk_set_error(pbp, nblock + 1);
	print_errors(L"pmemblk_set_error");

	run_mt_test(do_test);

	pmemobj_close(pop);
	pmemlog_close(plp);
	pmemblk_close(pbp);

	PMEMpoolcheck *ppc;
	struct pmempool_check_args args = {0, };
	ppc = pmempool_check_init(&args, sizeof(args) / 2);
	UT_ASSERTeq(ppc, NULL);
	print_errors(L"pmempool_check_init");

	DONEW(NULL);
}
