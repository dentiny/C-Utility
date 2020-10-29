#ifndef __CPS310_MM_H__
#define __CPS310_MM_H__

/*
 * Warning.  This file contains useful stuff and some experiment settings.
 * You can change the settings for your own experiments, but the grader
 * replaces your dmm.h with its own at grading time.  Do not add defs to
 * this file: anything you add is not present at grading time.
 */

/*
 * Experiment with different size heaps.  You can change it here or in the Makefile.
 * (But not both.)
 */
//#define MAX_HEAP_SIZE	(1024) /* max size restricted to 1kB*/
//#define MAX_HEAP_SIZE	(1024*1024*32) /* max size restricted to 32 MB */
//define MAX_HEAP_SIZE	(1024*1024*4) /* max size restricted to 4MB */

/* 
 * WORD_SIZE is the preferred alignment, the number of bytes in a pointer type.
 * On 32-bit machines you may change this to 4 to improve efficiency.
 */
#define WORD_SIZE	8
#define ALIGNMENT 	WORD_SIZE
#define ENTRY_SIZE 100 // number of linked list header

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#define METADATA_T_ALIGNED (ALIGN(sizeof(metadata_t)))
#define FOOTER_T_ALIGNED (ALIGN(sizeof(footer_t)))

typedef enum{
  false,
  true
} bool;

bool dmalloc_init();
void *dmalloc(size_t numbytes);
void dfree(void *allocptr);


#endif /* end of __CPS310_MM_H__ */
