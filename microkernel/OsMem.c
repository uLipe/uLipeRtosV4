/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsMem.c
 *
 *  \brief ulipe memory management impl file
 *
 *  Author: FSN
 *
 */


/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 2.4.4
 *
 * Written by Miguel Masmano Tello <mimastel@doctor.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) 2008, 2007, 2006, 2005, 2004
 *
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 */

 /*
 * Code contributions:
 *
 * (Jul 28 2007)  Herman ten Brugge <hermantenbrugge@home.nl>:
 *
 * - Add 64 bit support. It now runs on x86_64 and solaris64.
 * - I also tested this on vxworks/32and solaris/32 and i386/32 processors.
 * - Remove assembly code. I could not measure any performance difference
 *   on my core2 processor. This also makes the code more portable.
 * - Moved defines/typedefs from tlsf.h to tlsf.c
 * - Changed MIN_BLOCK_SIZE to sizeof (free_ptr_t) and BHDR_OVERHEAD to
 *   (sizeof (bhdr_t) - MIN_BLOCK_SIZE). This does not change the fact
 *    that the minumum size is still sizeof
 *   (bhdr_t).
 * - Changed all C++ comment style to C style. (// -> /.* ... *./)
 * - Used ls_bit instead of ffs and ms_bit instead of fls. I did this to
 *   avoid confusion with the standard ffs function which returns
 *   different values.
 * - Created set_bit/clear_bit fuctions because they are not present
 *   on x86_64.
 * - Added locking support + extra file target.h to show how to use it.
 * - Added get_used_size function (REMOVED in 2.4)
 * - Added rtl_realloc and rtl_calloc function
 * - Implemented realloc clever support.
 * - Added some test code in the example directory.
 *
 *
 * (Oct 23 2006) Adam Scislowicz:
 *
 * - Support for ARMv5 implemented
 *
 */

/* Code contributions:
 * (2015) Felipe Neves:
 * - added efficient ffs /fls based on armv7-m specifics
 * - optimized mapping search and insert on code size
 *
 */

#include "uLipeRtos4.h"



#define TLSF_ADD_SIZE(tlsf, b) do {                                 \
        tlsf->used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;  \
        if (tlsf->used_size > tlsf->max_size)                       \
            tlsf->max_size = tlsf->used_size;                       \
        } while(0)

#define TLSF_REMOVE_SIZE(tlsf, b) do {                              \
        tlsf->used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;  \
    } while(0)


#define BLOCK_ALIGN                         (sizeof(void *) * 2)


#if (OS_HEAP_SIZE <= 32768)
	#define MAX_FLI                             (15)
	#define MAX_LOG2_SLI                        (4)
	#define MAX_SLI                             (1 << MAX_LOG2_SLI)
#elif (OS_HEAP_SIZE > 32768) &&(OS_HEAP_SIZE < 65536)
	#define MAX_FLI                             (16)
	#define MAX_LOG2_SLI                        (4)
	#define MAX_SLI                             (1 << MAX_LOG2_SLI)
#else
	#define MAX_FLI                             (24)
	#define MAX_LOG2_SLI                        (5)
	#define MAX_SLI                             (1 << MAX_LOG2_SLI)
#endif


#define FLI_OFFSET                          (6)
#define SMALL_BLOCK                         (128)
#define REAL_FLI                            (MAX_FLI - FLI_OFFSET)
#define MIN_BLOCK_SIZE                      (sizeof (free_ptr_t))
#define BHDR_OVERHEAD                       (sizeof (bhdr_t) - MIN_BLOCK_SIZE)
#define TLSF_SIGNATURE                      (0x2A59FA59)

#define PTR_MASK                            (sizeof(void *) - 1)
#define BLOCK_SIZE                          (0xFFFFFFFF - PTR_MASK)

#define GET_NEXT_BLOCK(_addr, _r)           ((bhdr_t *) ((uint8_t *) (_addr) + (_r)))
#define MEM_ALIGN                           ((BLOCK_ALIGN) - 1)
#define ROUNDUP_SIZE(_r)                    (((_r) + MEM_ALIGN) & ~MEM_ALIGN)
#define ROUNDDOWN_SIZE(_r)                  ((_r) & ~MEM_ALIGN)
#define ROUNDUP(_x, _v)                     ((((~(_x)) + 1) & ((_v)-1)) + (_x))


#define BLOCK_STATE                         (0x1)
#define PREV_STATE                          (0x2)
#define FREE_BLOCK                          (0x1)
#define USED_BLOCK                          (0x0)
#define PREV_FREE                           (0x2)
#define PREV_USED                           (0x0)

