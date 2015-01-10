/*  File:     wmcalc_t.h
 *  Author:   Edward H. Flora <ehflora@access1.net>
 *  Version:  0.2
 *
 *  Description:
 *  This file contains the typedefs for the wmcalc program.
 *
 *  Change History:
 *  Date       Modification
 *  10/25/00   Original file creation, extracted from wmcalc.h
 */

#ifndef WMCALC_T_H
#define WMCALC_T_H

typedef struct _XpmIcon {
  Pixmap pixmap;
  Pixmap mask;
  XpmAttributes attributes;
} XpmIcon;

typedef struct _button_region {
  int x,y;
  int i,j;
} ButtonArea;

typedef enum {
  MEM_LABEL_0 = 0,
  MEM_LABEL_1,
  MEM_LABEL_2,
  MEM_LABEL_3,
  MEM_LABEL_4,
  MEM_LABEL_5,
  MEM_LABEL_6,
  MEM_LABEL_7,
  MEM_LABEL_8,
  MEM_LABEL_9,
  MEM_LOCK_0,
  MEM_LOCK_1,
  MEM_LOCK_2,
  MEM_LOCK_3,
  MEM_LOCK_4,
  MEM_LOCK_5,
  MEM_LOCK_6,
  MEM_LOCK_7,
  MEM_LOCK_8,
  MEM_LOCK_9,
  IMAG_LABEL,
  CALC_LABEL,
  MAX_LABEL
} cfg_var_label_type;


#endif
