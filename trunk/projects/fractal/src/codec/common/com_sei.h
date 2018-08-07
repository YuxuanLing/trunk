#ifndef _COM_SEI_H_
#define _COM_SEI_H_

typedef enum
{
  SEI_TYPE_USER_DATA_REGISTERED   = 4,
  SEI_TYPE_USER_DATA_UNREGISTERED = 5,
  SEI_TYPE_DEC_REF_PIC_MARKING_REPETITION = 7
} seitype_t;


static const uint8_t uuid[16] = {
  0x8a, 0xb2, 0xc3, 0x44, 0x85, 0x84, 0x2f, 0xfd,
  0xa1, 0x12, 0x7a, 0xbc, 0x75, 0x10, 0x22, 0x87
};

#endif