#define DEFAULT_AREA_SIZE (1024*10)


/* linked list heap managament structures */
typedef struct free_ptr_struct
{
    struct bhdr_struct *prev;
    struct bhdr_struct *next;
} free_ptr_t;

typedef struct bhdr_struct
{
    struct bhdr_struct *prev_hdr;
    size_t size;
    union
        {
        struct free_ptr_struct free_ptr;
        uint8_t buffer[1];
    } ptr;

}bhdr_t;

typedef struct area_info_struct
{
    bhdr_t *end;
    struct area_info_struct *next;
} area_info_t;


/* TLSF data structure to manage heap */
typedef struct TLSF_struct {
    uint32_t tlsf_signature;
    size_t used_size;
    size_t max_size;
    area_info_t *area_head;
    uint32_t fl_bitmap;
    uint32_t sl_bitmap[REAL_FLI];
    bhdr_t *matrix[REAL_FLI][MAX_SLI];
} tlsf_t;

/** private variables */
static uint8_t OsCoreMemory[OS_HEAP_SIZE + sizeof(tlsf_t)] = { 0 };
static uint8_t *mp = NULL;
OsHandler_t memSingleTonMutex;

/** private functions */
static __inline void set_bit(int32_t nr, uint32_t * addr);
static __inline void clear_bit(int32_t nr, uint32_t * addr);
static __inline int32_t ls_bit(int32_t x);
static __inline int32_t ms_bit(int32_t x);
static __inline void MAPPING_SEARCH(size_t * _r, int32_t *_fl, int32_t *_sl);
static __inline void MAPPING_INSERT(size_t _r, int32_t *_fl, int32_t *_sl);
static __inline bhdr_t *FIND_SUITABLE_BLOCK(tlsf_t * _tlsf, int32_t *_fl, int32_t *_sl);
static __inline bhdr_t *process_area(void *area, size_t size);
static size_t init_memory_pool(size_t mem_pool_size, void *mem_pool);
static void *malloc_ex(size_t size, void *mem_pool);
static void free_ex(void *ptr, void *mem_pool);


static __inline int32_t ls_bit(int32_t i) {
    return ((i == 0 )? 0 : 31 - uLipePortBitLSScan(i));
}

static __inline int32_t ms_bit(int32_t i) {
    return ((i == 0)? 0 : 31 - uLipePortBitFSScan(i));
}


static __inline void set_bit(int32_t nr, uint32_t * addr) {
    addr[nr >> 5] |= 1 << (nr & 0x1f);
}


static __inline void clear_bit(int32_t nr, uint32_t * addr) {
    addr[nr >> 5] &= ~(1 << (nr & 0x1f));
}



static __inline void MAPPING_SEARCH(size_t * _r, int32_t *_fl, int32_t *_sl) {
    int32_t _t;

    if (*_r < SMALL_BLOCK) {
        *_fl = 0;
        *_sl = *_r / (SMALL_BLOCK / MAX_SLI);
    } else {
        _t = (1 << (ms_bit(*_r) - MAX_LOG2_SLI)) - 1;
        *_r = *_r + _t;
        *_fl = ms_bit(*_r);
        *_sl = (*_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
        *_r &= ~_t;
    }
}


static __inline void MAPPING_INSERT(size_t _r, int32_t *_fl, int32_t *_sl) {

    if (_r < SMALL_BLOCK) {
        *_fl = 0;
        *_sl = _r / (SMALL_BLOCK / MAX_SLI);
    } else {
        *_fl = ms_bit(_r);
        *_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
    }
}

static __inline bhdr_t *FIND_SUITABLE_BLOCK(tlsf_t * _tlsf, int32_t *_fl, int32_t *_sl) {
    uint32_t _tmp = _tlsf->sl_bitmap[*_fl] & (~0 << *_sl);
    bhdr_t *_b = NULL;

    if (_tmp) {
        *_sl = ls_bit(_tmp);
        _b = _tlsf->matrix[*_fl][*_sl];
    } else {
        *_fl = ls_bit(_tlsf->fl_bitmap & (~0 << (*_fl + 1)));

        if (*_fl > 0) {
            *_sl = ls_bit(_tlsf->sl_bitmap[*_fl]);
            _b = _tlsf->matrix[*_fl][*_sl];
        }
    }
    return _b;
}


#define EXTRACT_BLOCK_HDR(_b, _tlsf, _fl, _sl) do {                 \
        _tlsf -> matrix [_fl] [_sl] = _b -> ptr.free_ptr.next;      \
        if (_tlsf -> matrix[_fl][_sl])                              \
            _tlsf -> matrix[_fl][_sl] -> ptr.free_ptr.prev = NULL;  \
        else {                                                      \
            clear_bit (_sl, &_tlsf -> sl_bitmap [_fl]);             \
            if (!_tlsf -> sl_bitmap [_fl])                          \
                clear_bit (_fl, &_tlsf -> fl_bitmap);               \
        }                                                           \
        _b -> ptr.free_ptr.prev =  NULL;                \
        _b -> ptr.free_ptr.next =  NULL;                \
    }while(0)


