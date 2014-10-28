/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsFlags.h
 *
 *  \brief uLipe Rtos flag bits management interface file
 *
 *  In this file the user will find the definitions and prototypes
 *  of the functions which manages the flag bits groups
 *
 *  Author: FSN
 *
 */

#ifndef __OS_FLAGS_H
#define __OS_FLAGS_H


/*
 * Custom flags codes
 */

#define OS_FLAGS_PEND_ALL	0x01	//Wait for a specific group
#define OS_FLAGS_PEND_ANY   0x02	//Wait for any flag in group
#define OS_FLAGS_CONSUME    0x04	//COnsume these flag after its asserted

/*
 * flag node control block:
 */

struct flag_
{
	uint32_t flagRegister;					 //flagGrpRegister
	uint8_t  taskPending[OS_NUMBER_OF_TASKS];//tasks that pending this flag grp
	struct flag_ *nextNode;
};

typedef struct flag_  FlagsGrp_t;
typedef struct flag_* FlagsGrpPtr_t;

#if OS_FLAGS_MODULE_EN > 0

/*
 * Function prototypes:
 */

/*!
 *  uLipeFlagsInit()
 *  \brief Inits all flags kernel objects
 */
void uLipeFlagsInit(void);

/*!
 * 	uLipeFlagsCreate()
 * 	\brief Create a flag bits group
 *  \param
 *  \return
 */
OsHandler_t uLipeFlagsCreate(OsStatus_t *err);

/*!
 *  uLipeFlagsPend()
 *  \brief Make a task to pend for a flag bit or a group of bits
 *  \param
 *  \return
 */
OsStatus_t uLipeFlagsPend(OsHandler_t h, uint32_t flags, uint8_t opt, uint16_t timeout);

/*!
 *  uLipeFlagsPost()
 *  \brief Assert a flag bit or a group of flag bits
 *  \param
 *  \return
 */
OsStatus_t uLipeFlagsPost(OsHandler_t h, uint32_t flags);

/*!
 *  uLipeFlagsDelete()
 *  \brief Destroy a kernel object that control a group of flags
 *
 */
OsStatus_t uLipeFlagsDelete(OsHandler_t *h );



#endif

#endif
