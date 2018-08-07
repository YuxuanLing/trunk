/******************************************************************************
*
* DESCRIPTION:
*   All hardware simulation modules, such as semaphore, DMA operations
*
* Implementation C files:
*	- "./src/hw_util.c"
*
* $Log: vsys#comm#cmodel#inc#hw_util.h,v $
* Revision 1.5  2007-08-27 18:50:56-07  lsha
* Re-partition include files
*
* Revision 1.4  2006-11-22 15:19:15-08  junqiang
* redcued .h file including.
*
* Revision 1.3  2006-10-05 21:33:22-07  junqiang
* changed the interface of dual parse and VLD to be used by AP.
*
* Revision 1.2  2006-06-22 12:19:06-07  junqiang
* bug fixing: declaration mismatch
*
* Revision 1.1  2006-06-22 10:46:58-07  junqiang
* C model of DMA channel, DMA descriptor link list and semaphore
*
*
*******************************************************************************/
#ifndef	HW_UTIL
#define	HW_UTIL						"         HW_UTIL >>>    "

#include "cinclude.h"

#ifdef	__cplusplus
extern	"C"
{
#endif

    /* macros, enums */
    enum {
        SEMA_NORMAL,
        SEMA_OVERFLOW,
        SEMA_UNDERFLOW
    };

    /* data structures */

    /*hardware semaphore simulator, cannot block system, acts like a counter, 
    needs to be re-written in AP*/
    typedef struct tagSEMAPHORE {
        UNSG32 id;        /* Semaphore ID (may not be necessary??) */
        SIGN32 counter;   /* Semaphore counter, between 0 and max */
        SIGN32 max;       /* upper limit of the counter, maximum resources */
    }SEMAPHORE ;

    typedef struct tagDMA_DESCRIPTOR {
        UNSG8                       *buf;  /* buffer starting address */
        SIGN32                      len;   /* buffer length in bytes */
        SEMAPHORE                   *sem;  /* semaphore of the link list,
                                              control the access of the descriptor*/
        struct tagDMA_DESCRIPTOR    *next; /* pointer to next descriptor for link list */
    }DMA_DESCRIPTOR ;

    typedef struct tagDESCRIPTOR_LIST {
        DMA_DESCRIPTOR *rd_ptr; /* read pointer of the link list (head) */
        DMA_DESCRIPTOR *wr_ptr; /* write pointer of the link list (tail) */
        SEMAPHORE      *sem;    /* semaphore of the link list, 
                                   control the access of the descriptor */
    }DESCRIPTOR_LIST;

    enum {
        READ_DMA_CHANNEL = 0,   /* constant definition for read dma channel */
        WRITE_DMA_CHANNEL       /* constant definition for write dma channel */
    };

    typedef struct tagDMA_CHANNEL {        
        UNSG32          rd_wr;      /* 0 or 1 to denote whether it is read/write DMA channel */
        DMA_DESCRIPTOR  *curr_desc; /* current descriptor in use, has a buffer and length */
        SIGN32          pos;        /* current position relatvie to the current buffer */
    }DMA_CHANNEL;

    /*------------semahpore operations------------*/

    void InitSemaphore(SEMAPHORE *sem,UNSG32 sem_id,UNSG32 max);
    void ResetSemaphore(SEMAPHORE *sem);
    SIGN32 IncrementSemaphore(SEMAPHORE *sem);
    SIGN32 DecrementSemaphore(SEMAPHORE *sem);
    UNSG32 SemaphoreOverFlow(SEMAPHORE *sem);
    UNSG32 SemphoreUnderFlow(SEMAPHORE *sem);

    /*------------link list operations------------*/
    DMA_DESCRIPTOR* InitDescriptorRingList(DESCRIPTOR_LIST *list,
                                  SEMAPHORE *sem,UNSG8 *bufs[],
                                  SIGN32 *buf_length,
                                  SIGN32 num_bufs);
    void DeleteDescriptorRingList(DESCRIPTOR_LIST *list);
    DMA_DESCRIPTOR *GetNextWriteDescriptor(DMA_DESCRIPTOR *desc);
    DMA_DESCRIPTOR *GetNextReadDescriptor(DMA_DESCRIPTOR *desc);

    /*------------dma transfer operations------------*/
    void InitDMAChannel(DMA_CHANNEL *chan, UNSG32 rd_wr, DMA_DESCRIPTOR *desc);
    SIGN32 DMATransferWords(void *chan, UNSG8 *buf, SIGN32 num_words);

#ifdef	__cplusplus
}
#endif

#endif
