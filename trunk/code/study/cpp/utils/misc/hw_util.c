/*****************************************************************************
*
* DESCRIPTION:
*	C-model hardware simulation utility functions.
*
* $Log: vsys#comm#cmodel#inc#src#hw_util.c,v $
* Revision 1.4  2006-10-11 16:18:18-07  junqiang
* DMA transfer unit is changed to 64-bit
*
* Revision 1.3  2006-10-05 21:33:26-07  junqiang
* changed the interface of dual parse and VLD to be used by AP.
*
* Revision 1.2  2006-06-22 12:19:01-07  junqiang
* bug fixing: declaration mismatch
*
* Revision 1.1  2006-06-22 10:47:06-07  junqiang
* C model of DMA channel, DMA descriptor link list and semaphore
*
*
*******************************************************************************/

#include "hw_util.h"

/**************************************************************************
* Function:
*   InitSemaphore
*
* Description: 
*   Initialize a semaphore, set the counter max and initialize 
*    the counter to be zero
*
* Input params:
*   SEMAPHORE *sem ---- pointer to semaphore
*   UNSG32 sem_id  ---- semaphore id
*   UNSG32 max     ---- upper limit of semahper counter
*
* Return:  
*    void
***************************************************************************/
void InitSemaphore( SEMAPHORE *sem,
                    UNSG32 sem_id,
                    UNSG32 max ) 
{
    sem->id = sem_id;
    sem->counter = 0;
    sem->max = max;
}

/**************************************************************************
* Function:
*   InitSemaphore
*
* Description: 
*   Reset a semaphore's counter to be zero
*
* Input params:
*   SEMAPHORE *sem ---- pointer to semaphore
*
* Return:  
*   void
**************************************************************************/
void ResetSemaphore( SEMAPHORE *sem ) 
{
    sem->counter = 0;
}

/**************************************************************************
* Function:
*    IncrementSemaphore
*
* Description:
*   Increment the semaphore counter by one, called by producer.
*
* Input params:
*   SEMAPHORE *sem ---- pointer to semaphore
*
* Return:  
*    SIGN32  0 if successful, no overflow
*           -1 if overflow happens
***************************************************************************/
SIGN32 IncrementSemaphore( SEMAPHORE *sem )
{
    SIGN32 ret = SEMA_NORMAL;
    if(sem->counter < sem->max) {
        sem->counter++;
    }
    else {
        ret = SEMA_OVERFLOW;
    }
    return ret;    
}

/**************************************************************************
* Function:
*   DecrementSemaphore
*
* Description: 
*   Decrement the semaphore counter by one, called by consumer.
*
* Input params:
*   SEMAPHORE *sem ---- pointer to semaphore
*
* Return:  
*    SIGN32    0 if successful, no underflow
*             -1 if underflow happens
***************************************************************************/
SIGN32 DecrementSemaphore( SEMAPHORE *sem )
{
    SIGN32 ret = SEMA_NORMAL;
    if(sem->counter > 0) {
        sem->counter--;
    }
    else {
        ret = SEMA_UNDERFLOW;
    }
    return ret;    
}

/**************************************************************************
* Function:
*   SemaphoreUnderFlow
*
* Description:
*   To tell the semaphore is underflow, (counter == 0)
*
* Input params:
*   SEMAPHORE *sem ---- pointer to semaphore
*
* Return:
*   UNSG32  1: underflow 0: no - underflow
***************************************************************************/
UNSG32 SemaphoreUnderFlow( SEMAPHORE *sem )
{
    return (sem->counter == 0);
}

/**************************************************************************
* Function:
*   SemaphoreOverFlow
*
* Description:
*   To tell the semaphore is underflow, (counter == 0)
*
* Input params:
*   SEMAPHORE *sem ---- pointer to semaphore
*
* Return:
*   UNSG32  1: overflow 0: no - overflow
***************************************************************************/
UNSG32 SemaphoreOverFlow( SEMAPHORE *sem )
{
    return (sem->counter == sem->max);
}