#define EXTRACT_BLOCK(_b, _tlsf, _fl, _sl) do {                         \
        if (_b -> ptr.free_ptr.next)                                    \
            _b -> ptr.free_ptr.next -> ptr.free_ptr.prev = _b -> ptr.free_ptr.prev; \
        if (_b -> ptr.free_ptr.prev)                                    \
            _b -> ptr.free_ptr.prev -> ptr.free_ptr.next = _b -> ptr.free_ptr.next; \
        if (_tlsf -> matrix [_fl][_sl] == _b) {                         \
            _tlsf -> matrix [_fl][_sl] = _b -> ptr.free_ptr.next;       \
            if (!_tlsf -> matrix [_fl][_sl]) {                          \
                clear_bit (_sl, &_tlsf -> sl_bitmap[_fl]);              \
                if (!_tlsf -> sl_bitmap [_fl])                          \
                    clear_bit (_fl, &_tlsf -> fl_bitmap);               \
            }                                                           \
        }                                                               \
        _b -> ptr.free_ptr.prev = NULL;                 \
        _b -> ptr.free_ptr.next = NULL;                 \
    } while(0)



#define INSERT_BLOCK(_b, _tlsf, _fl, _sl) do {                          \
        _b -> ptr.free_ptr.prev = NULL; \
        _b -> ptr.free_ptr.next = _tlsf -> matrix [_fl][_sl]; \
        if (_tlsf -> matrix [_fl][_sl])                                 \
            _tlsf -> matrix [_fl][_sl] -> ptr.free_ptr.prev = _b;       \
        _tlsf -> matrix [_fl][_sl] = _b;                                \
        set_bit (_sl, &_tlsf -> sl_bitmap [_fl]);                       \
        set_bit (_fl, &_tlsf -> fl_bitmap);                             \
    } while(0)



static __inline void *get_new_area(size_t * size) {
    return ((void *) ~0);
}


static __inline bhdr_t *process_area(void *area, size_t size) {
    bhdr_t *b, *lb, *ib;
    area_info_t *ai;

    ib = (bhdr_t *) area;
    ib->size =
            (sizeof(area_info_t) < MIN_BLOCK_SIZE) ?
                    MIN_BLOCK_SIZE : ROUNDUP_SIZE(sizeof(area_info_t)) | USED_BLOCK | PREV_USED;

    b = (bhdr_t *) GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    b->size = ROUNDDOWN_SIZE(size - 3 * BHDR_OVERHEAD - (ib->size & BLOCK_SIZE)) | USED_BLOCK | PREV_USED;
    b->ptr.free_ptr.prev = b->ptr.free_ptr.next = 0;
    lb = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    lb->prev_hdr = b;
    lb->size = 0 | USED_BLOCK | PREV_FREE;
    ai = (area_info_t *) ib->ptr.buffer;
    ai->next = 0;
    ai->end = lb;
    return ib;
}



/*!
 *  \brief initis a memory pool with tlsf infraestructure
 */
size_t init_memory_pool(size_t mem_pool_size, void *mem_pool) {
    tlsf_t *tlsf;
    bhdr_t *b, *ib;

    if (!mem_pool || !mem_pool_size || mem_pool_size < sizeof(tlsf_t) + BHDR_OVERHEAD * 8) {
        return -1;
    }

    if (((unsigned long) mem_pool & PTR_MASK)) {
        return -1;
    }
    tlsf = (tlsf_t *) mem_pool;
    if (tlsf->tlsf_signature == TLSF_SIGNATURE) {
        mp = mem_pool;
        b = GET_NEXT_BLOCK(mp, ROUNDUP_SIZE(sizeof(tlsf_t)));
        return b->size & BLOCK_SIZE;
    }

    mp = mem_pool;

    tlsf->tlsf_signature = TLSF_SIGNATURE;

    ib = process_area(GET_NEXT_BLOCK(mem_pool, ROUNDUP_SIZE(sizeof(tlsf_t))),
            ROUNDDOWN_SIZE(mem_pool_size - sizeof(tlsf_t)));
    b = GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    free_ex(b->ptr.buffer, tlsf);
    tlsf->area_head = (area_info_t *) ib->ptr.buffer;

    tlsf->used_size = mem_pool_size - (b->size & BLOCK_SIZE);
    tlsf->max_size = tlsf->used_size;

    return (b->size & BLOCK_SIZE);
}


