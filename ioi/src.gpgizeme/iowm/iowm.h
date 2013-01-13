#ifndef _IOWM_H_
#define _IOWM_H_




/*
 * -------------------------------------------------------------------------------------------------
 * iowm.h
 * --
 * I/O Workload Management
 * --
 * Brian M. JUNG <brian.m.jung@{gmail.com, samsung.com}>
 * 20121005_192749 (c)
 * -------------------------------------------------------------------------------------------------
 */ 




/*
 * I/O pattern (element)
 */
#define IOWM_TOTAL_SUM_OF_IOPTRN_SHARE 1000
/* sum of iop[0].share ~ iop[num_io_ptrn - 1].share should be 1000 */

#define IOWM_TOTAL_SUM_OF_IOPTRN_V_DIST 1000
/* sum of v_dist[0] ~ v_dist[IOWM_V_DIST_MAX - 1] should be 1000 */

#define IOWM_V_DIST_MAX 256
/* to hold all the possible values for 1-byte data */


/*
 * I/O workload pattern information
 */
typedef struct iop {

	void *	iow; /* link to the I/O workload (iow_st) */

	int	share; /* share(ratio) of this iop among whole iop's: value in permill (integer) */

	struct { /* access pattern */
		char	type; /* access pattern: (bit flag)
				 * (0x01)	. . . . | . . . 1	= sequential
				 * (0x02)	. . . . | . . 1 .	= random
				 * (0x10)	. . . 1 | . . . .	= read
				 * (0x20)	. . 1 . | . . . .	= write
				 */
		char *	type_str; /* access pattern (string) */
		int	(* ioop_f)(void *);	/* I/O OPeration function
						   * ioop_f will generate I/Os (based on .a_ptrn.type):
						   * 	I/O operation type (read/write),
						   * 	data address, (logical block address)
						   * 	data length,
						   * 	data value,
						   */
	} a_ptrn; /* Access PaTteRN */

	struct { /* address space range this iop reaches */
		long	offset; /* start offset */
		long	length; /* range length */
	} as_range; /* Address Space RANGE */
	int	v_dist[IOWM_V_DIST_MAX]; /* byte value distribution: value in permill (integer) */

} iop_st; /* I/O pattern structure */

#define a_ptrn_chk_exception(ptrn) ( ( (ptrn & 0x33) != 0 ) ? 0 : -1 )
#define a_ptrn_chk_seqn(ptrn) ( ((ptrn & 0x01) == 0x01) ? "seqn" : "----" )
#define a_ptrn_chk_rand(ptrn) ( ((ptrn & 0x02) == 0x02) ? "rand" : "----" )
#define a_ptrn_chk_read(ptrn) ( ((ptrn & 0x10) == 0x10) ? "read" : "----" )
#define a_ptrn_chk_writ(ptrn) ( ((ptrn & 0x20) == 0x20) ? "writ" : "----" )
#define a_ptrn_str(ptrn) ( \
	 ((ptrn & 0x10) == 0x10) ? ( ((ptrn & 0x01) == 0x01) ? "seqn-read" : (((ptrn & 0x02) == 0x02) ? "rand-read" : "?-read") ) : \
	 ( ((ptrn & 0x20) == 0x20) ? ( ((ptrn & 0x01) == 0x01) ? "seqn-writ" : (((ptrn & 0x02) == 0x02) ? "rand-writ" : "?-writ") ) : "?-?" ) \
		)

#define IOWM_A_PTRN__SEQN "seqn"
#define IOWM_A_PTRN__RAND "rand"
#define IOWM_A_PTRN__READ "read"
#define IOWM_A_PTRN__WRIT "writ"

#define IOWM_A_PTRN__READ_SEQN "read-seqn"
#define IOWM_A_PTRN__READ_RAND "read-rand"
#define IOWM_A_PTRN__WRIT_SEQN "writ-seqn"
#define IOWM_A_PTRN__WRIT_RAND "writ-rand"




/*
 * I/O workload
 * comprised of num_iop I/O patterns
 */
typedef struct iow {

	/* IOP list */
	int		num_iop;
	iop_st *	iop;

	char		iow_type; /* IOW_TYPE__SIMUL | IOW_TYPE__REAL */

	/*
	 * IOOP configuration
	 */
	struct {
#if 0
		char *	tgt_dev;
#endif
	} ioop_conf;

	/*
	 * IOP scheduler configuration
	 */
	struct {
		long	nioop_total; /* number of total I/O operations for this workload generation */
		int	nioop_sch_term; /* IOP scheduling period in number of I/O operations */
		int (* iop_sch_f)(void *); /* IOP_SCH function (currently selected) */
	} iop_sch_conf;

	/*
	 * Logical target device
	 */
	struct {
#if 0
		tgt_dev;
		???

#endif
	} tgt_dev; /* TarGeT DEVice */

	struct {

		/*
		 * I/O operation functions
		 */
		struct {
			int (* f_read_seqn)(void *);	/* 0x11 */
			int (* f_read_rand)(void *);	/* 0x12 */
			int (* f_writ_seqn)(void *);	/* 0x21 */
			int (* f_writ_rand)(void *);	/* 0x22 */
		} ioop; /* IOOP: I/O OPeration */

		/*
		 * Function to schedule execution of each member I/O pattern
		 */
		struct {
			int (* f_manu)(void *);	/* scheduling based on the manually specified sequence */
			int (* f_rr)(void *);	/* scheduling in a round-robin fashion */
			int (* f_rnd)(void *);	/* scheduling in a random fashion */
		} iop_sch; /* IOP_SCH: I/O Pattern SCHeduler */

	} loadables;

} iow_st;




/*
 * function signatures
 */
int iow_init(iow_st * iow, int num);
int iow_destroy(iow_st * iow);
int iop_clear(iop_st * iop);
int iop_set_by_value(iop_st * iop,
		int share, char a_ptrn,
		unsigned long as_range_offset,
		unsigned long as_range_length,
		int *v_dist);
int iop_set_by_struct(iop_st * iop, iop_st * data);




/*
 * Wrapper function pointers
 * for ease of later code maintenance
 */
#define IOWM_malloc malloc
#define IOWM_free free

#if 0 /* this does not work! */
void * (* IOWM_malloc)(size_t *);
void (* IOWM_free)(void *);
IOWM_malloc = malloc;
IOWM_free = free;
#endif




#define IOWM_A_PTRN_NULL	0




#endif /* _IOWM_H_ */
