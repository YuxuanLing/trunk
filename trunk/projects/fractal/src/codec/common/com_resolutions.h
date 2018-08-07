#ifndef RESOLUTIONS_H
#define RESOLUTIONS_H

typedef enum source_res_t {
  RES_SQCIF = 1,        /* 1 */
  RES_QCIF,             /* 2 */
  RES_SIF,              /* 3 */
  RES_CIF,              /* 4 */
  RES_WCIF,             /* 5 */
  RES_25CIF,            /* 6 */
  RES_VGA,              /* 7 */
  RES_4SIF,             /* 8 */
  RES_W3CIF,   /* 448p    // 9 */
  RES_4CIF,             /* 10 */
  RES_SVGA,             /* 11 */
  RES_W4CIF,            /* 12 */
  RES_XGA,              /* 13 */
  RES_720P,             /* 14 */
  RES_1080P,            /* 15 */
  RES_UNKNOWN           /* 16 */
} source_res_t;

#endif