/*!
 * \brief gets used heap
 */
//size_t get_used_size(void *mem_pool) {
//    return ((tlsf_t *) mem_pool)->used_size;
//}


/*!
 * \brief allocates block from specified heap
 */
void *malloc_ex(size_t size, void *mem_pool) {
    tlsf_t *tlsf = (tlsf_t *) mem_pool;
    bhdr_t *b, *b2, *next_b;
    int32_t fl, sl;
    size_t tmp_size;

    size = (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(size);

    MAPPING_SEARCH(&size, &fl, &sl);

    b = FIND_SUITABLE_BLOCK(tlsf, &fl, &sl);

    if (b == 0) return NULL;

    EXTRACT_BLOCK_HDR(b, tlsf, fl, sl);

    next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    tmp_size = (b->size & BLOCK_SIZE) - size;

    if (tmp_size >= sizeof(bhdr_t)) {
        tmp_size -= BHDR_OVERHEAD;
        b2 = GET_NEXT_BLOCK(b->ptr.buffer, size);
        b2->size = tmp_size | FREE_BLOCK | PREV_USED;
        next_b->prev_hdr = b2;
        MAPPING_INSERT(tmp_size, &fl, &sl);
        INSERT_BLOCK(b2, tlsf, fl, sl);
        b->size = size | (b->size & PREV_STATE);
    } else {
        next_b->size &= (~PREV_FREE);
        b->size &= (~FREE_BLOCK);
    }

    TLSF_ADD_SIZE(tlsf, b);
    return (void *) b->ptr.buffer;
}


/*!
 * \brief free a block from specific memory pool
 */
void free_ex(void *ptr, void *mem_pool) {
    tlsf_t *tlsf = (tlsf_t *) mem_pool;
    bhdr_t *b, *tmp_b;
    int32_t fl = 0, sl = 0;

    if (!ptr) return;

    b = (bhdr_t *) ((uint8_t *) ptr - BHDR_OVERHEAD);
    b->size |= FREE_BLOCK;
    TLSF_REMOVE_SIZE(tlsf, b);

    b->ptr.free_ptr.prev = NULL;
    b->ptr.free_ptr.next = NULL;

    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);

    if (tmp_b->size & FREE_BLOCK) {
        MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);
        b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
    }
    if (b->size & PREV_FREE) {
        tmp_b = b->prev_hdr;
        MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);
        tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
        b = tmp_b;
    }

    MAPPING_INSERT(b->size & BLOCK_SIZE, &fl, &sl);
    INSERT_BLOCK(b, tlsf, fl, sl);

    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    tmp_b->size |= PREV_FREE;
    tmp_b->prev_hdr = b;
}

/** Public functions */

OsStatus_t uLipeMemInit(void)
{
    OsStatus_t ret = kStatusOk;
    size_t s = OS_HEAP_SIZE + sizeof(tlsf_t);

    s = init_memory_pool(s, OsCoreMemory);
    memSingleTonMutex = (OsHandler_t) malloc_ex(sizeof(Mutex_t), OsCoreMemory);

    uLipeAssert(memSingleTonMutex != NULL);
    uLipeAssert(s != 0);

    return ret;
}

void *uLipeMemAlloc(size_t size)
{
    void *ret = NULL;
    uint32_t sReg = 0;

    /* wraps the requested block to highest value supported by system */
    if (size > OS_MAX_SIZED_HEAP_BLOCK) size = OS_MAX_SIZED_HEAP_BLOCK;

    if(uLipeKernelIsRunning())
        uLipeMutexTake(memSingleTonMutex);
    else
        OS_CRITICAL_IN();

    /* request memory from the allocator block. */
    ret = malloc_ex(size, OsCoreMemory);

    if(uLipeKernelIsRunning())
        uLipeMutexGive(memSingleTonMutex);
    else
        OS_CRITICAL_OUT();



    return ret;
}

void uLipeMemFree(void *mem)
{

    uint32_t sReg = 0;

    if (mem != NULL)
    {
        if(uLipeKernelIsRunning())
            uLipeMutexTake(memSingleTonMutex);
        else
            OS_CRITICAL_IN();

        free_ex(mem, OsCoreMemory);


        if(uLipeKernelIsRunning())
            uLipeMutexGive(memSingleTonMutex);
        else
            OS_CRITICAL_OUT();

    }
}

