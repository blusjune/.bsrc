/*
 * -------------------------------------------------------------------------------------------------
 * iowm.c
 * --
 * I/O Workload Management
 * --
 * Brian M. JUNG <brian.m.jung@{gmail.com, samsung.com}>
 * 20121005_192749 (c)
 * -------------------------------------------------------------------------------------------------
 */ 




#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include "iowm.h"




/*
 * I/O Workload Structure (iow_st) Handlers
 * --
 * iow_init();
 * iow_destroy();
 * --
 */
int iow_init(iow_st * iow, int num)
{
	int i=0;

	if (iow == (iow_st *)0 || num <= 0) {
		return -1;
	}

	iow->num_iop = num;
	iow->iop = (iop_st *)IOWM_malloc(sizeof(iop_st) * iow->num_iop);
	for (i=0; i<num; i++) {
		iop_clear((iop_st *)&(iow->iop[i]));
	}

	return 0;
}

int iow_destroy(iow_st * iow)
{
	int i=0;

	if (iow == (iow_st *)0) {
		return -1;
	}

	for (i=0; i<iow->num_iop; i++) {
		iop_clear((iop_st *)&(iow->iop[i]));
	}
	iow->num_iop = 0;
	IOWM_free((void *)(iow->iop));

	return 0;
}

int iow_set_by_struct(iow_st * iow, int num_iop, iop_st * iop_data)
{
	int i = 0;
	int total_sum = 0;

	for (i=0; i<num_iop; i++) {
		iop_set_by_struct(&(iow->iop[i]), &(iop_data[i]));
		total_sum += iop_data[i].share;
	}
	if (total_sum != IOWM_TOTAL_SUM_OF_IOPTRN_SHARE) {
		return -1;
	}
}

int iow_print_iop(iow_st * iow)
{
	int i = 0;
	int j = 0;
	int total_sum_share = 0;
	int total_sum_v_dist = 0;

	if (iow == (iow_st *)0) {
		return -1;
	}

	for (i=0, total_sum_share=0; i<iow->num_iop; i++) {
		printf("\n-----");
		printf("\niop[%d]:\n", i);
		printf("\t.share: %d\n", iow->iop[i].share); total_sum_share += iow->iop[i].share;
		printf("\t.a_ptrn: %s (0x%02x)\n", a_ptrn_str(iow->iop[i].a_ptrn), iow->iop[i].a_ptrn);
		printf("\t.as_range.offset: %ld\n", iow->iop[i].as_range.offset);
		printf("\t.as_range.length: %ld\n", iow->iop[i].as_range.length);
		printf("\t.v_dist[~]:");
		for (j=0, total_sum_v_dist=0; j<IOWM_V_DIST_MAX; j++) {
			if ((j%16) == 0) printf("\n\t\t");
			printf("%d ", iow->iop[i].v_dist[j]);
			total_sum_v_dist += iow->iop[i].v_dist[j];
		}
		if (total_sum_v_dist != IOWM_TOTAL_SUM_OF_IOPTRN_V_DIST) {
			printf("ERROR: sum of 'v_dist' is %d, should be %d\n", total_sum_v_dist, IOWM_TOTAL_SUM_OF_IOPTRN_V_DIST);
			return -1;
		}
	}
	printf("\n");
	if (total_sum_share != IOWM_TOTAL_SUM_OF_IOPTRN_SHARE) {
		printf("ERROR: sum of 'share' is %d, should be %d\n", total_sum_share, IOWM_TOTAL_SUM_OF_IOPTRN_SHARE);
		return -1;
	}

	return 0;
}




/*
 * I/O Pattern Structure (iop_st) Handlers
 * --
 * iop_clear();
 * iop_set_by_value();
 * iop_set_by_struct();
 * --
 */

int iop_clear(iop_st * iop)
{
	int i = 0;

	if (iop == (iop_st *)0) {
		return -1;
	}

	iop->share = 0;
	iop->a_ptrn = IOWM_A_PTRN_NULL; /* 0 */
	iop->as_range.offset = 0;
	iop->as_range.length = 0;
	for (i=0; i<IOWM_V_DIST_MAX; i++) {
		iop->v_dist[i] = 0;
	}

	return 0;
}

int iop_set_by_value(iop_st * iop,
		int share, char a_ptrn,
		unsigned long as_range_offset,
		unsigned long as_range_length,
		int *v_dist)
{
	int i = 0;
	int total_sum = 0;

	if (
			iop == (iop_st *)0 ||
			share < 0 ||
			a_ptrn_chk_exception(a_ptrn) < 0 ||
			as_range_offset < 0 ||
			as_range_length < 0 ||
			v_dist == (int *)0
			) {
		return -1;
	}

	iop->share = share;
	iop->a_ptrn = a_ptrn;
	iop->as_range.offset = as_range_offset;
	iop->as_range.length = as_range_length;
	for (i=0; i<IOWM_V_DIST_MAX; i++) {
		iop->v_dist[i] = v_dist[i];
		total_sum += iop->v_dist[i];
	}
	if (total_sum != IOWM_TOTAL_SUM_OF_IOPTRN_V_DIST) {
		return -1;
	}

	return 0;
}

int iop_set_by_struct(iop_st * iop, iop_st * data)
{
	int i = 0;
	int total_sum = 0;

	if (iop == (iop_st *)0 || data == (iop_st *)0) {
		return -1;
	}

	iop->share = data->share;
	iop->a_ptrn = data->a_ptrn;
	iop->as_range.offset = data->as_range.offset;
	iop->as_range.length = data->as_range.length;
	for (i=0; i<IOWM_V_DIST_MAX; i++) {
		iop->v_dist[i] = data->v_dist[i];
		total_sum += iop->v_dist[i];
	}
	if (total_sum != IOWM_TOTAL_SUM_OF_IOPTRN_V_DIST) {
		return -1;
	}

	return 0;
}

ioop_conf ioop_cf;
int ioop_bootstrap(ioop_conf * cf)
{
	return 0;
}

int ioop_read_seqn(void * vp)
{
	iop_st *iop = (iop_st *)vp;
	int as_ran_off;
	int as_ran_len;

	if (iop == (iop_st *)0) {
		return -1;
	}

	as_ran_off = iop->as_range.offset;
	as_ran_len = iop->as_range.length;



	return 0;
}

unsigned long util_rand_getrandnum(void)
{
	static int flag_randseed;

	if (flag_randseed == 0) {
		util_rand_setrandseed();
		flag_randseed = 1;
	}
	return random();
}

struct timeb ct; /* current time */
void util_rand_setrandseed(void)
{
	unsigned long	seed;
	ftime(&ct);
	seed = (unsigned long)(ct.time + ct.millitm);
	srandom(seed);
}




/********************************************************************/
/********************************************************************/


#define IOWM_L_BLOCK_SIZE		512	/* logical block size: 512 bytes */
#define IOWM_P_BLOCK_SIZE		4096	/* physical block size: 4096 bytes */

int iowm_block_data_fill(char * blk_buf, int blk_size)
{
	for (i=0; i<blk_size; i++) {
		blk_buf[i] = get_byte_value();
	}

}
































