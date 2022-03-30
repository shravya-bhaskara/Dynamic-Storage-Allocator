/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "d1g174l_f0rtr355",
    /* First member's full name */
    "Shravya B",
    /* First member's email address */
    "digitalfortress@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

# define WSIZE 4
# define DSIZE 8
# define CHUNKSIZE 4096

# define MAX(a,b) (((a) > (b)) ? (a) : (b))
# define MIN(a,b) (((a) > (b)) ? (b) : (a))

# define GET(pt) 	(*(unsigned int *)(pt))
# define PUT(pt, value)	(*(unsigned int *)(pt) = (value))

# define GET_SIZE(pt)	(GET(pt) & ~0x7)
# define GET_ALLOC(pt)	(GET(pt) & 0x1)

# define HEADER(pt)	((char *)(pt) - WSIZE)
# define FOOTER(pt)	((char *)(pt) + GET_SIZE(HEADER(pt)) - DSIZE)

# define NEXT_BLK(pt)	((char *)(pt) + GET_SIZE((char *)(pt)) - WSIZE)
# define PREV_BLK(pt)   ((char *)(pt) - GET_SIZE((char *)(pt)) - DSIZE)

static char *heap_listp = 0;
/*

User defined functions needed:

1) extend_heap : to extend or add heap memory
2) coalesce: to manage and merge free blocks of memory
3) place: to place the requested block at the beginning of the free block
4) find_fit: implement a first fit algorithm to find a freed block of memory of suitable size for requested block size.

*/


static void *coalesce(void * pt){
    size_t previous_blk;
    previous_blk = GET_ALLOC(FOOTER(PREV_BLK(pt))) || PREV_BLK(pt) == pt;
    size_t next_blk = GET_ALLOC(HEADER(NEXT_BLK(pt)));

    size_t blk_size = GET_SIZE(HEADER(pt));

    if(previous_blk && next_blk){
        return pt;

    }else if(!previous_blk && next_blk){

        blk_size += GET_SIZE(HEADER(PREV_BLK(pt)));

        PUT(HEADER(PREV_BLK(pt)), (blk_size | 0x0));
        PUT(FOOTER(pt), (blk_size | 0x0));

        pt = PREV_BLK(pt);
        
    }else if(previous_blk && !next_blk){

        blk_size += GET_SIZE(HEADER(NEXT_BLK(pt)));

        PUT(HEADER(pt), (blk_size | 0x0));
        PUT(FOOTER(NEXT_BLK(pt)), (blk_size | 0x0));

    }else if(!previous_blk && !next_blk){

        blk_size += GET_SIZE(HEADER(PREV_BLK(pt))) + GET_SIZE(HEADER(NEXT_BLK(pt)));

        PUT(HEADER(PREV_BLK(pt)), (blk_size | 0x0));
        PUT(FOOTER(NEXT_BLK(pt)), (blk_size | 0x0));

        pt = PREV_BLK(pt);

    }
    return pt;


}

static void *place(char * pt, size_t size){

    size_t available_block_size = GET_SIZE(HEADER(pt));
    
    if(GET_ALLOC(HEADER(pt)) == 0){
        if((available_block_size - size) <= 2*DSIZE){
            PUT(HEADER(pt), (available_block_size | 0x1));
            PUT(FOOTER(pt), (available_block_size | 0x1));

        }else{
            PUT(HEADER(pt), (size | 0x1));
            PUT(FOOTER(pt), (size | 0x1));

            pt = NEXT_BLK(HEADER(pt));
            PUT(HEADER(pt), ((available_block_size - size) | 0x0));
            PUT(FOOTER(pt), ((available_block_size - size) | 0x0));

            coalesce(pt);

        }
    }
    return pt;
}

static void *find_fit(size_t asize, char *pt){

    // implementing a first fit search to find the free block available for a given size of memory.
    // heap_listp = 0
    // 
    

    /*if((GET_SIZE(HEADER(pt)) > asize) && (GET_ALLOC(HEADER(pt)) == 0)){
        return pt;
    }else{
        pt = NEXT_BLK(pt);
        find_fit(asize, pt);
    }
    */
    for(pt = heap_listp; GET_SIZE(HEADER(pt)) > 0; pt = NEXT_BLK(pt)){
        if(!GET_ALLOC(HEADER(pt)) && (asize <= GET_SIZE(HEADER(pt)))){
            return pt;
        }
    }
    
    return NULL;
    // return NULL; // No fit found

}