/**************************************************************************
* Function:
*   InitDescriptorRingList
*
* Description:
*   Initialize a dma descriptor ring list, number of descriptor
*   is determined by semaphore.
*
* Input params:
*   DESCRIPTOR_LIST *list ---- pointer to descriptor list to be initialized
*   SEMAPHORE *sem        ---- pointer to the semaphore
*   UNSG8 *bufs[]         ---- the list of buffer starting addresses
*   SIGN32 *buf_length    ---- the list of buffer lengths
*
* Return:
*   DMA_DESCRIPTOR*  >0:     the head of the list
*                  NULL:    error happens (e.g ERR_MEMORY)
***************************************************************************/
DMA_DESCRIPTOR* InitDescriptorRingList( DESCRIPTOR_LIST *list,
                               SEMAPHORE *sem,
                               UNSG8 *bufs[],
                               SIGN32 *buf_length,
                               SIGN32 num_bufs)
{
    SIGN32 i, num_desc;
    DMA_DESCRIPTOR *prev = NULL, *desc;

    list->sem = sem;
    num_desc = num_bufs;

    for(i = 0; i < num_desc; i++) {
        if( ( desc = (DMA_DESCRIPTOR *)malloc(sizeof(DMA_DESCRIPTOR)) ) == NULL )
            return NULL;

        desc->buf = bufs[i];
        desc->len = buf_length[i];
        desc->next = NULL;
        desc->sem = sem;

        if(prev == NULL) {
            list->wr_ptr = desc;
            list->rd_ptr = desc;                
        }
        else {
            prev->next = desc;
        }
        prev = desc;
    }

    /* make it a ring list */
    desc->next = list->wr_ptr;
    
    return list->wr_ptr;
}

/**************************************************************************
* Function
*   DeleteDescriptorList
*
* Description:
*   Delete a dma descriptor list, free all the memory 
*   allocated in InitDescriptorRingList.
*
* Input params:
*   DESCRIPTOR_LIST *list ---- pointer to descriptor list to be deleted
*
* Return:
*   void
***************************************************************************/
void DeleteDescriptorRingList( DESCRIPTOR_LIST *list )
{
    DMA_DESCRIPTOR *desc, *last;

    /* free semaphore resource */
    ResetSemaphore(list->sem);

    desc = list->wr_ptr;
    /* free the link list */
    do {
        last = desc;
        desc = desc->next;
        free(last);
    }while (desc != list->wr_ptr);

    list->wr_ptr = list->rd_ptr = NULL;
}


/**************************************************************************
* Function:
*   GetNextWriteDescriptor
*
* Description: Get the write pointer (tail) of the link list, 
*              to write a new descriptor
*
* Input params:
*   DMA_DESCRIPTOR *desc ---- pointer to current write descriptor
*
* Return:
*   (DMA_DESCRIPTOR *) >0      pointer to the write position
*                     NULL    if the list is empty
***************************************************************************/
DMA_DESCRIPTOR *GetNextWriteDescriptor( DMA_DESCRIPTOR *desc )
{
    if( !SemaphoreOverFlow(desc->sem) ) {
        IncrementSemaphore(desc->sem);
        return desc->next;
    }
    else
        return NULL;
}

/**************************************************************************
* Function:
*   GetNextReadDescriptor
*
* Description:
*   Get the read pointer (tail) of the link list,to consume the buffer 
*   prepared by producer
*
* Input params:
*   DMA_DESCRIPTOR *desc ---- pointer to current read descriptor
*
* Return:
*   (DMA_DESCRIPTOR *) >0    pointer to the current write position
*	                   NULL  if the list is empty
***************************************************************************/
DMA_DESCRIPTOR *GetNextReadDescriptor( DMA_DESCRIPTOR *desc )
{
    if( !SemaphoreUnderFlow(desc->sem) ) {
       DecrementSemaphore(desc->sem);
       return desc->next;          
    }
    else
        return NULL;
}

