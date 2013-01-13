/*
 * -------------------------------------------------------------------------------------------------
 * main.c
 * --
 * I/O Workload Management
 * --
 * Brian M. JUNG <brian.m.jung@{gmail.com, samsung.com}>
 * 20121005_192749 (c)
 * -------------------------------------------------------------------------------------------------
 */ 

#include "iowm.h"
#include "iowm_workload.h"

iow_st my_workload;
int main(void)
{
	iow_init(&my_workload, WORKLOAD_IOP_NUM);

	iow_set_by_struct(&my_workload, WORKLOAD_IOP_NUM, iop_conf_data);
	/* iow_set_by_struct() will be renamed to: 'iow_load_iopconf' */
	/* WORKLOAD_IOP_NUM will be replaced to: sizeof(iop_conf_data)/sizeof(iop_st) */

	iow_print_iop(&my_workload);

	iow_destroy(&my_workload);

	return 0;
}




int main2(void)
{
	iow_init(&my_workload);




	/*
	 * iow_st <- iow_conf_data
	 * iow_st <- iop_conf_data[]
	 *
	 */
	iow_conf(&my_workload, IOW_TYPE__SIMUL);


	iow_set_iopsch(&my_workload, &iopsch_rr);
	iow_gen_workload(&my_workload);
	return 0;
}