static void *extend_heap(size_t words){
    char *pt;
    size_t size;

    size = words % 2;
    if(size){
        size = (words + 1)*WSIZE;
    }else{
        size = words * WSIZE;
    }

    //pt = mem_sbrk(size);
    if((long)(pt = mem_sbrk(size)) == -1){
        return NULL;
    }

    // HEADER and FOOTER for extended free block of memory
    PUT(HEADER(pt), (size | 0x0));
    PUT(FOOTER(pt), (size | 0x0));

    // epilogue for extended free block of memory
    PUT(HEADER(NEXT_BLK(pt)), (0x0 | 0x1));

    // if previous block of memory is free, then merge with the extended block and return ptr.
    return coalesce(pt);
}

static void mm_checkheap(int line){


    printf("Error at line number: %d\n", line);
    /*void * pt = heap_listp;
    if(verbose){
        printf("Heap (%p):\n", heap_listp);
    }

    if((GET_SIZE(HEADER(heap_listp))!= DSIZE)){// || (!GET_ALLOC(HEADER(heap_listp)))){
        // if Size of header is not 8 bytes, or if heap ptr is not allocate


    }*/

}


/* 
 * mm_init - initialize the malloc package.
 */

 // creating an initial heap free block
int mm_init(void)
{
    heap_listp = mem_sbrk(4*WSIZE);
    if(heap_listp == (void*)(-1)){
        return -1;
    }

    /* 
    PADDING PROLOGUE EPILOGUE
         _______________
        |   |8/1|8/1|0/1|
        |___|___|___|___|

        WSIZE ------ WSIZE
    */

    // heap list header initialization
    PUT(heap_listp, 0x0);

    // prologue block - header and footer
    
    PUT(heap_listp + WSIZE, (DSIZE | 0x1));
    PUT(heap_listp + 2*WSIZE, (DSIZE | 0X1));

    // epilogue block
    PUT(heap_listp + 3*WSIZE, (0x0 | 0X1));
    
    heap_listp += 2*WSIZE;

    if(extend_heap(CHUNKSIZE/WSIZE)==NULL){
        return -1;
    }
    return 0;
    
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    /*
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
    */
    size_t extendsize;
    char * pt;

    if (size == 0){
        return NULL;
    }
    if(size <= DSIZE){
        size = 2*DSIZE;
    }else{
        size = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    pt = find_fit(size, heap_listp);
    if(pt == NULL){
        extendsize = MAX(size, CHUNKSIZE);
        pt = extend_heap(extendsize/WSIZE);
        if(pt == NULL){
            return NULL;
        }else{
            place(pt, size);
            return pt;
        }
    }

    place(pt, size);
    //mm_checkheap(__LINE__);
    return pt;

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HEADER(ptr));

    PUT(HEADER(ptr), (size | 0x0));
    PUT(FOOTER(ptr), (size | 0x0));

    coalesce(ptr);
    mm_checkheap(__LINE__);


}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t current_size = GET_SIZE(HEADER(ptr));
    size_t free_blk_size;
    size_t req_size = ALIGN(size);
    size_t copySize;
    // int inc_size;

    if(oldptr == NULL){
        return mm_malloc(size); // if pointer is NULL then it will be same as mm_malloc()
    }
    if(size == 0){
        mm_free(ptr); // since size is 0 thus freeing memory.
    }

    if(GET_ALLOC(HEADER(PREV_BLK(ptr))) == 0){
        current_size = current_size + GET_SIZE(HEADER(PREV_BLK(ptr)));
        ptr = PREV_BLK(ptr);
    }
    if(GET_ALLOC(HEADER(NEXT_BLK(ptr))) == 0){
        current_size += GET_SIZE(HEADER(NEXT_BLK(ptr)));
    }
    // checking if we need to increase the size of the block for reallocating memory.
    if(current_size < size + DSIZE){
        // inc_size = 1;
        
        PUT(HEADER(ptr), (current_size | 0x0));
        PUT(FOOTER(ptr), (current_size | 0x0));

        coalesce(ptr);

        mm_malloc(size);

    }else{
       // inc_size = 0;

        // Case 1: curr_size - (req_size + DSIZE) > 2*DSIZE
        if((current_size - (req_size + DSIZE)) > 2*DSIZE){
            free_blk_size = ALIGN(current_size - (req_size + DSIZE));

            PUT(HEADER(ptr), (req_size | 0x1));
            PUT(FOOTER(ptr), (req_size | 0x1));

            ptr = NEXT_BLK(HEADER(ptr));

            PUT(HEADER(ptr), (free_blk_size | 0x0));
            PUT(FOOTER(ptr), (free_blk_size | 0x0));

            coalesce(ptr);
            
        }else{
            // Case 2: curr_size - (req_size + DSIZE) < 2*DSIZE
            PUT(HEADER(ptr), (current_size | 0x1));
            PUT(FOOTER(ptr), (current_size | 0x1));

        }

    }
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = GET_SIZE(HEADER(oldptr)); // calculates size to copy
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    mm_checkheap(__LINE__);
    return newptr;
}














