/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file uLipeRtos4.h
 *
 *  \brief ulipe memory management interface file
 *
 *  Author: FSN
 *
 */

#ifndef __OSMEM_H
#define __OSMEM_H

/*!
 * \brief initialize a memory region to be used as memory source
 */
OsStatus_t uLipeMemInit(void);


/*!
 * \brief allocates a memory block from specified heap
 */
void *uLipeMemAlloc(size_t size);


/*!
 * \brief free a previous memory allocated block
 */
void uLipeMemFree(void *mem);


#endif /* OSMEM_H_ */