/**************************************************************************
* Function: 
*   InitDMAChannel 
*
* Description: Initlize the DMA channel, set its read/write flag, 
*              associate with the descriptor list
*
* Input params:
*   DMA_CHANNEL *chan    ---- pointer to dma channel to be initialzied
*   UNSG32 rd_wr,        ---- read/write flag 0: READ_CHANNEL 1: WRITE_CHANNEL
*   DMA_DESCRIPTOR *desc ---- first dma descriptor in the list
*
* Return:
*   void
***************************************************************************/
void InitDMAChannel( DMA_CHANNEL *chan,
                     UNSG32 rd_wr,
                     DMA_DESCRIPTOR *desc ) 
{
    chan->curr_desc  = desc;
    chan->pos = 0;
    chan->rd_wr = rd_wr;
}

/**************************************************************************
* Function:
*   DMATransferWords
* Description:
*   Transfer a number of words from/to DMA, assume that transferring length
*   is no larger than the underlying buffer length in link list.
*
* Input params:
*   void  *chan       ---- pointer to dma channel
*   UNSG8 *buf        ---- dest/src address for write/read dma channel
*   SIGN32 num_words  ---- number of words (32-bit) to be transffered 
*                          in this transaction
* Return:
*   SIGN32	>=0 number of words been transffered
*            <0 if error happens, e.g end of dma descriptor list
***************************************************************************/
SIGN32 DMATransferWords( void  *dma_chan,
                         UNSG8 *buf,
                         SIGN32 num_words ) 
{
    SIGN32 ret = 0;
    SIGN32 num_bytes = num_words*8;
    SIGN32 avail_len;
    DMA_CHANNEL  *chan = (DMA_CHANNEL *)dma_chan;
    DMA_DESCRIPTOR *next_desc;
    DMA_DESCRIPTOR *desc = chan->curr_desc;    

    SEMAPHORE bak_sem;

    memcpy(&bak_sem, desc->sem, sizeof(SEMAPHORE));

    assert(chan->rd_wr == READ_DMA_CHANNEL || chan->rd_wr == WRITE_DMA_CHANNEL);

    /* no buffers available */
    if( (chan->rd_wr == READ_DMA_CHANNEL && SemaphoreUnderFlow(desc->sem)) || 
        (chan->rd_wr == WRITE_DMA_CHANNEL && SemaphoreOverFlow(desc->sem)) )
        return 0;

    

    avail_len = desc->len - chan->pos;
    if(avail_len > num_bytes) {
        /*current buffer has enough bits */
        if(chan->rd_wr == READ_DMA_CHANNEL)
            memcpy(buf, desc->buf + chan->pos, num_bytes);
        else
            memcpy(desc->buf + chan->pos, buf, num_bytes);
        chan->pos += num_bytes;
        ret = num_bytes;
    }
    else {        
        /* copy the rest of current buffer */
        if(avail_len) {
            if(chan->rd_wr == READ_DMA_CHANNEL)
                memcpy(buf, desc->buf + chan->pos, avail_len);
            else
                memcpy(desc->buf + chan->pos, buf, avail_len);
            ret = avail_len;
        }        
        

        /* go to next buffer, update semaphore since current buffer is done */
        if(chan->rd_wr == READ_DMA_CHANNEL)
            next_desc = GetNextReadDescriptor(desc);            
        else            
            next_desc = GetNextWriteDescriptor(desc);

        /* reaches the last buffer */
        if(next_desc != NULL) {
            /* has enough bytes to transfer */
            chan->pos = num_bytes-ret;
            desc = next_desc;

            if(chan->pos) {
                if(chan->rd_wr == READ_DMA_CHANNEL)
                    memcpy(buf + ret, desc->buf, chan->pos);
                else
                    memcpy(desc->buf, buf + ret, chan->pos);
            }

            chan->curr_desc = desc;
            ret = num_bytes;
        }
    }

   
    return ret;
}
